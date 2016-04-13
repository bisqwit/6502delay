; delay_n, delays n cycles with small code
; Will clobber Y, Z+N, and D
; Will preserve A, X, S, C, V, and I (interrupt-disable flag)
;
; This version of delay_n does not change any bytes in the $100 page.
;
; Generated code is:
; - 2 bytes or less when n <= 4
; - 3 bytes or less when n <= 6
; - 4 bytes or less when n <= 8
; - 5 bytes or less when n <= 14
; - 6 bytes or less when n <= 16
; - 7 bytes or less when n <= 26
; - 8 bytes or less when n <= 1293
; - 9 bytes or less when n <= 1303
; -11 bytes or less when n <= 2319
; -12 bytes or less when n <= 2506
; -13 bytes or less when n <= 3316
; -14 bytes or less when n <= 4569
; -15 bytes or less when n <= 5032
; -16 bytes or less when n <= 5493
; -17 bytes or less when n <= 6088
; -18 bytes or less when n <= 7534
; -20 bytes or less when n <= 8522
; -21 bytes or less when n <= 8607
; -22 bytes or less when n <= 10164
; -23 bytes or less when n <= 10660
; -24 bytes or less when n <= 11140
; -25 bytes or less when n <= 11365
; -26 bytes or less when n <= 12696
; -27 bytes or less when n <= 13710
; -29 bytes or less when n <= 15308
; -30 bytes or less when n <= 15830
; -31 bytes or less when n <= 16527
; -32 bytes or less when n <= 17249
; -33 bytes or less when n <= 17809
; -34 bytes or less when n <= 18508
; -35 bytes or less when n <= 18925
; -36 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=3.1, max=5
; - When 11 <= n <= 20: min=4, avg=5.6, max=7
; - When 21 <= n <= 30: min=5, avg=6.3, max=8
; - When 31 <= n <= 50: min=4, avg=6.2, max=8
; - When 51 <= n <= 75: min=5, avg=6.2, max=8
; - When 76 <= n <= 100: min=5, avg=6.3, max=8
; - When 101 <= n <= 200: min=5, avg=6.3, max=8
; - When 201 <= n <= 300: min=4, avg=6.4, max=8
; - When 301 <= n <= 500: min=5, avg=6.4, max=8
; - When 501 <= n <= 750: min=4, avg=6.4, max=8
; - When 751 <= n <= 1000: min=5, avg=6.4, max=8
; - When 1001 <= n <= 2000: min=5, avg=7.6, max=11
; - When 2001 <= n <= 3000: min=6, avg=9.9, max=13
; - When 3001 <= n <= 5000: min=8, avg=12.0, max=15
; - When 5001 <= n <= 7500: min=10, avg=15.5, max=18
; - When 7501 <= n <= 10000: min=13, avg=19.2, max=22
; - When 10001 <= n <= 20000: min=17, avg=27.5, max=36
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
	ldy $A4
.elseif((n > 3 && n < 8) || n = 9 || n = 13 || n = 14 || n = 17 || n = 20 || n = 23 || n = 25 || n = 27 || n = 28 || n = 30 || n = 33 || n = 38 || n = 40 || n = 42 || n = 47 || n = 48 || n = 53 || n = 58 || n = 60 || n = 62 || n = 63 || n = 65 || n = 68 || n = 70 || n = 73 || n = 75 || n = 77)
	; 2..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 8)
	; 4 bytes
	.repeat 4
	 nop
	.endrepeat
.elseif(n = 10)
	; 4 bytes
	rol $26
	ror $26
.elseif(n = 11)
	; 4 bytes
	tay
	pla
	pha
	tya
.elseif(n = 12)
	; 4 bytes
	rol $36,X
	ror $36,X
.elseif(n = 15 || n = 19 || n = 24 || n = 32 || n = 39 || n = 52 || n = 54 || n = 59 || n = 67 || n = 72 || n = 74)
	; 6..8 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16 || n = 21 || n = 31 || n = 61 || n = 66 || n = 76)
	; 5 bytes
	ldy #(n-1)/5
	dey
	Jne *-1
.elseif(n = 18)
	; 6 bytes
	tay
	.repeat 2
	 pla
	 pha
	.endrepeat
	tya
.elseif(n = 22 || n = 29 || n = 43)
	; 6 bytes
	ldy #(n-1)/7
	nop
	dey
	Jne *-2
.elseif((n .mod 5 = 1) && (n = 26 || (n > 35 && n < 57) || n = 71))
	; 5 bytes
	ldy #(n-6)/5
	dey
	Jpl *-1
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
	; 7 bytes
	ldy #(n-1)/9
	delay_n 4
	dey
	Jne *-3
.elseif(n = 44)
	; 6 bytes
	ldy #136 ;hides 'dey'
	nop
	dey
	Jmi *-3
.elseif(n = 45)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 49)
	; 4 bytes
	ldy #136 ;hides 'dey'
	Jmi *-1
.elseif(n = 50 || n = 57 || n = 64 || n = 78)
	; 6 bytes
	ldy #(n-8)/7
	nop
	dey
	Jpl *-2
.elseif(n = 55)
	; 5 bytes
	ldy #248 ;hides 'sed'
	iny
	Jne *-2
.elseif(n = 69)
	; 5 bytes
	ldy #136 ;hides 'dey'
	nop
	Jmi *-2
.endif
.else
; 79 to 156
.if(n = 79 || n = 84 || n = 94 || n = 102 || n = 104 || n = 109 || n = 114 || n = 119 || n = 124 || n = 129 || n = 137 || n = 139 || n = 144 || n = 149 || n = 154)
	; 7..8 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 80 || n = 115 || n = 150)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 81 || n = 86 || n = 96 || n = 101 || n = 111 || n = 116 || n = 121 || n = 141 || n = 146 || n = 151)
	; 5 bytes
	ldy #(n-6)/5
	dey
	Jpl *-1
.elseif(n = 82)
	; 7 bytes
	ldy #(n-10)/9
	delay_n 4
	dey
	Jpl *-3
.elseif(n = 83 || n = 87 || n = 88 || n = 90 || n = 93 || n = 95 || n = 97 || n = 98 || n = 100 || n = 103 || n = 105 || n = 108 || n = 110 || n = 112 || n = 117 || n = 118 || n = 122 || n = 123 || n = 125 || n = 128 || n = 130 || n = 132 || n = 133 || n = 135 || n = 138 || n = 140 || n = 143 || n = 145 || n = 147 || n = 152)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 85 || n = 92 || n = 127 || n = 148)
	; 6 bytes
	ldy #(n-1)/7
	nop
	dey
	Jne *-2
.elseif(n = 89)
	; 6 bytes
	ldy #136 ;hides 'dey'
	delay_n 4
	Jmi *-3
.elseif(n = 91 || n = 106 || n = 126 || n = 131 || n = 136 || n = 156)
	; 5 bytes
	ldy #(n-1)/5
	dey
	Jne *-1
.elseif(n = 99 || n = 113 || n = 120 || n = 134 || n = 155)
	; 6 bytes
	ldy #(n-8)/7
	nop
	dey
	Jpl *-2
.elseif(n = 107)
	; 6 bytes
	ldy #24 ;hides 'clc'
	dey
	dey
	Jne *-3
.elseif(n = 142)
	; 7 bytes
	ldy #24 ;hides 'clc'
	nop
	dey
	dey
	Jpl *-4
.elseif(n = 153)
	; 5 bytes
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
.endif
.endif
.else
; 157 to 312
.if(n = 157 || n = 158 || n = 160 || n = 163 || n = 165 || n = 168 || n = 173 || n = 175 || n = 178 || n = 180 || n = 182 || n = 187 || n = 188 || n = 192 || n = 193 || n = 195 || n = 198 || n = 200 || n = 202 || n = 203 || n = 205 || n = 208 || n = 210 || n = 212 || n = 213 || n = 222 || n = 223 || n = 227 || n = 228 || n = 230 || n = 233 || n = 235 || n = 237 || n = 238 || n = 240 || n = 243 || n = 245 || n = 247 || n = 248 || n = 250 || n = 252 || n = 257 || n = 258 || n = 263 || n = 265 || n = 268 || n = 270 || n = 272 || n = 273 || n = 275 || n = 278 || n = 280 || n = 283 || n = 285 || n = 292 || n = 293 || n = 297 || n = 298 || n = 300 || n = 303 || n = 305 || n = 308 || n = 310)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 159 || n = 164 || n = 177 || n = 184 || n = 189 || n = 194 || n = 199 || n = 207 || n = 209 || n = 214 || n = 219 || n = 229 || n = 234 || n = 242 || n = 244 || n = 249 || n = 254 || n = 259 || n = 264 || n = 269 || n = 277 || n = 282 || n = 287 || n = 289 || n = 294 || n = 299 || n = 304 || n = 312)
	; 6..8 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 161 || n = 166 || n = 171 || n = 181 || n = 191 || n = 201 || n = 206 || n = 231 || n = 236 || n = 241 || n = 251 || n = 266 || n = 281 || n = 291 || n = 296)
	; 5 bytes
	ldy #(n-6)/5
	dey
	Jpl *-1
.elseif(n = 162 || n = 169 || n = 197 || n = 204 || n = 225 || n = 232 || n = 239 || n = 253 || n = 267 || n = 288)
	; 6 bytes
	ldy #(n-8)/7
	nop
	dey
	Jpl *-2
.elseif(n = 167)
	; 5 bytes
	ldy #24 ;hides 'clc'
	dey
	Jne *-2
.elseif(n = 170)
	; 6 bytes
	ldy #200 ;hides 'iny'
	iny
	iny
	Jne *-3
.elseif(n = 172 || n = 217 || n = 262 || n = 307)
	; 7 bytes
	ldy #(n-10)/9
	delay_n 4
	dey
	Jpl *-3
.elseif(n = 174)
	; 5 bytes
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
.elseif((n .mod 5 = 1) && (n = 176 || n = 186 || n = 196 || (n > 210 && n < 227) || n = 246 || n = 256 || n = 261 || n = 271 || n = 276 || n = 286 || (n > 300 && n < 312)))
	; 5 bytes
	ldy #(n-1)/5
	dey
	Jne *-1
.elseif(n = 179)
	; 6 bytes
	ldy #216 ;hides 'cld'
	iny
	iny
	Jne *-3
.elseif(n = 183 || n = 190 || n = 218 || n = 260 || n = 274 || n = 295 || n = 302 || n = 309)
	; 6 bytes
	ldy #(n-1)/7
	nop
	dey
	Jne *-2
.elseif(n = 185 || n = 220 || n = 255 || n = 290)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 215)
	; 6 bytes
	ldy #24 ;hides 'clc'
	nop
	dey
	Jne *-3
.elseif(n = 224)
	; 6 bytes
	ldy #24 ;hides 'clc'
	nop
	dey
	Jpl *-3
.elseif(n = 279)
	; 5 bytes
	ldy #216 ;hides 'cld'
	iny
	Jne *-2
.elseif(n = 284)
	; 4 bytes
	ldy #200 ;hides 'iny'
	Jmi *-1
.endif
.endif
.else
; 313 to 625
.if(n = 313 || n = 315 || n = 317 || n = 318 || n = 320 || n = 322 || n = 327 || n = 328 || n = 332 || n = 333 || n = 335 || n = 338 || n = 340 || n = 342 || n = 343 || n = 345 || n = 348 || n = 350 || n = 353 || n = 355 || n = 357 || n = 362 || n = 363 || n = 367 || n = 368 || n = 370 || n = 373 || n = 375 || n = 377 || n = 378 || n = 380 || n = 383 || n = 385 || n = 387 || n = 388 || n = 390 || n = 392 || n = 402 || n = 403 || n = 405 || n = 408 || n = 410 || n = 412 || n = 413 || n = 415 || n = 418 || n = 420 || n = 422 || n = 423 || n = 425 || n = 427 || n = 432 || n = 433 || n = 437 || n = 438 || n = 440 || n = 443 || n = 445 || n = 447 || n = 448 || n = 450 || n = 453 || n = 455 || n = 457 || n = 458 || n = 460 || n = 462 || n = 468 || n = 472 || n = 473 || n = 475 || n = 478 || n = 480 || n = 482 || n = 483 || n = 488 || n = 490 || n = 492 || n = 493 || n = 495 || n = 497 || n = 502 || n = 507 || n = 508 || n = 510 || n = 513 || n = 515 || n = 517 || n = 518 || n = 520 || n = 523 || n = 525 || n = 527 || n = 528 || n = 530 || n = 535 || n = 537 || n = 538 || n = 542 || n = 543 || n = 545 || n = 548 || n = 552 || n = 553 || n = 555 || n = 558 || n = 560 || n = 563 || n = 565 || n = 567 || n = 570 || n = 572 || n = 573 || n = 578 || n = 580 || n = 583 || n = 585 || n = 587 || n = 588 || n = 590 || n = 593 || n = 597 || n = 598 || n = 600 || n = 602 || n = 605 || n = 607 || n = 608 || n = 612 || n = 613 || n = 615 || n = 618 || n = 620 || n = 623 || n = 625)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 314 || n = 319 || n = 324 || n = 329 || n = 334 || n = 339 || n = 347 || n = 349 || n = 354 || n = 364 || n = 369 || n = 374 || n = 382 || n = 384 || n = 389 || n = 394 || n = 399 || n = 409 || n = 417 || n = 419 || n = 424 || n = 429 || n = 434 || n = 439 || n = 444 || n = 452 || n = 454 || n = 459 || n = 464 || n = 469 || n = 474 || n = 479 || n = 489 || n = 494 || n = 499 || n = 504 || n = 509 || n = 514 || n = 522 || n = 524 || n = 529 || n = 534 || n = 539 || n = 544 || n = 549 || n = 557 || n = 559 || n = 564 || n = 569 || n = 574 || n = 579 || n = 584 || n = 592 || n = 594 || n = 599 || n = 604 || n = 609 || n = 614 || n = 619)
	; 7..8 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n .mod 5 = 1) && (n = 316 || n = 341 || n = 346 || n = 361 || n = 381 || n = 396 || n = 406 || n = 411 || n = 421 || n = 451 || n = 456 || n = 466 || n = 476 || n = 481 || n = 491 || (n > 500 && n < 512) || (n > 520 && n < 562) || n = 586 || (n > 595 && n < 607)))
	; 5 bytes
	ldy #(n-1)/5
	dey
	Jne *-1
.elseif((n .mod 5 = 1) && ((n > 320 && n < 337) || n = 351 || n = 356 || (n > 365 && n < 377) || n = 386 || n = 391 || n = 401 || n = 416 || (n > 425 && n < 447) || n = 461 || n = 471 || n = 486 || n = 496 || n = 516 || (n > 565 && n < 582) || n = 591 || (n > 610 && n < 622)))
	; 5 bytes
	ldy #(n-6)/5
	dey
	Jpl *-1
.elseif((n .mod 7 = 1) && ((n > 322 && n < 338) || n = 372 || n = 393 || n = 407 || n = 414 || n = 449 || n = 463 || n = 470 || n = 498 || n = 512 || n = 519 || n = 540 || n = 547 || n = 582 || (n > 602 && n < 625)))
	; 6 bytes
	ldy #(n-8)/7
	nop
	dey
	Jpl *-2
.elseif(n = 325 || n = 360 || n = 395 || n = 430 || n = 465 || n = 500)
	; 7 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 344 || n = 358 || n = 365 || n = 379 || n = 400 || n = 428 || n = 435 || n = 442 || n = 477 || n = 484 || n = 505 || n = 533 || n = 554 || n = 568 || n = 575 || n = 589)
	; 6 bytes
	ldy #(n-1)/7
	nop
	dey
	Jne *-2
.elseif(n = 352 || n = 397 || n = 487 || n = 550)
	; 7 bytes
	ldy #(n-1)/9
	delay_n 4
	dey
	Jne *-3
.elseif(n = 359)
	; 6 bytes
	ldy #216 ;hides 'cld'
	nop
	iny
	Jne *-3
.elseif(n = 398)
	; 5 bytes
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
.elseif(n = 404)
	; 6 bytes
	ldy #216 ;hides 'cld'
	dey
	dey
	Jmi *-3
.elseif(n = 467)
	; 6 bytes
	ldy #24 ;hides 'clc'
	iny
	iny
	Jpl *-3
.elseif(n = 485)
	; 6 bytes
	ldy #234 ;hides 'nop'
	dey
	dey
	Jmi *-3
.elseif(n = 503)
	; 5 bytes
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
.elseif(n = 532 || n = 577 || n = 595)
	; 7 bytes
	ldy #(n-10)/9
	delay_n 4
	dey
	Jpl *-3
.elseif(n = 562)
	; 8 bytes
	ldy #(n-1)/11
	delay_n 6
	dey
	Jne *-4
.elseif(n = 622)
	; 5 bytes
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
.endif
.endif
.else
; 626 to 1250
.if(n = 626 || n = 636 || n = 641 || n = 646)
	; 5 bytes
	ldy #(n-6)/5
	dey
	Jpl *-1
.elseif(n = 627 || n = 629 || n = 634 || n = 639 || n = 644 || n = 649 || n = 654 || n = 662 || n = 664 || n = 669 || n = 674 || n = 679 || n = 689 || n = 697 || n = 699 || n = 704 || n = 709 || n = 714 || n = 719 || n = 724 || n = 732 || n = 734 || n = 739 || n = 744 || n = 749 || n = 754 || n = 759 || n = 767 || n = 769 || n = 774 || n = 779 || n = 784 || n = 789 || n = 794 || n = 804 || n = 809 || n = 814 || n = 819 || n = 824 || n = 829 || n = 837 || n = 839 || n = 844 || n = 849 || n = 854 || n = 859 || n = 864 || n = 872 || n = 874 || n = 879 || n = 884 || n = 889 || n = 894 || n = 907 || n = 909 || n = 914 || n = 919 || n = 924 || n = 929 || n = 934 || n = 942 || n = 944 || n = 949 || n = 954 || n = 959 || n = 964 || n = 969 || n = 977 || n = 979 || n = 984 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1012 || n = 1014 || n = 1019 || n = 1024 || n = 1029 || n = 1034 || n = 1039 || n = 1047 || n = 1049 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1082 || n = 1084 || n = 1089 || n = 1094 || n = 1099 || n = 1104 || n = 1109 || n = 1119 || n = 1124 || n = 1129 || n = 1134 || n = 1139 || n = 1144 || n = 1152 || n = 1154 || n = 1159 || n = 1164 || n = 1169 || n = 1174 || n = 1179 || n = 1189 || n = 1194 || n = 1199 || n = 1204 || n = 1209 || n = 1214 || n = 1222 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1244 || n = 1249)
	; 7..8 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 628 || n = 630 || n = 632 || n = 633 || n = 635 || n = 637 || n = 643 || n = 648 || n = 650 || n = 653 || n = 655 || n = 657 || n = 658 || n = 660 || n = 663 || n = 665 || n = 668 || n = 670 || n = 675 || n = 677 || n = 678 || n = 682 || n = 683 || n = 688 || n = 690 || n = 692 || n = 693 || n = 695 || n = 698 || n = 700 || n = 702 || n = 703 || n = 705 || n = 707 || n = 713 || n = 717 || n = 718 || n = 720 || n = 723 || n = 725 || n = 728 || n = 733 || n = 735 || n = 737 || n = 738 || n = 740 || n = 742 || n = 747 || n = 752 || n = 753 || n = 755 || n = 758 || n = 760 || n = 762 || n = 763 || n = 765 || n = 768 || n = 770 || n = 772 || n = 773 || n = 777 || n = 783 || n = 787 || n = 788 || n = 790 || n = 793 || n = 795 || n = 797 || n = 798 || n = 803 || n = 805 || n = 807 || n = 808 || n = 810 || n = 812 || n = 817 || n = 818 || n = 822 || n = 823 || n = 825 || n = 828 || n = 830 || n = 832 || n = 833 || n = 835 || n = 838 || n = 840 || n = 842 || n = 843 || n = 845 || n = 852 || n = 853 || n = 857 || n = 858 || n = 860 || n = 863 || n = 867 || n = 868 || n = 870 || n = 873 || n = 875 || n = 877 || n = 878 || n = 880 || n = 882 || n = 887 || n = 888 || n = 893 || n = 895 || n = 898 || n = 900 || n = 902 || n = 903 || n = 905 || n = 908 || n = 910 || n = 912 || n = 913 || n = 915 || n = 917 || n = 922 || n = 923 || n = 927 || n = 928 || n = 930 || n = 933 || n = 938 || n = 940 || n = 943 || n = 945 || n = 948 || n = 950 || n = 952 || n = 957 || n = 963 || n = 965 || n = 968 || n = 970 || n = 972 || n = 973 || n = 975 || n = 978 || n = 980 || n = 983 || n = 985 || n = 987 || n = 992 || n = 993 || n = 997 || n = 998 || n = 1000 || n = 1003 || n = 1005 || n = 1007 || n = 1008 || n = 1010 || n = 1013 || n = 1015 || n = 1017 || n = 1018 || n = 1020 || n = 1022 || n = 1028 || n = 1032 || n = 1033 || n = 1035 || n = 1038 || n = 1040 || n = 1042 || n = 1043 || n = 1045 || n = 1048 || n = 1050 || n = 1053 || n = 1055 || n = 1062 || n = 1063 || n = 1067 || n = 1068 || n = 1073 || n = 1075 || n = 1077 || n = 1078 || n = 1080 || n = 1083 || n = 1085 || n = 1087 || n = 1088 || n = 1090 || n = 1092 || n = 1097 || n = 1098 || n = 1102 || n = 1103 || n = 1105 || n = 1108 || n = 1110 || n = 1112 || n = 1113 || n = 1118 || n = 1120 || n = 1122 || n = 1123 || n = 1125 || n = 1127 || n = 1132 || n = 1133 || n = 1137 || n = 1138 || n = 1140 || n = 1143 || n = 1145 || n = 1147 || n = 1148 || n = 1150 || n = 1153 || n = 1155 || n = 1157 || n = 1158 || n = 1160 || n = 1167 || n = 1168 || n = 1172 || n = 1173 || n = 1175 || n = 1178 || n = 1180 || n = 1182 || n = 1183 || n = 1185 || n = 1188 || n = 1190 || n = 1192 || n = 1193 || n = 1195 || n = 1197 || n = 1202 || n = 1203 || n = 1208 || n = 1210 || n = 1213 || n = 1215 || n = 1217 || n = 1218 || n = 1220 || n = 1223 || n = 1225 || n = 1227 || n = 1228 || n = 1230 || n = 1237 || n = 1238 || n = 1242 || n = 1243 || n = 1245 || n = 1248 || n = 1250)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && (n = 631 || (n > 650 && n < 1247)))
	; 5 bytes
	ldy #(n-1)/5
	dey
	Jne *-1
.elseif((n .mod 7 = 1) && (n = 638 || n = 652 || n = 673 || n = 687 || n = 708 || n = 715 || n = 729 || (n > 742 && n < 765) || n = 778 || n = 785 || n = 820 || n = 834 || n = 848 || n = 883 || (n > 917 && n < 940) || (n > 952 && n < 975) || (n > 987 && n < 1010) || (n > 1022 && n < 1045) || (n > 1057 && n < 1080) || (n > 1092 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldy #(n-1)/7
	nop
	dey
	Jne *-2
.elseif(n = 640 || n = 667 || n = 775 || n = 802 || n = 847 || n = 892 || n = 1027 || n = 1117 || n = 1162)
	; 7 bytes
	ldy #(n-10)/9
	delay_n 4
	dey
	Jpl *-3
.elseif(n = 642 || n = 710 || n = 745 || n = 780 || n = 815 || n = 850 || n = 885 || n = 920 || n = 955 || n = 990 || n = 1025 || n = 1060 || n = 1095 || n = 1130 || n = 1165 || n = 1200 || n = 1235)
	; 7..8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 645 || n = 659 || n = 680 || n = 694 || n = 722 || n = 792 || n = 799 || n = 813 || n = 827 || n = 855 || n = 862 || n = 869 || n = 890 || n = 897 || n = 904)
	; 6 bytes
	ldy #(n-8)/7
	nop
	dey
	Jpl *-2
.elseif(n = 647)
	; 6 bytes
	ldy #56 ;hides 'sec'
	nop
	iny
	Jpl *-3
.elseif(n = 672 || n = 1057)
	; 8 bytes
	ldy #(n-1)/11
	delay_n 6
	dey
	Jne *-4
.elseif(n = 684)
	; 4 bytes
	ldy #136 ;hides 'dey'
	Jne *-1
.elseif(n = 685 || n = 712 || n = 730 || n = 865 || n = 937 || n = 982 || n = 1207)
	; 7 bytes
	ldy #(n-1)/9
	delay_n 4
	dey
	Jne *-3
.elseif(n = 727)
	; 5 bytes
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
.elseif(n = 748)
	; 5 bytes
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
.elseif(n = 782 || n = 947)
	; 8 bytes
	ldy #(n-12)/11
	delay_n 6
	dey
	Jpl *-4
.elseif(n = 800)
	; 6 bytes
	ldy #216 ;hides 'cld'
	nop
	dey
	Jmi *-3
.elseif(n = 899)
	; 6 bytes
	ldy #56 ;hides 'sec'
	iny
	iny
	Jne *-3
.elseif(n = 935)
	; 6 bytes
	ldy #24 ;hides 'clc'
	nop
	iny
	Jpl *-3
.elseif(n = 958)
	; 5 bytes
	ldy #136 ;hides 'dey'
	nop
	Jne *-2
.elseif(n = 962)
	; 6 bytes
	ldy #234 ;hides 'nop'
	nop
	dey
	Jmi *-3
.elseif(n = 1052)
	; 6 bytes
	ldy #234 ;hides 'nop'
	dey
	dey
	Jne *-3
.elseif(n = 1070)
	; 6 bytes
	ldy #136 ;hides 'dey'
	iny
	iny
	Jne *-3
.elseif(n = 1115)
	; 6 bytes
	ldy #248 ;hides 'sed'
	dey
	dey
	Jne *-3
.elseif(n = 1187)
	; 7 bytes
	ldy #216 ;hides 'cld'
	nop
	dey
	dey
	Jne *-4
.elseif(n = 1232)
	; 6 bytes
	ldy #136 ;hides 'dey'
	delay_n 4
	Jne *-3
.endif
.endif
.else
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
; 1251 to 1406
.if((n .mod 5 = 1) && ((n > 1250 && n < 1277)))
	; 5 bytes
	ldy #(n-1)/5
	dey
	Jne *-1
.elseif((n .mod 9 = 1) && (n = 1252 || (n > 1287 && n < 1316) || (n > 1332 && n < 1379) || n = 1396 || n = 1405))
	; 7 bytes
	ldy #(n-1)/9
	delay_n 4
	dey
	Jne *-3
.elseif(n = 1253 || n = 1255 || n = 1258 || n = 1260 || n = 1262 || n = 1263 || n = 1265 || n = 1267 || n = 1272 || n = 1273 || n = 1277 || n = 1278 || n = 1280 || n = 1283 || n = 1285 || n = 1287 || n = 1290 || n = 1291 || n = 1293 || n = 1294 || n = 1298 || (n > 1299 && n < 1303) || n = 1305 || n = 1307 || n = 1312 || n = 1314 || n = 1316 || n = 1319 || (n > 1320 && n < 1324) || n = 1326 || n = 1328 || n = 1329 || n = 1337 || n = 1340 || n = 1344 || n = 1346 || n = 1347 || n = 1349 || n = 1350 || n = 1353 || n = 1354 || (n > 1355 && n < 1359) || n = 1361 || n = 1364 || n = 1368 || n = 1370 || n = 1371 || n = 1374 || n = 1375 || n = 1377 || n = 1379 || n = 1382 || (n > 1383 && n < 1387) || n = 1389 || (n > 1390 && n < 1394) || n = 1400 || n = 1403 || n = 1406)
	; 6..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 1254 || (n > 1267 && n < 1402)))
	; 6 bytes
	ldy #(n-1)/7
	nop
	dey
	Jne *-2
.elseif(n = 1257 || n = 1259 || n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1292 || n = 1299 || n = 1309 || n = 1311 || n = 1313 || n = 1318 || n = 1320 || n = 1327 || n = 1334 || n = 1341 || n = 1348 || n = 1355 || n = 1362 || n = 1372 || n = 1376 || n = 1383 || n = 1390 || n = 1397 || n = 1402 || n = 1404)
	; 7..9 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 1270 || n = 1335 || n = 1398)
	; 7..8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1281)
	; 5 bytes
	ldy #0
	dey
	Jne *-1
.elseif(n = 1286)
	; 7 bytes
	ldy #234 ;hides 'nop'
	nop
	dey
	dey
	Jne *-4
.elseif(n = 1295 || n = 1325)
	; 9 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 1304 || n = 1339)
	; 10 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 1308)
	; 7 bytes
	ldy #136 ;hides 'dey'
	nop
	iny
	iny
	Jne *-4
.elseif(n = 1330)
	; 7 bytes
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jmi *-4
.elseif(n = 1332)
	; 8 bytes
	ldy #(n-1)/11
	delay_n 6
	dey
	Jne *-4
.elseif(n = 1336 || n = 1381)
	; 9 bytes
	ldy #(n-16)/15
	delay_n 10
	dey
	Jpl *-5
.elseif(n = 1343 || n = 1365)
	; 8 bytes
	ldy #(n-12)/11
	delay_n 6
	dey
	Jpl *-4
.elseif(n = 1363)
	; 7 bytes
	ldy #248 ;hides 'sed'
	nop
	dey
	dey
	Jne *-4
.elseif(n = 1367)
	; 9 bytes
	ldy #56 ;hides 'sec'
	delay_n 12
	iny
	Jpl *-6
.elseif(n = 1388)
	; 10 bytes
	ldy #(n-20)/19
	nop
	delay_n 12
	dey
	Jpl *-6
.elseif(n = 1395)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 1399)
	; 5 bytes
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
.endif
.else
; 1407 to 1562
.if(n = 1407 || n = 1410 || n = 1412 || n = 1413 || n = 1416 || n = 1417 || n = 1419 || n = 1421 || n = 1424 || (n > 1425 && n < 1429) || n = 1431 || n = 1433 || n = 1434 || n = 1438 || n = 1445 || (n > 1446 && n < 1450) || n = 1452 || n = 1454 || n = 1455 || n = 1458 || n = 1462 || n = 1465 || n = 1466 || n = 1469 || n = 1470 || n = 1472 || n = 1473 || n = 1475 || n = 1476 || n = 1479 || n = 1480 || (n > 1481 && n < 1485) || n = 1487 || (n > 1488 && n < 1492) || n = 1494 || n = 1496 || n = 1500 || n = 1501 || n = 1503 || n = 1505 || n = 1508 || n = 1510 || n = 1512 || n = 1515 || n = 1517 || n = 1518 || n = 1521 || n = 1525 || n = 1526 || n = 1529 || n = 1532 || n = 1533 || n = 1535 || n = 1536 || n = 1538 || n = 1539 || n = 1542 || n = 1543 || n = 1545 || n = 1547 || n = 1550 || (n > 1551 && n < 1555) || n = 1557 || n = 1559 || n = 1560)
	; 7..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1563)))
	; 6 bytes
	ldy #(n-1)/7
	nop
	dey
	Jne *-2
.elseif(n = 1409 || n = 1420)
	; 8 bytes
	ldy #(n-12)/11
	delay_n 6
	dey
	Jpl *-4
.elseif(n = 1411 || n = 1418 || n = 1425 || n = 1435 || n = 1439 || n = 1444 || n = 1446 || n = 1453 || n = 1460 || n = 1467 || n = 1474 || n = 1481 || n = 1488 || n = 1498 || n = 1502 || n = 1509 || n = 1514 || n = 1516 || n = 1523 || n = 1530 || n = 1537 || n = 1544 || n = 1551)
	; 7..9 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n .mod 9 = 1) && ((n > 1413 && n < 1442) || (n > 1458 && n < 1505) || (n > 1521 && n < 1559)))
	; 7 bytes
	ldy #(n-1)/9
	delay_n 4
	dey
	Jne *-3
.elseif(n = 1430)
	; 10 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1437)
	; 10 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 1440)
	; 7 bytes
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
.elseif(n = 1442 || n = 1497 || n = 1519)
	; 8 bytes
	ldy #(n-1)/11
	delay_n 6
	dey
	Jne *-4
.elseif(n = 1451 || n = 1493 || n = 1528 || n = 1556)
	; 10 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 1456)
	; 9 bytes
	ldy #(n-16)/15
	delay_n 10
	dey
	Jpl *-5
.elseif(n = 1461 || n = 1524)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1463)
	; 9 bytes
	ldy #(n-18)/17
	delay_n 12
	dey
	Jpl *-5
.elseif(n = 1507)
	; 8 bytes
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	iny
	Jne *-5
.elseif(n = 1511)
	; 5 bytes
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
.elseif(n = 1546)
	; 8 bytes
	ldy #136 ;hides 'dey'
	delay_n 4
	iny
	iny
	Jne *-5
.elseif(n = 1561)
	; 9 bytes
	ldy #(n-1)/15
	delay_n 10
	dey
	Jne *-5
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
; 1563 to 1718
.if(n = 1563 || n = 1574 || n = 1596 || n = 1651 || n = 1673 || n = 1717)
	; 8 bytes
	ldy #(n-1)/11
	delay_n 6
	dey
	Jne *-4
.elseif(n = 1564 || n = 1566 || n = 1568 || n = 1571 || n = 1573 || n = 1575 || n = 1578 || n = 1580 || n = 1581 || n = 1584 || n = 1588 || n = 1589 || n = 1592 || n = 1595 || n = 1599 || n = 1601 || n = 1602 || n = 1605 || n = 1606 || (n > 1607 && n < 1611) || n = 1613 || n = 1615 || n = 1617 || n = 1620 || n = 1622 || n = 1627 || n = 1629 || n = 1631 || n = 1634 || n = 1636 || n = 1638 || n = 1641 || (n > 1642 && n < 1646) || n = 1652 || n = 1655 || n = 1658 || n = 1659 || n = 1661 || n = 1662 || n = 1664 || n = 1665 || n = 1668 || n = 1669 || n = 1671 || n = 1672 || n = 1676 || (n > 1677 && n < 1681) || n = 1683 || n = 1685 || n = 1686 || n = 1689 || n = 1690 || n = 1692 || n = 1694 || n = 1697 || n = 1699 || n = 1700 || n = 1703 || n = 1704 || (n > 1705 && n < 1709) || n = 1714 || n = 1715 || n = 1718)
	; 7..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1565 || n = 1570 || n = 1572 || n = 1577 || n = 1579 || n = 1586 || n = 1593 || n = 1600 || n = 1607 || n = 1614 || n = 1624 || n = 1626 || n = 1628 || n = 1635 || n = 1640 || n = 1642 || n = 1649 || n = 1656 || n = 1663 || n = 1670 || n = 1677 || n = 1687 || n = 1691 || n = 1698 || n = 1705 || n = 1712)
	; 7..10 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n .mod 9 = 1) && (n = 1567 || (n > 1584 && n < 1631) || (n > 1647 && n < 1694) || n = 1711))
	; 7 bytes
	ldy #(n-1)/9
	delay_n 4
	dey
	Jne *-3
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1717)))
	; 6 bytes
	ldy #(n-1)/7
	nop
	dey
	Jne *-2
.elseif(n = 1582 || n = 1616 || n = 1633 || n = 1701)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 1587 || n = 1598 || n = 1650 || n = 1713)
	; 8..9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1591)
	; 9 bytes
	ldy #(n-1)/15
	delay_n 10
	dey
	Jne *-5
.elseif(n = 1619)
	; 9 bytes
	ldy #216 ;hides 'cld'
	delay_n 6
	dey
	dey
	Jne *-6
.elseif(n = 1623)
	; 5 bytes
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
.elseif(n = 1637)
	; 5 bytes
	ldy #234 ;hides 'nop'
	dey
	Jne *-2
.elseif(n = 1647)
	; 9 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1654)
	; 10 bytes
	ldy #(n-20)/19
	nop
	delay_n 12
	dey
	Jpl *-6
.elseif(n = 1682 || n = 1710)
	; 10 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 1696)
	; 9 bytes
	ldy #(n-16)/15
	delay_n 10
	dey
	Jpl *-5
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1726 || n = 1733 || n = 1740 || n = 1754 || n = 1759 || n = 1761 || n = 1768 || n = 1775 || n = 1782 || n = 1789 || n = 1796)
	; 8..9 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n .mod 9 = 1) && ((n > 1719 && n < 1757) || (n > 1773 && n < 1793)))
	; 7 bytes
	ldy #(n-1)/9
	delay_n 4
	dey
	Jne *-3
.elseif(n = 1721 || n = 1722 || n = 1724 || n = 1725 || n = 1727 || n = 1731 || n = 1732 || n = 1734 || n = 1736 || n = 1739 || (n > 1740 && n < 1744) || n = 1746 || n = 1748 || n = 1749 || n = 1753 || n = 1755 || n = 1757 || n = 1760 || (n > 1761 && n < 1765) || n = 1767 || n = 1769 || n = 1770 || n = 1773 || n = 1777 || n = 1778 || n = 1781 || n = 1784 || n = 1785 || n = 1787 || n = 1788 || n = 1791 || n = 1795)
	; 7..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1787)))
	; 6 bytes
	ldy #(n-1)/7
	nop
	dey
	Jne *-2
.elseif(n = 1728 || n = 1750 || n = 1794)
	; 8 bytes
	ldy #(n-1)/11
	delay_n 6
	dey
	Jne *-4
.elseif(n = 1735)
	; 5 bytes
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
.elseif(n = 1745)
	; 9 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1752)
	; 9 bytes
	ldy #(n-18)/17
	delay_n 12
	dey
	Jpl *-5
.elseif(n = 1766)
	; 10 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 1771)
	; 9 bytes
	ldy #(n-16)/15
	delay_n 10
	dey
	Jpl *-5
.elseif(n = 1776)
	; 8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1780)
	; 8 bytes
	ldy #136 ;hides 'dey'
	delay_n 8
	Jne *-5
.elseif(n = 1790)
	; 6 bytes
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
.elseif(n = 1793)
	; 6 bytes
	ldy #0
	nop
	dey
	Jne *-2
.endif
.else
; 1797 to 1875
.if(n = 1797 || n = 1798 || n = 1800 || n = 1803 || n = 1807 || n = 1812 || n = 1814 || n = 1815 || n = 1821 || n = 1823 || n = 1824 || n = 1829 || n = 1830 || n = 1832 || n = 1833 || n = 1836 || n = 1839 || (n > 1840 && n < 1844) || n = 1848 || n = 1850 || n = 1851 || n = 1853 || n = 1856 || n = 1857 || n = 1859 || n = 1862 || n = 1863 || n = 1865 || n = 1866 || (n > 1867 && n < 1871) || n = 1875)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1799)
	; 6 bytes
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
.elseif((n .mod 9 = 1) && ((n > 1800 && n < 1874)))
	; 7 bytes
	ldy #(n-1)/9
	delay_n 4
	dey
	Jne *-3
.elseif(n = 1802 || n = 1804 || n = 1813 || n = 1822 || n = 1840 || n = 1852 || n = 1858 || n = 1867 || n = 1874)
	; 8..10 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n .mod 11 = 1) && ((n > 1804 && n < 1872)))
	; 8 bytes
	ldy #(n-1)/11
	delay_n 6
	dey
	Jne *-4
.elseif(n = 1806 || n = 1825)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 1808 || n = 1834 || n = 1847)
	; 9 bytes
	ldy #(n-1)/13
	delay_n 8
	dey
	Jne *-5
.elseif(n = 1809)
	; 10 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1811)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1817)
	; 11 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 1818)
	; 9 bytes
	ldy #234 ;hides 'nop'
	delay_n 10
	dey
	Jmi *-6
.elseif(n = 1820 || n = 1854)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 1826)
	; 10 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 1831 || n = 1861)
	; 9 bytes
	ldy #(n-16)/15
	delay_n 10
	dey
	Jpl *-5
.elseif(n = 1835)
	; 10 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 1844)
	; 10 bytes
	ldy #(n-20)/19
	nop
	delay_n 12
	dey
	Jpl *-6
.elseif(n = 1845)
	; 11 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 1872)
	; 11 bytes
	delay_n 86
	delay_n (n-86)
.endif
.endif
.endif
.endif
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
; 1876 to 2031
.if(n = 1876 || n = 1906 || n = 1921)
	; 9 bytes
	ldy #(n-16)/15
	delay_n 10
	dey
	Jpl *-5
.elseif((n > 1876 && n < 1881) || n = 1884 || n = 1886 || n = 1887 || n = 1890 || n = 1896 || n = 1897 || n = 1902 || n = 1907 || n = 1908 || n = 1910 || n = 1911 || n = 1913 || n = 1914 || n = 1917 || n = 1919 || n = 1920 || n = 1923 || n = 1924 || n = 1928 || n = 1929 || (n > 1930 && n < 1934) || n = 1938 || (n > 1939 && n < 1943) || n = 1947 || n = 1949 || n = 1950 || n = 1952 || n = 1953 || n = 1956 || n = 1958 || n = 1960 || n = 1961 || n = 1965 || (n > 1966 && n < 1970) || n = 1974 || n = 1976 || n = 1978 || n = 1979 || n = 1983 || (n > 1984 && n < 1988) || n = 1995 || n = 1997 || n = 1998 || n = 2000 || n = 2001 || n = 2004 || n = 2005 || n = 2009 || n = 2010 || n = 2012 || n = 2013 || n = 2019 || (n > 2020 && n < 2024) || n = 2027 || n = 2028 || n = 2030 || n = 2031)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1881)
	; 10 bytes
	delay_n 146
	delay_n (n-146)
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 2027)))
	; 7 bytes
	ldy #(n-1)/9
	delay_n 4
	dey
	Jne *-3
.elseif(n = 1883 || n = 1892 || n = 1982 || n = 1991)
	; 11 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1885 || n = 1894 || n = 1903 || n = 1912 || n = 1930 || n = 1946 || n = 1957 || n = 1962 || n = 1975 || n = 1980 || n = 1984 || n = 1993 || n = 2002 || n = 2006 || n = 2020 || n = 2029)
	; 8..11 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 1888 || n = 1905 || n = 2024)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 1889)
	; 11 bytes
	delay_n 96
	delay_n (n-96)
.elseif((n .mod 11 = 1) && ((n > 1892 && n < 1971) || (n > 1991 && n < 2026)))
	; 8 bytes
	ldy #(n-1)/11
	delay_n 6
	dey
	Jne *-4
.elseif(n = 1895 || n = 1994 || n = 2018)
	; 9..10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1898)
	; 11 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 1899 || n = 1925 || n = 1964 || n = 1977 || n = 2016)
	; 9 bytes
	ldy #(n-1)/13
	delay_n 8
	dey
	Jne *-5
.elseif(n = 1901 || n = 2015)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 1916)
	; 10 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 1922 || n = 1939 || n = 1973 || n = 2007)
	; 9 bytes
	ldy #(n-18)/17
	delay_n 12
	dey
	Jpl *-5
.elseif(n = 1934)
	; 11 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 1935)
	; 11 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 1943)
	; 6 bytes
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
.elseif(n = 1944)
	; 11 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 1951 || n = 1966 || n = 1996 || n = 2011)
	; 9 bytes
	ldy #(n-1)/15
	delay_n 10
	dey
	Jne *-5
.elseif(n = 1955)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 1971)
	; 10 bytes
	ldy #24 ;hides 'clc'
	delay_n 8
	iny
	iny
	Jne *-7
.elseif(n = 1988)
	; 10 bytes
	ldy #234 ;hides 'nop'
	delay_n 8
	dey
	dey
	Jne *-7
.elseif(n = 1989)
	; 11 bytes
	delay_n 196
	delay_n (n-196)
.endif
.else
; 2032 to 2187
.if(n = 2032)
	; 9 bytes
	ldy #234 ;hides 'nop'
	delay_n 12
	dey
	Jmi *-6
.elseif(n = 2033 || n = 2037 || n = 2039 || n = 2040 || n = 2043 || n = 2045 || n = 2046 || n = 2048 || n = 2049 || n = 2051 || n = 2052 || n = 2055 || n = 2057 || n = 2059 || n = 2060 || n = 2064 || n = 2066 || n = 2067 || n = 2070 || n = 2073 || n = 2076 || n = 2077 || n = 2079 || n = 2082 || n = 2084 || n = 2085 || n = 2088 || n = 2095 || n = 2096 || n = 2100 || n = 2103 || n = 2104 || n = 2106 || n = 2109 || n = 2111 || n = 2112 || n = 2114 || n = 2115 || n = 2118 || (n > 2119 && n < 2124) || n = 2127 || n = 2129 || n = 2130 || n = 2132 || n = 2136 || (n > 2137 && n < 2142) || n = 2145 || n = 2147 || n = 2148 || n = 2150 || n = 2151 || n = 2154 || n = 2156 || n = 2158 || n = 2162 || n = 2163 || n = 2165 || n = 2166 || n = 2169 || n = 2172 || n = 2174 || n = 2175 || n = 2178 || n = 2181 || n = 2183 || n = 2184 || n = 2187)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2034 || n = 2072)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif((n .mod 9 = 1) && ((n > 2034 && n < 2180)))
	; 7 bytes
	ldy #(n-1)/9
	delay_n 4
	dey
	Jne *-3
.elseif((n .mod 11 = 1) && ((n > 2035 && n < 2070) || (n > 2090 && n < 2169)))
	; 8 bytes
	ldy #(n-1)/11
	delay_n 6
	dey
	Jne *-4
.elseif(n = 2038 || n = 2050 || n = 2061 || n = 2065 || n = 2074 || n = 2078 || n = 2083 || n = 2090 || n = 2108 || n = 2110 || n = 2119 || n = 2128 || n = 2137 || n = 2149 || n = 2155 || n = 2164 || n = 2171 || n = 2173 || n = 2180 || n = 2182)
	; 8..11 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 2041 || n = 2056 || n = 2086 || n = 2101 || n = 2131 || n = 2176)
	; 9 bytes
	ldy #(n-1)/15
	delay_n 10
	dey
	Jne *-5
.elseif(n = 2042 || n = 2068 || n = 2081 || n = 2094 || n = 2133 || n = 2159 || n = 2185)
	; 9 bytes
	ldy #(n-1)/13
	delay_n 8
	dey
	Jne *-5
.elseif(n = 2054)
	; 9 bytes
	ldy #136 ;hides 'dey'
	.repeat 5
	 nop
	.endrepeat
	Jne *-6
.elseif(n = 2063 || n = 2097 || n = 2144 || n = 2153)
	; 10..11 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2075 || n = 2092 || n = 2126 || n = 2160 || n = 2177)
	; 9 bytes
	ldy #(n-18)/17
	delay_n 12
	dey
	Jpl *-5
.elseif(n = 2087)
	; 6 bytes
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
.elseif(n = 2093)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2099)
	; 10 bytes
	ldy #56 ;hides 'sec'
	delay_n 12
	iny
	iny
	Jne *-7
.elseif(n = 2105)
	; 6 bytes
	ldy #234 ;hides 'nop'
	nop
	dey
	Jne *-3
.elseif(n = 2117)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2142)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 2167 || n = 2186)
	; 10 bytes
	ldy #(n-20)/19
	nop
	delay_n 12
	dey
	Jpl *-6
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
; 2188 to 2343
.if((n .mod 9 = 1) && ((n > 2187 && n < 2297)))
	; 7 bytes
	ldy #(n-1)/9
	delay_n 4
	dey
	Jne *-3
.elseif(n = 2189 || n = 2207 || n = 2241 || n = 2261 || n = 2288 || n = 2297 || n = 2306)
	; 10..11 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 11 = 1) && ((n > 2189 && n < 2268) || (n > 2288 && n < 2334)))
	; 8 bytes
	ldy #(n-1)/11
	delay_n 6
	dey
	Jne *-4
.elseif(n = 2191 || n = 2221 || n = 2236 || n = 2266 || n = 2281 || n = 2326 || n = 2341)
	; 9 bytes
	ldy #(n-1)/15
	delay_n 10
	dey
	Jne *-5
.elseif(n = 2192 || n = 2291 || n = 2304)
	; 9..10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2193 || n = 2195 || n = 2196 || n = 2203 || n = 2208 || n = 2210 || n = 2213 || n = 2214 || n = 2216 || n = 2217 || n = 2219 || n = 2220 || n = 2225 || n = 2226 || n = 2229 || n = 2230 || n = 2232 || n = 2235 || (n > 2236 && n < 2241) || n = 2244 || n = 2246 || n = 2247 || n = 2249 || n = 2252 || n = 2253 || n = 2255 || n = 2257 || n = 2258 || n = 2262 || n = 2264 || n = 2265 || n = 2268 || n = 2271 || (n > 2272 && n < 2276) || n = 2280 || (n > 2281 && n < 2286) || (n > 2291 && n < 2295) || n = 2298 || n = 2301 || n = 2307 || n = 2309 || n = 2310 || n = 2312 || n = 2317 || n = 2321 || n = 2324 || n = 2327 || n = 2332 || n = 2334 || n = 2335 || n = 2337 || n = 2339 || n = 2340 || n = 2343)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2194)
	; 9 bytes
	ldy #(n-18)/17
	delay_n 12
	dey
	Jpl *-5
.elseif(n = 2198 || n = 2250 || n = 2276 || n = 2302 || n = 2315 || n = 2328)
	; 9 bytes
	ldy #(n-1)/13
	delay_n 8
	dey
	Jne *-5
.elseif(n = 2199)
	; 7 bytes
	ldy #56 ;hides 'sec'
	delay_n 4
	iny
	Jne *-4
.elseif(n = 2200 || n = 2202 || n = 2204 || n = 2209 || n = 2218 || n = 2227 || n = 2248 || n = 2254 || n = 2259 || n = 2263 || n = 2270 || n = 2272 || n = 2290 || n = 2299 || n = 2303 || n = 2308 || n = 2314 || n = 2316 || n = 2318 || n = 2325 || n = 2329 || n = 2331 || n = 2336)
	; 9..11 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 2205 || n = 2243 || n = 2319 || n = 2338)
	; 10 bytes
	ldy #(n-20)/19
	nop
	delay_n 12
	dey
	Jpl *-6
.elseif(n = 2211 || n = 2228 || n = 2279 || n = 2313 || n = 2330)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 2222)
	; 10 bytes
	ldy #234 ;hides 'nop'
	delay_n 10
	dey
	dey
	Jne *-7
.elseif(n = 2231)
	; 6 bytes
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
.elseif(n = 2277 || n = 2295)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 2286)
	; 11 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 2305)
	; 7 bytes
	ldy #0
	delay_n 4
	dey
	Jne *-3
.elseif(n = 2320)
	; 12 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 2323)
	; 12 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 2342)
	; 11 bytes
	delay_n 111
	delay_n (n-111)
.endif
.else
.if((n >= 2344) && (n < 2422))
; 2344 to 2421
.if((n .mod 11 = 1) && ((n > 2343 && n < 2422)))
	; 8 bytes
	ldy #(n-1)/11
	delay_n 6
	dey
	Jne *-4
.elseif(n = 2345 || n = 2346 || n = 2348 || n = 2349 || n = 2351 || (n > 2356 && n < 2362) || n = 2368 || n = 2370 || n = 2372 || n = 2373 || n = 2379 || n = 2382 || n = 2383 || n = 2385 || n = 2390 || n = 2392 || n = 2394 || n = 2397 || n = 2400 || n = 2403 || n = 2405 || n = 2408 || n = 2412 || n = 2417 || n = 2418 || n = 2420)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347 || n = 2364 || n = 2381 || n = 2398 || n = 2415)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 2350 || n = 2369 || n = 2374 || n = 2378 || n = 2384 || n = 2389 || n = 2391 || n = 2396 || n = 2402 || n = 2404 || n = 2409 || n = 2413)
	; 9..11 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 2352)
	; 11 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 2353)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif((n .mod 13 = 1) && ((n > 2353 && n < 2420)))
	; 9 bytes
	ldy #(n-1)/13
	delay_n 8
	dey
	Jne *-5
.elseif((n .mod 15 = 1) && ((n > 2355 && n < 2417)))
	; 9 bytes
	ldy #(n-1)/15
	delay_n 10
	dey
	Jne *-5
.elseif(n = 2362)
	; 11 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 2363)
	; 12 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 2365)
	; 12 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2375)
	; 7 bytes
	ldy #216 ;hides 'cld'
	delay_n 4
	dey
	Jne *-4
.elseif(n = 2376 || n = 2414)
	; 10 bytes
	ldy #(n-20)/19
	nop
	delay_n 12
	dey
	Jpl *-6
.elseif(n = 2387)
	; 11 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 2395)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 2407)
	; 11 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 2411)
	; 12 bytes
	delay_n 106
	delay_n (n-106)
.endif
.else
; 2422 to 2500
.if(n = 2422 || n = 2424 || n = 2435 || n = 2455 || n = 2457 || n = 2464 || n = 2474 || n = 2479 || n = 2494)
	; 10..12 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 2423 || (n > 2424 && n < 2428) || n = 2433 || n = 2434 || n = 2436 || n = 2438 || n = 2448 || n = 2450 || n = 2451 || n = 2453 || n = 2456 || n = 2459 || n = 2460 || n = 2462 || n = 2463 || (n > 2466 && n < 2471) || n = 2473 || n = 2475 || n = 2478 || (n > 2479 && n < 2483) || n = 2485 || n = 2486 || n = 2489 || n = 2492 || n = 2493 || n = 2495 || n = 2496 || n = 2499)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2428)
	; 12 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 2429)
	; 12 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 2430 || n = 2439)
	; 12 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 2431 || n = 2446 || n = 2461 || n = 2491)
	; 9 bytes
	ldy #(n-1)/15
	delay_n 10
	dey
	Jne *-5
.elseif((n .mod 11 = 1) && ((n > 2431 && n < 2499)))
	; 8 bytes
	ldy #(n-1)/11
	delay_n 6
	dey
	Jne *-4
.elseif(n = 2437)
	; 11 bytes
	ldy #(n-22)/21
	delay_n 4
	delay_n 12
	dey
	Jpl *-7
.elseif(n = 2440)
	; 12 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 2441)
	; 12 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 2442)
	; 12 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2444)
	; 12 bytes
	delay_n 166
	delay_n (n-166)
.elseif((n .mod 13 = 1) && ((n > 2444 && n < 2498)))
	; 9 bytes
	ldy #(n-1)/13
	delay_n 8
	dey
	Jne *-5
.elseif(n = 2447)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2449 || n = 2466 || n = 2483 || n = 2500)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 2452)
	; 10 bytes
	ldy #(n-20)/19
	nop
	delay_n 12
	dey
	Jpl *-6
.elseif(n = 2472)
	; 12 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 2477)
	; 12 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 2488)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 10
	dey
	dey
	Jne *-7
.elseif(n = 2490)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
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
; 2501 to 2812
.if(n = 2501 || n = 2503 || n = 2512 || n = 2526 || n = 2537 || n = 2539 || n = 2545 || n = 2550 || n = 2552 || n = 2554 || n = 2556 || n = 2565 || n = 2567 || n = 2569 || n = 2571 || n = 2576 || n = 2578 || n = 2584 || n = 2589 || n = 2600 || n = 2605 || n = 2617 || n = 2622 || n = 2633 || n = 2639 || n = 2644 || n = 2650 || n = 2659 || n = 2664 || n = 2669 || n = 2677 || n = 2690 || n = 2695 || n = 2710 || n = 2719 || n = 2724 || n = 2730 || n = 2732 || n = 2734 || n = 2741 || n = 2743 || n = 2747 || n = 2749 || n = 2754 || n = 2758 || n = 2760 || n = 2765 || n = 2779 || n = 2787 || n = 2792 || n = 2798 || n = 2812)
	; 9..13 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 2502 || n = 2504 || n = 2505 || n = 2507 || n = 2508 || n = 2511 || (n > 2512 && n < 2517) || n = 2519 || n = 2522 || n = 2524 || n = 2525 || n = 2527 || n = 2529 || n = 2530 || n = 2532 || n = 2533 || n = 2535 || n = 2538 || n = 2541 || n = 2544 || n = 2546 || n = 2548 || n = 2555 || (n > 2556 && n < 2561) || n = 2570 || n = 2572 || n = 2577 || n = 2579 || n = 2580 || n = 2582 || n = 2583 || n = 2587 || (n > 2590 && n < 2594) || n = 2598 || n = 2603 || n = 2607 || n = 2610 || n = 2612 || n = 2613 || n = 2615 || n = 2616 || n = 2618 || n = 2621 || n = 2624 || n = 2625 || n = 2628 || n = 2629 || n = 2631 || n = 2632 || n = 2634 || n = 2635 || n = 2637 || n = 2638 || n = 2643 || n = 2645 || n = 2646 || n = 2648 || n = 2649 || n = 2654 || n = 2655 || n = 2657 || n = 2658 || n = 2660 || n = 2662 || n = 2665 || n = 2667 || n = 2668 || n = 2672 || n = 2673 || n = 2675 || n = 2676 || n = 2678 || (n > 2680 && n < 2684) || n = 2688 || n = 2689 || n = 2691 || n = 2693 || n = 2694 || n = 2698 || n = 2700 || n = 2702 || n = 2703 || n = 2706 || n = 2708 || n = 2709 || (n > 2710 && n < 2715) || n = 2720 || n = 2722 || n = 2723 || n = 2725 || n = 2726 || n = 2733 || n = 2735 || n = 2739 || n = 2742 || n = 2745 || n = 2748 || n = 2753 || n = 2759 || n = 2763 || n = 2764 || (n > 2765 && n < 2770) || n = 2774 || n = 2775 || n = 2777 || n = 2778 || n = 2780 || n = 2781 || n = 2785 || n = 2786 || n = 2788 || n = 2790 || n = 2793 || n = 2797 || (n > 2798 && n < 2803) || n = 2808 || n = 2810 || n = 2811)
	; 9..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 15 = 1) && ((n > 2505 && n < 2537) || (n > 2565 && n < 2627) || (n > 2655 && n < 2792)))
	; 9 bytes
	ldy #(n-1)/15
	delay_n 10
	dey
	Jne *-5
.elseif((n .mod 11 = 1) && ((n > 2508 && n < 2807)))
	; 8 bytes
	ldy #(n-1)/11
	delay_n 6
	dey
	Jne *-4
.elseif((n .mod 13 = 1) && (n = 2510 || n = 2523 || n = 2549 || n = 2562 || (n > 2587 && n < 2641) || (n > 2665 && n < 2706) || (n > 2743 && n < 2810)))
	; 9 bytes
	ldy #(n-1)/13
	delay_n 8
	dey
	Jne *-5
.elseif(n = 2517 || n = 2534 || n = 2568 || n = 2585 || n = 2602 || n = 2636 || n = 2653 || n = 2670 || n = 2687 || n = 2704 || n = 2721 || n = 2738 || n = 2755 || n = 2772 || n = 2789)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 2518 || n = 2543 || n = 2609 || n = 2620 || n = 2697)
	; 12..13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2528 || n = 2547 || n = 2604 || n = 2623 || n = 2642 || n = 2661 || n = 2680 || n = 2699 || n = 2737 || n = 2756 || n = 2794)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 2540)
	; 10 bytes
	ldy #248 ;hides 'sed'
	nop
	delay_n 12
	dey
	Jmi *-7
.elseif(n = 2551)
	; 7 bytes
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	Jne *-4
.elseif(n = 2561 || n = 2574 || n = 2595 || n = 2651 || n = 2684 || n = 2717 || n = 2728 || n = 2771 || n = 2805)
	; 11..13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2563)
	; 11 bytes
	ldy #(n-22)/21
	delay_n 4
	delay_n 12
	dey
	Jpl *-7
.elseif(n = 2573)
	; 7 bytes
	ldy #234 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
.elseif(n = 2590 || n = 2606)
	; 10..11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2594)
	; 12 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 2599)
	; 8 bytes
	ldy #56 ;hides 'sec'
	delay_n 6
	iny
	Jne *-5
.elseif(n = 2647 || n = 2752)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 2715)
	; 12 bytes
	ldy #(n-24)/23
	delay_n 6
	delay_n 12
	dey
	Jpl *-8
.elseif(n = 2727)
	; 7 bytes
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jne *-4
.elseif(n = 2736)
	; 11 bytes
	ldy #136 ;hides 'dey'
	nop
	delay_n 12
	iny
	iny
	Jne *-8
.elseif(n = 2750)
	; 10 bytes
	ldy #136 ;hides 'dey'
	delay_n 12
	dey
	dey
	Jne *-7
.elseif(n = 2782)
	; 11 bytes
	ldy #248 ;hides 'sed'
	delay_n 4
	delay_n 12
	dey
	Jmi *-8
.elseif(n = 2803)
	; 12 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 2804)
	; 13 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 2807)
	; 8 bytes
	ldy #216 ;hides 'cld'
	delay_n 6
	dey
	Jne *-5
.endif
.else
.if((n >= 2813) && (n < 2969))
; 2813 to 2968
.if(n = 2813 || n = 2832 || n = 2870 || n = 2889 || n = 2927 || n = 2946)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 2814 || n = 2819 || n = 2824 || n = 2825 || n = 2827 || n = 2828 || n = 2834 || n = 2837 || n = 2838 || n = 2841 || n = 2842 || (n > 2843 && n < 2848) || n = 2850 || n = 2852 || n = 2853 || n = 2855 || n = 2856 || n = 2859 || n = 2862 || n = 2863 || n = 2865 || n = 2867 || n = 2868 || n = 2871 || n = 2872 || n = 2875 || n = 2876 || (n > 2877 && n < 2881) || n = 2883 || n = 2885 || n = 2886 || n = 2893 || n = 2895 || n = 2897 || n = 2898 || n = 2901 || n = 2902 || (n > 2903 && n < 2907) || n = 2910 || n = 2912 || n = 2915 || (n > 2916 && n < 2920) || n = 2928 || (n > 2929 && n < 2934) || n = 2943 || n = 2944 || n = 2947 || n = 2948 || n = 2950 || n = 2954 || n = 2957 || n = 2958 || n = 2960 || n = 2961 || n = 2963 || n = 2964 || n = 2967)
	; 9..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2815 || n = 2920)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 2816 || n = 2820 || n = 2826 || n = 2839 || n = 2843 || n = 2854 || n = 2860 || n = 2864 || n = 2869 || n = 2873 || n = 2877 || n = 2884 || n = 2890 || n = 2892 || n = 2894 || n = 2899 || n = 2903 || n = 2914 || n = 2916 || n = 2929 || n = 2945 || n = 2949 || n = 2955 || n = 2962 || n = 2968)
	; 10..12 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 2817)
	; 8 bytes
	ldy #0
	delay_n 6
	dey
	Jne *-4
.elseif(n = 2818 || n = 2829 || n = 2937 || n = 2938 || n = 2953)
	; 12..13 bytes
	delay_n 12
	delay_n (n-12)
.elseif((n .mod 15 = 1) && ((n > 2820 && n < 2957)))
	; 9 bytes
	ldy #(n-1)/15
	delay_n 10
	dey
	Jne *-5
.elseif((n .mod 13 = 1) && ((n > 2821 && n < 2862) || (n > 2886 && n < 2914) || (n > 2938 && n < 2966)))
	; 9 bytes
	ldy #(n-1)/13
	delay_n 8
	dey
	Jne *-5
.elseif(n = 2823 || n = 2840 || n = 2857 || n = 2874 || n = 2891 || n = 2908 || n = 2925 || n = 2942 || n = 2959)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 2830 || n = 2922)
	; 12 bytes
	ldy #(n-24)/23
	delay_n 6
	delay_n 12
	dey
	Jpl *-8
.elseif(n = 2831 || n = 2921 || n = 2935 || n = 2936 || n = 2966)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2833 || n = 2940)
	; 12..13 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 2849 || n = 2882)
	; 13 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 2858)
	; 12 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 2888)
	; 12 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 2907)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 2909)
	; 13 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 2923)
	; 12 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 2924)
	; 12 bytes
	ldy #234 ;hides 'nop'
	delay_n 4
	delay_n 12
	dey
	dey
	Jne *-9
.elseif(n = 2934)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 2951)
	; 13 bytes
	ldy #117
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jpl *-9
.endif
.else
; 2969 to 3125
.if(n = 2969 || n = 2970 || n = 2973 || n = 2975 || n = 2977 || n = 2980 || n = 2982 || n = 2985 || n = 2988 || n = 2990 || n = 2992 || n = 2995 || n = 2997 || n = 2998 || n = 3000 || n = 3003 || n = 3005 || n = 3006 || n = 3008 || n = 3009 || n = 3012 || n = 3014 || (n > 3017 && n < 3022) || n = 3023 || n = 3024 || n = 3026 || n = 3029 || n = 3032 || n = 3033 || (n > 3034 && n < 3039) || n = 3045 || n = 3048 || (n > 3049 && n < 3053) || n = 3058 || n = 3062 || n = 3063 || n = 3065 || n = 3066 || n = 3068 || n = 3071 || (n > 3072 && n < 3076) || n = 3080 || n = 3083 || n = 3084 || n = 3086 || n = 3087 || n = 3090 || n = 3093 || n = 3096 || n = 3097 || n = 3099 || n = 3100 || n = 3102 || n = 3110 || n = 3113 || n = 3114 || n = 3116 || n = 3118 || n = 3119 || n = 3123 || n = 3125)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 15 = 1) && ((n > 2970 && n < 3122)))
	; 9 bytes
	ldy #(n-1)/15
	delay_n 10
	dey
	Jne *-5
.elseif(n = 2972)
	; 13 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 2974 || n = 2979 || n = 2981 || n = 2987 || n = 2989 || n = 2994 || n = 2996 || n = 3002 || n = 3007 || n = 3013 || n = 3034 || n = 3047 || n = 3049 || n = 3059 || n = 3064 || n = 3070 || n = 3072 || n = 3081 || n = 3085 || n = 3094 || n = 3101 || n = 3109 || n = 3111 || n = 3115 || n = 3120 || n = 3124)
	; 11..13 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 2976 || n = 2993 || n = 3010 || n = 3027 || n = 3044 || n = 3078 || n = 3095 || n = 3112)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif((n .mod 13 = 1) && ((n > 2977 && n < 3083) || n = 3108))
	; 9 bytes
	ldy #(n-1)/13
	delay_n 8
	dey
	Jne *-5
.elseif(n = 2983 || n = 3025 || n = 3067 || n = 3088)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 2984)
	; 9 bytes
	ldy #200 ;hides 'iny'
	delay_n 6
	dey
	dey
	Jne *-6
.elseif(n = 2999)
	; 9 bytes
	ldy #56 ;hides 'sec'
	delay_n 8
	iny
	Jne *-6
.elseif(n = 3011 || n = 3040 || n = 3054 || n = 3092 || n = 3105 || n = 3122)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3015)
	; 8 bytes
	ldy #24 ;hides 'clc'
	delay_n 6
	iny
	Jne *-5
.elseif(n = 3022 || n = 3060 || n = 3079 || n = 3098 || n = 3117)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 3028 || n = 3039 || n = 3053 || n = 3103 || n = 3107)
	; 12..13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3041)
	; 8 bytes
	ldy #234 ;hides 'nop'
	delay_n 6
	dey
	Jne *-5
.elseif(n = 3042 || n = 3055 || n = 3057 || n = 3089)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 3077)
	; 13 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 3104)
	; 13 bytes
	ldy #107
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
; 3126 to 3281
.if((n > 3125 && n < 3129) || n = 3131 || n = 3133 || n = 3135 || n = 3138 || (n > 3139 && n < 3144) || (n > 3147 && n < 3151) || n = 3152 || n = 3153 || n = 3156 || n = 3157 || n = 3159 || n = 3162 || n = 3164 || n = 3165 || n = 3167 || n = 3168 || n = 3170 || n = 3171 || (n > 3174 && n < 3180) || n = 3182 || n = 3183 || n = 3185 || n = 3187 || n = 3188 || (n > 3189 && n < 3193) || n = 3195 || n = 3198 || n = 3201 || (n > 3202 && n < 3206) || n = 3213 || n = 3215 || n = 3216 || (n > 3217 && n < 3221) || n = 3227 || n = 3228 || n = 3230 || n = 3232 || n = 3233 || (n > 3234 && n < 3238) || n = 3240 || n = 3242 || n = 3243 || (n > 3244 && n < 3248) || n = 3253 || n = 3255 || n = 3257 || n = 3258 || (n > 3259 && n < 3264) || n = 3266 || n = 3267 || n = 3270 || n = 3273 || n = 3275 || n = 3276 || n = 3279 || n = 3281)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3129 || n = 3146 || n = 3163 || n = 3180 || n = 3197 || n = 3214 || n = 3231 || n = 3248 || n = 3265)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 3130 || n = 3172)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 3132 || n = 3137 || n = 3139 || n = 3154 || n = 3158 || n = 3169 || n = 3184 || n = 3189 || n = 3200 || n = 3202 || n = 3217 || n = 3229 || n = 3234 || n = 3244 || n = 3254 || n = 3259 || n = 3268 || n = 3272 || n = 3274 || n = 3280)
	; 11..12 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n .mod 13 = 1) && ((n > 3133 && n < 3278)))
	; 9 bytes
	ldy #(n-1)/13
	delay_n 8
	dey
	Jne *-5
.elseif((n .mod 15 = 1) && ((n > 3135 && n < 3272)))
	; 9 bytes
	ldy #(n-1)/15
	delay_n 10
	dey
	Jne *-5
.elseif(n = 3144 || n = 3194)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3145 || n = 3210)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 3155 || n = 3174 || n = 3193 || n = 3250 || n = 3269)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 3161 || n = 3206 || n = 3207 || n = 3222 || n = 3224)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3208 || n = 3209)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3221)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 3223)
	; 8 bytes
	ldy #248 ;hides 'sed'
	delay_n 6
	dey
	Jne *-5
.elseif(n = 3239)
	; 9 bytes
	ldy #216 ;hides 'cld'
	delay_n 8
	dey
	Jne *-6
.elseif(n = 3249)
	; 13 bytes
	ldy #112
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 3252)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3278)
	; 13 bytes
	ldy #112
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jpl *-9
.endif
.else
; 3282 to 3437
.if(n = 3282 || n = 3299 || n = 3333 || n = 3350 || n = 3367 || n = 3384 || n = 3401 || n = 3418 || n = 3435)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 3283 || n = 3284 || n = 3287 || n = 3288 || n = 3291 || n = 3292 || (n > 3293 && n < 3297) || n = 3300 || n = 3305 || n = 3308 || n = 3309 || n = 3311 || n = 3312 || n = 3315 || n = 3318 || (n > 3319 && n < 3324) || n = 3328 || n = 3330 || n = 3335 || (n > 3336 && n < 3340) || n = 3342 || n = 3344 || n = 3347 || n = 3348 || n = 3351 || n = 3352 || (n > 3353 && n < 3358) || n = 3363 || n = 3365 || n = 3366 || n = 3368 || n = 3369 || (n > 3370 && n < 3375) || n = 3378 || n = 3380 || n = 3381 || n = 3385 || n = 3386 || (n > 3387 && n < 3391) || n = 3393 || (n > 3394 && n < 3399) || n = 3403 || n = 3405 || n = 3407 || n = 3408 || (n > 3409 && n < 3413) || n = 3420 || n = 3422 || n = 3423 || (n > 3424 && n < 3428) || n = 3437)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3285 || n = 3289 || n = 3293 || n = 3302 || n = 3304 || n = 3306 || n = 3310 || n = 3319 || n = 3332 || n = 3334 || n = 3336 || n = 3349 || n = 3353 || n = 3370 || n = 3379 || n = 3387 || n = 3394 || n = 3404 || n = 3409 || n = 3424)
	; 11..12 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n .mod 15 = 1) && ((n > 3285 && n < 3437)))
	; 9 bytes
	ldy #(n-1)/15
	delay_n 10
	dey
	Jne *-5
.elseif(n = 3290 || n = 3303)
	; 9 bytes
	ldy #(n-1)/13
	delay_n 8
	dey
	Jne *-5
.elseif(n = 3297 || n = 3327 || n = 3417 || n = 3432)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 3298 || n = 3340)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 3307 || n = 3326 || n = 3345 || n = 3364 || n = 3383 || n = 3402)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 3313 || n = 3359 || n = 3428)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 3314)
	; 13 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 3317 || n = 3400 || n = 3434)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3324 || n = 3375 || n = 3392 || n = 3414 || n = 3429)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3325)
	; 14 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3329)
	; 9 bytes
	ldy #0
	delay_n 8
	dey
	Jne *-5
.elseif(n = 3341 || n = 3343 || n = 3360 || n = 3377 || n = 3416 || n = 3431)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3358 || n = 3362 || n = 3413 || n = 3433)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3382)
	; 10 bytes
	ldy #200 ;hides 'iny'
	delay_n 8
	dey
	dey
	Jne *-7
.elseif(n = 3399)
	; 9 bytes
	ldy #56 ;hides 'sec'
	delay_n 10
	iny
	Jne *-6
.elseif(n = 3415)
	; 14 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3419)
	; 13 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 3430)
	; 13 bytes
	ldy #126
	delay_n 10
	delay_n 12
	dey
	Jpl *-9
.endif
.endif
.else
; 3438 to 3750
.if(n = 3438 || n = 3441 || n = 3442 || n = 3444 || n = 3447 || n = 3449 || n = 3453 || n = 3454 || (n > 3455 && n < 3459) || n = 3461 || n = 3463 || n = 3465 || n = 3468 || n = 3470 || n = 3471 || (n > 3472 && n < 3476) || n = 3480 || n = 3483 || n = 3485 || n = 3488 || (n > 3489 && n < 3493) || n = 3498 || (n > 3499 && n < 3503) || n = 3505 || n = 3507 || n = 3510 || n = 3513 || n = 3515 || n = 3517 || n = 3518 || n = 3522 || n = 3524 || n = 3525 || n = 3527 || n = 3528 || (n > 3529 && n < 3534) || n = 3539 || n = 3542 || n = 3543 || (n > 3544 && n < 3549) || n = 3552 || n = 3555 || n = 3558 || (n > 3559 && n < 3564) || n = 3573 || (n > 3574 && n < 3579) || n = 3590 || n = 3593 || n = 3594 || n = 3596 || n = 3603 || n = 3606 || n = 3607 || n = 3609 || n = 3610 || n = 3618 || n = 3620 || n = 3621 || n = 3623 || n = 3624 || (n > 3625 && n < 3630) || n = 3632 || n = 3633 || (n > 3634 && n < 3639) || n = 3641 || (n > 3642 && n < 3646) || n = 3648 || n = 3650 || n = 3651 || n = 3653 || n = 3654 || n = 3657 || n = 3658 || n = 3660 || n = 3662 || n = 3663 || (n > 3664 && n < 3668) || n = 3670 || n = 3672 || n = 3675 || n = 3677 || n = 3678 || (n > 3679 && n < 3683) || n = 3689 || n = 3692 || n = 3693 || n = 3695 || n = 3696 || n = 3698 || n = 3699 || n = 3708 || n = 3709 || (n > 3710 && n < 3715) || n = 3720 || n = 3723 || n = 3726 || (n > 3727 && n < 3731) || n = 3738 || n = 3740 || n = 3742 || n = 3743 || n = 3745 || n = 3746 || n = 3748 || n = 3749)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3439 || n = 3443 || n = 3455 || n = 3462 || n = 3472 || n = 3482 || n = 3484 || n = 3489 || n = 3499 || n = 3506 || n = 3512 || n = 3514 || n = 3519 || n = 3523 || n = 3529 || n = 3538 || n = 3540 || n = 3544 || n = 3557 || n = 3559 || n = 3574 || n = 3589 || n = 3591 || n = 3595 || n = 3604 || n = 3608 || n = 3614 || n = 3619 || n = 3625 || n = 3634 || n = 3642 || n = 3652 || n = 3659 || n = 3664 || n = 3674 || n = 3679 || n = 3694 || n = 3710 || n = 3722 || n = 3727 || n = 3739 || n = 3747)
	; 11..12 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 3440 || n = 3459 || n = 3478 || n = 3497 || n = 3516 || n = 3535 || n = 3592 || n = 3611 || n = 3630 || n = 3649 || n = 3668 || n = 3687 || n = 3725 || n = 3744)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 3445 || n = 3487 || n = 3508 || n = 3550 || n = 3613 || n = 3655 || n = 3697 || n = 3718)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 3446 || n = 3521 || n = 3536 || n = 3564 || n = 3581 || n = 3598 || n = 3640 || n = 3683 || n = 3686 || n = 3716 || n = 3717 || n = 3731 || n = 3734)
	; 13..14 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3448 || n = 3464 || n = 3493 || n = 3549 || n = 3553 || n = 3568 || n = 3583 || n = 3600 || n = 3617 || n = 3685 || n = 3688 || n = 3702 || n = 3703 || n = 3733)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3450 || n = 3467 || n = 3569 || n = 3705)
	; 13 bytes
	delay_n 49
	delay_n (n-49)
.elseif((n .mod 15 = 1) && ((n > 3450 && n < 3737)))
	; 9 bytes
	ldy #(n-1)/15
	delay_n 10
	dey
	Jne *-5
.elseif(n = 3452 || n = 3469 || n = 3486 || n = 3503 || n = 3520 || n = 3537 || n = 3554 || n = 3588 || n = 3605 || n = 3622 || n = 3639 || n = 3656 || n = 3673 || n = 3690 || n = 3707 || n = 3724 || n = 3741)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 3460 || n = 3494 || n = 3504 || n = 3534 || n = 3579 || n = 3647 || n = 3669 || n = 3715)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3476 || n = 3551 || n = 3701)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 3477 || n = 3567 || n = 3582 || n = 3599 || n = 3732 || n = 3735)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 3479)
	; 9 bytes
	ldy #24 ;hides 'clc'
	delay_n 8
	iny
	Jne *-6
.elseif(n = 3495 || n = 3580)
	; 14 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3509)
	; 9 bytes
	ldy #234 ;hides 'nop'
	delay_n 8
	dey
	Jne *-6
.elseif(n = 3565 || n = 3700)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 3566 || n = 3612 || n = 3704 || n = 3750)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 3570 || n = 3572 || n = 3587 || n = 3602 || n = 3737)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3584)
	; 14 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3585)
	; 14 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3597)
	; 13 bytes
	ldy #123
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jpl *-9
.elseif(n = 3615)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3671)
	; 9 bytes
	ldy #216 ;hides 'cld'
	delay_n 10
	dey
	Jne *-6
.elseif(n = 3684)
	; 13 bytes
	ldy #126
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jpl *-9
.elseif(n = 3719)
	; 9 bytes
	ldy #248 ;hides 'sed'
	delay_n 8
	dey
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
; 3751 to 3906
.if((n .mod 15 = 1) && ((n > 3750 && n < 3827)))
	; 9 bytes
	ldy #(n-1)/15
	delay_n 10
	dey
	Jne *-5
.elseif(n = 3752 || n = 3753 || (n > 3754 && n < 3758) || n = 3760 || n = 3762 || n = 3764 || n = 3765 || n = 3767 || n = 3768 || (n > 3769 && n < 3773) || n = 3777 || n = 3779 || n = 3783 || (n > 3784 && n < 3789) || n = 3794 || n = 3797 || n = 3798 || n = 3800 || n = 3803 || n = 3805 || n = 3813 || (n > 3814 && n < 3819) || n = 3822 || n = 3824 || n = 3825 || n = 3827 || n = 3828 || (n > 3829 && n < 3834) || n = 3845 || (n > 3846 && n < 3851) || n = 3862 || n = 3864 || n = 3866 || n = 3867 || n = 3869 || n = 3878 || n = 3879 || (n > 3880 && n < 3885) || n = 3899 || n = 3900 || n = 3903)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3754 || n = 3761 || n = 3769 || n = 3778 || n = 3784 || n = 3795 || n = 3804 || n = 3812 || n = 3814 || n = 3829 || n = 3842 || n = 3844 || n = 3846 || n = 3861 || n = 3863 || n = 3868 || n = 3880 || n = 3897)
	; 11..13 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 3758 || n = 3775 || n = 3792 || n = 3809 || n = 3843 || n = 3860 || n = 3877 || n = 3894)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 3759 || n = 3774 || n = 3789 || n = 3834 || n = 3885 || n = 3902)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3763 || n = 3782 || n = 3801 || n = 3820 || n = 3839 || n = 3858 || n = 3896)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 3773 || n = 3819 || n = 3888)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 3776 || n = 3876 || n = 3901)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 3780)
	; 10 bytes
	ldy #200 ;hides 'iny'
	delay_n 10
	dey
	dey
	Jne *-7
.elseif(n = 3790 || n = 3875 || n = 3890 || n = 3892)
	; 13 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 3791 || n = 3806 || n = 3821 || n = 3836 || n = 3851 || n = 3870 || n = 3887 || n = 3904)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3793 || n = 3838 || n = 3853 || n = 3855 || n = 3872 || n = 3906)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3799)
	; 9 bytes
	ldy #56 ;hides 'sec'
	delay_n 12
	iny
	Jne *-6
.elseif(n = 3802 || n = 3823 || n = 3865 || n = 3886)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 3807 || n = 3810 || n = 3837 || n = 3852 || n = 3854 || n = 3871 || n = 3905)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 3808 || n = 3835 || n = 3889)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 3840 || n = 3874 || n = 3891)
	; 14 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3841)
	; 9 bytes
	ldy #0
	delay_n 10
	dey
	Jne *-5
.elseif(n = 3856 || n = 3873)
	; 14 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3857 || n = 3859 || n = 3893)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3895)
	; 14 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 3898)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
.if((n >= 3907) && (n < 3985))
; 3907 to 3984
.if(n = 3907 || n = 3970)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 3908 || n = 3925)
	; 14 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3909 || n = 3913 || n = 3916 || n = 3917 || n = 3919 || n = 3920 || n = 3930 || n = 3932 || n = 3933 || n = 3935 || n = 3936 || n = 3938 || n = 3939 || n = 3947 || (n > 3948 && n < 3953) || n = 3955 || (n > 3956 && n < 3960) || n = 3964 || (n > 3965 && n < 3970) || n = 3974 || n = 3976 || n = 3978 || n = 3981 || n = 3983 || n = 3984)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3910 || n = 3914 || n = 3918 || n = 3931 || n = 3937 || n = 3946 || n = 3948 || n = 3956 || n = 3965 || n = 3973 || n = 3975 || n = 3980 || n = 3982)
	; 11..13 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 3911 || n = 3928 || n = 3945 || n = 3962 || n = 3979)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 3912)
	; 14 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 3915 || n = 3934 || n = 3953 || n = 3972)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 3921 || n = 3963)
	; 13..14 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3922 || n = 3954)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 3923 || n = 3940)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3924 || n = 3941)
	; 14 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 3926)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 3927 || n = 3929)
	; 14 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 3942)
	; 14 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 3943)
	; 9 bytes
	ldy #24 ;hides 'clc'
	delay_n 10
	iny
	Jne *-6
.elseif(n = 3944 || n = 3961)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3960)
	; 13 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 3971)
	; 14 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3977)
	; 9 bytes
	ldy #234 ;hides 'nop'
	delay_n 10
	dey
	Jne *-6
.endif
.else
; 3985 to 4062
.if(n = 3985 || n = 3986 || n = 3993 || n = 3995 || n = 3998 || (n > 3999 && n < 4003) || n = 4005 || n = 4015 || (n > 4016 && n < 4021) || n = 4031 || n = 4032 || (n > 4033 && n < 4038) || n = 4049 || (n > 4050 && n < 4054) || n = 4056 || n = 4058)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3987 || n = 3989 || n = 4006 || n = 4023 || n = 4040 || n = 4057)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3988 || n = 3990 || n = 4007 || n = 4041)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 3991 || n = 4010 || n = 4029 || n = 4048)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 3992 || n = 4011 || n = 4028 || n = 4045 || n = 4062)
	; 13 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 3994 || n = 3999 || n = 4016 || n = 4033 || n = 4050)
	; 11..12 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 3996 || n = 4013 || n = 4030 || n = 4047)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 3997 || n = 4024)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 4003 || n = 4026)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 4004 || n = 4021 || n = 4038 || n = 4055)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4008 || n = 4025 || n = 4042 || n = 4059)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4009 || n = 4043 || n = 4060)
	; 14 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4012 || n = 4054)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4014)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4022)
	; 14 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 4027)
	; 14 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 4039)
	; 14 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4044)
	; 14 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4046)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4061)
	; 13 bytes
	ldy #140
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.endif
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
; 4063 to 4218
.if(n = 4063 || n = 4080 || n = 4131 || n = 4216)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4064 || n = 4081 || n = 4098 || n = 4115 || n = 4132 || n = 4149 || n = 4166 || n = 4183 || n = 4200 || n = 4217)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 4065 || n = 4082 || n = 4116 || n = 4133 || n = 4150)
	; 14 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 4066 || n = 4068 || n = 4069 || n = 4071 || n = 4073 || n = 4077 || n = 4083 || n = 4085 || n = 4087 || n = 4088 || n = 4090 || n = 4092 || n = 4097 || n = 4100 || n = 4102 || n = 4104 || n = 4107 || n = 4109 || n = 4111 || n = 4117 || (n > 4118 && n < 4123) || n = 4126 || n = 4128 || n = 4129 || n = 4134 || n = 4136 || n = 4137 || n = 4139 || n = 4140 || n = 4145 || n = 4147 || n = 4151 || (n > 4152 && n < 4157) || n = 4161 || n = 4163 || n = 4164 || n = 4167 || n = 4168 || (n > 4169 && n < 4174) || n = 4182 || n = 4185 || (n > 4186 && n < 4191) || n = 4202 || (n > 4203 && n < 4208))
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4067 || n = 4086 || n = 4105 || n = 4124 || n = 4143 || n = 4162 || n = 4181)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 4070 || n = 4084 || n = 4089 || n = 4099 || n = 4101 || n = 4106 || n = 4108 || n = 4118 || n = 4127 || n = 4135 || n = 4146 || n = 4152 || n = 4165 || n = 4169 || n = 4184 || n = 4186 || n = 4203 || n = 4218)
	; 11..13 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 4072 || n = 4095 || n = 4141 || n = 4210)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 4074 || n = 4091 || n = 4125 || n = 4142 || n = 4193)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4075 || n = 4096 || n = 4138 || n = 4159 || n = 4180 || n = 4201)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4076 || n = 4176)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4078 || n = 4213)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 4079 || n = 4113 || n = 4130 || n = 4198)
	; 13 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 4093 || n = 4110 || n = 4144 || n = 4195 || n = 4212)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4094 || n = 4179 || n = 4196)
	; 14 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4103)
	; 9 bytes
	ldy #216 ;hides 'cld'
	delay_n 12
	dey
	Jne *-6
.elseif(n = 4112 || n = 4197 || n = 4214)
	; 14 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4114 || n = 4160 || n = 4194 || n = 4211)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 4123 || n = 4157 || n = 4174 || n = 4191 || n = 4208)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4148)
	; 13 bytes
	ldy #143
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4158 || n = 4175 || n = 4192 || n = 4209)
	; 14 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4177)
	; 13 bytes
	ldy #144
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4178)
	; 10 bytes
	ldy #200 ;hides 'iny'
	delay_n 12
	dey
	dey
	Jne *-7
.elseif(n = 4199)
	; 10 bytes
	ldy #56 ;hides 'sec'
	nop
	delay_n 12
	iny
	Jne *-7
.elseif(n = 4215)
	; 9 bytes
	ldy #248 ;hides 'sed'
	delay_n 10
	dey
	Jne *-6
.endif
.else
; 4219 to 4375
.if(n = 4219 || n = 4238 || n = 4257 || n = 4276 || n = 4295 || n = 4314 || n = 4333 || n = 4352 || n = 4371)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 4220 || n = 4225 || n = 4237 || n = 4241 || n = 4254 || n = 4260 || n = 4271 || n = 4279 || n = 4288 || n = 4298 || n = 4305 || n = 4317 || n = 4322 || n = 4330 || n = 4339 || n = 4356 || n = 4372 || n = 4374)
	; 11..13 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 4221)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4222 || n = 4243 || n = 4264 || n = 4327 || n = 4348 || n = 4369)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4223 || n = 4224 || n = 4236 || n = 4239 || n = 4240 || n = 4242 || n = 4245 || n = 4247 || n = 4253 || n = 4255 || n = 4256 || n = 4258 || n = 4259 || n = 4261 || n = 4262 || n = 4266 || n = 4269 || n = 4270 || (n > 4271 && n < 4276) || n = 4278 || (n > 4279 && n < 4283) || n = 4287 || (n > 4288 && n < 4293) || n = 4297 || n = 4299 || n = 4300 || n = 4303 || n = 4304 || (n > 4305 && n < 4310) || n = 4316 || n = 4318 || n = 4320 || n = 4321 || (n > 4322 && n < 4326) || n = 4328 || n = 4329 || n = 4335 || n = 4337 || n = 4338 || (n > 4339 && n < 4344) || n = 4350 || n = 4354 || n = 4355 || (n > 4356 && n < 4361) || n = 4373 || n = 4375)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4226 || n = 4301 || n = 4326 || n = 4351)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4227 || n = 4244 || n = 4286 || n = 4312 || n = 4346 || n = 4363)
	; 13..14 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4228 || n = 4296 || n = 4313 || n = 4347 || n = 4364)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 4229 || n = 4246 || n = 4263 || n = 4331 || n = 4365)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4230 || n = 4315 || n = 4332 || n = 4349 || n = 4366)
	; 14 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4231 || n = 4248 || n = 4265 || n = 4367)
	; 14 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4232 || n = 4249 || n = 4283 || n = 4334 || n = 4368)
	; 13 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 4233)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 4234 || n = 4251 || n = 4268 || n = 4285 || n = 4302 || n = 4319 || n = 4336)
	; 9 bytes
	ldy #(n-1)/17
	delay_n 12
	dey
	Jne *-5
.elseif(n = 4235)
	; 13 bytes
	ldy #146
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4250 || n = 4284)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4252)
	; 14 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 4267 || n = 4294)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 4277 || n = 4311 || n = 4345 || n = 4362)
	; 14 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4293)
	; 13 bytes
	ldy #148
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4310 || n = 4344 || n = 4361)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4353)
	; 9 bytes
	ldy #0
	delay_n 12
	dey
	Jne *-5
.elseif(n = 4370)
	; 14 bytes
	delay_n 51
	delay_n (n-51)
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
.if((n >= 4376) && (n < 4454))
.if((n >= 4376) && (n < 4415))
; 4376 to 4414
.if(n = 4376 || n = 4401)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4377 || n = 4378 || n = 4392 || (n > 4393 && n < 4397) || n = 4403 || n = 4414)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4379)
	; 14 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4380)
	; 13 bytes
	ldy #151
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4381 || n = 4400)
	; 14 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4382 || n = 4399)
	; 14 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4383)
	; 14 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4384)
	; 14 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4385)
	; 13 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 4386)
	; 14 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 4387 || n = 4404)
	; 14 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 4388 || n = 4405)
	; 14 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 4389)
	; 14 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4390 || n = 4409)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 4391 || n = 4408)
	; 14 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4393 || n = 4410 || n = 4412)
	; 11..12 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 4397)
	; 14 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 4398)
	; 14 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4402)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 4406)
	; 14 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 4407)
	; 9 bytes
	ldy #24 ;hides 'clc'
	delay_n 12
	iny
	Jne *-6
.elseif(n = 4411)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4413)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
; 4415 to 4453
.if(n = 4415 || n = 4430 || n = 4433 || n = 4434 || n = 4436 || n = 4442 || n = 4449 || n = 4451 || n = 4452)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4416)
	; 14 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 4417 || n = 4440)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 4418)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 4419)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4420 || n = 4437)
	; 14 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 4421)
	; 14 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 4422 || n = 4439)
	; 14 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 4423)
	; 14 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 4424)
	; 14 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 4425)
	; 14 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4426)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4427 || n = 4444)
	; 14 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4428 || n = 4447)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 4429)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 4431 || n = 4435 || n = 4448 || n = 4450)
	; 11..13 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 4432 || n = 4453)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4438)
	; 13 bytes
	ldy #153
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4441)
	; 14 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 4443)
	; 14 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4445)
	; 9 bytes
	ldy #234 ;hides 'nop'
	delay_n 12
	dey
	Jne *-6
.elseif(n = 4446)
	; 14 bytes
	delay_n 161
	delay_n (n-161)
.endif
.endif
.else
.if((n >= 4454) && (n < 4493))
; 4454 to 4492
.if(n = 4454)
	; 14 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 4455 || n = 4458 || n = 4465 || n = 4468 || (n > 4469 && n < 4473) || n = 4476 || n = 4478 || n = 4487 || (n > 4488 && n < 4492))
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4456 || n = 4483)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 4457)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4459)
	; 14 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4460)
	; 14 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4461)
	; 14 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 4462 || n = 4479)
	; 14 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 4463 || n = 4486)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 4464)
	; 14 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 4466 || n = 4485)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 4467)
	; 13 bytes
	ldy #154
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4469 || n = 4477 || n = 4488)
	; 12..13 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 4473)
	; 14 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 4474)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4475 || n = 4492)
	; 14 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 4480)
	; 14 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 4481)
	; 14 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 4482)
	; 14 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 4484)
	; 14 bytes
	delay_n 131
	delay_n (n-131)
.endif
.else
; 4493 to 4531
.if(n = 4493)
	; 14 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 4494)
	; 13 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 4495 || n = 4516)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4496)
	; 13 bytes
	ldy #155
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4497 || n = 4499 || n = 4503 || n = 4506 || n = 4508 || n = 4511 || n = 4512 || n = 4518 || n = 4520 || n = 4521 || n = 4525 || (n > 4526 && n < 4531))
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4498 || n = 4507 || n = 4519 || n = 4526)
	; 12..13 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 4500 || n = 4517)
	; 14 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 4501)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4502)
	; 14 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 4504 || n = 4523)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 4505 || n = 4522)
	; 14 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 4509)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 4510)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 4513)
	; 14 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4514)
	; 14 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 4515)
	; 14 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 4524)
	; 14 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 4531)
	; 14 bytes
	delay_n 86
	delay_n (n-86)
.endif
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
.if((n >= 4532) && (n < 4571))
; 4532 to 4570
.if(n = 4532 || n = 4555)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 4533 || n = 4552)
	; 14 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4534 || n = 4539 || n = 4541 || n = 4544 || (n > 4545 && n < 4549) || n = 4553 || n = 4556 || n = 4557 || n = 4559 || n = 4560 || n = 4562 || n = 4563 || (n > 4564 && n < 4569))
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4535)
	; 10 bytes
	ldy #216 ;hides 'cld'
	nop
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4536)
	; 14 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4537 || n = 4558)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4538 || n = 4540 || n = 4545 || n = 4564)
	; 12..13 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 4542 || n = 4561)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 4543)
	; 14 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 4549)
	; 14 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 4550 || n = 4569)
	; 14 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4551)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4554)
	; 13 bytes
	ldy #157
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4570)
	; 15 bytes
	delay_n 66
	delay_n (n-66)
.endif
.else
; 4571 to 4609
.if(n = 4571 || n = 4590 || n = 4609)
	; 14 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4572)
	; 14 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 4573 || n = 4592)
	; 14 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4574)
	; 14 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 4575)
	; 15 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 4576)
	; 11 bytes
	ldy #200 ;hides 'iny'
	nop
	delay_n 12
	dey
	dey
	Jne *-8
.elseif(n = 4577)
	; 15 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4578)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 4579 || n = 4600)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4580 || n = 4599)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 4581 || n = 4582 || (n > 4583 && n < 4588) || n = 4593 || n = 4595 || n = 4601 || (n > 4602 && n < 4607))
	; 11..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 4583 || n = 4594 || n = 4602)
	; 12..15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 4588)
	; 14 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 4589)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 4591)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 4596)
	; 14 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4597)
	; 15 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4598)
	; 14 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 4607)
	; 14 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4608)
	; 15 bytes
	delay_n 66
	delay_n (n-66)
.endif
.endif
.else
; 4610 to 4687
.if(n = 4610 || n = 4629 || n = 4648)
	; 14 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 4611 || n = 4630 || n = 4668)
	; 14 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4612)
	; 13 bytes
	ldy #159
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4613 || n = 4632)
	; 15 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 4614 || n = 4620 || n = 4622 || n = 4623 || n = 4625 || n = 4628 || n = 4639 || n = 4641 || n = 4643 || n = 4644 || n = 4646 || n = 4649 || n = 4653 || n = 4655 || n = 4658 || (n > 4659 && n < 4663) || n = 4665 || n = 4667 || n = 4669 || n = 4674 || n = 4677 || (n > 4678 && n < 4682) || n = 4686)
	; 11..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 4615 || n = 4627 || n = 4640 || n = 4650 || n = 4654 || n = 4659 || n = 4666 || n = 4673 || n = 4678 || n = 4687)
	; 12..15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 4616)
	; 14 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 4617)
	; 15 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 4618 || n = 4637 || n = 4656 || n = 4675)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 4619)
	; 14 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 4621 || n = 4642 || n = 4663 || n = 4684)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4624 || n = 4647 || n = 4670)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 4626 || n = 4651 || n = 4676)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4631)
	; 14 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 4633 || n = 4671)
	; 15 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4634)
	; 15 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4635)
	; 15 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4636)
	; 14 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 4638 || n = 4657)
	; 15 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4645 || n = 4672)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 4652 || n = 4685)
	; 14..15 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4664 || n = 4683)
	; 14 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4682)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
; 4688 to 4765
.if((n > 4687 && n < 4691) || n = 4695 || n = 4696 || n = 4698 || n = 4700 || n = 4703 || n = 4707 || n = 4709 || n = 4710 || n = 4715 || (n > 4716 && n < 4721) || n = 4728 || n = 4730 || n = 4731 || n = 4734 || (n > 4735 && n < 4739) || n = 4741 || n = 4749 || n = 4752 || n = 4753 || n = 4755 || n = 4756 || n = 4758 || n = 4759 || n = 4764)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4691 || n = 4727 || n = 4748)
	; 14..15 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4692)
	; 15 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4693 || n = 4739 || n = 4762)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 4694 || n = 4713 || n = 4732 || n = 4751)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 4697 || n = 4708 || n = 4714 || n = 4716 || n = 4729 || n = 4735 || n = 4750 || n = 4754 || n = 4765)
	; 11..14 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 4699)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 4701)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4702 || n = 4740)
	; 14 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4704 || n = 4721 || n = 4742 || n = 4761)
	; 13..14 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4705 || n = 4726 || n = 4747)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4706 || n = 4723 || n = 4725 || n = 4744 || n = 4763)
	; 13..14 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4711)
	; 9 bytes
	ldy #248 ;hides 'sed'
	delay_n 12
	dey
	Jne *-6
.elseif(n = 4712)
	; 15 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 4722)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 4724 || n = 4743 || n = 4760)
	; 13..14 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 4733)
	; 15 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4745)
	; 14 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 4746)
	; 15 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 4757)
	; 13 bytes
	ldy #164
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.endif
.else
; 4766 to 4843
.if(n = 4766 || n = 4772 || n = 4774 || n = 4775 || n = 4777 || n = 4778 || n = 4782 || n = 4784 || n = 4787 || n = 4788 || n = 4791 || (n > 4792 && n < 4797) || n = 4803 || n = 4805 || n = 4809 || n = 4813 || n = 4814 || n = 4817 || n = 4828 || n = 4829 || n = 4832 || n = 4833 || n = 4835 || n = 4836)
	; 11..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 4767 || n = 4769)
	; 14..15 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 4768 || n = 4810 || n = 4831)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4770 || n = 4789 || n = 4808 || n = 4827)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 4771 || n = 4773 || n = 4779 || n = 4783 || n = 4792 || n = 4804 || n = 4811 || n = 4830)
	; 12..15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 4776 || n = 4801 || n = 4826)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4780 || n = 4807 || n = 4834)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 4781 || n = 4800 || n = 4819 || n = 4838)
	; 14 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 4785)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 4786)
	; 13 bytes
	ldy #165
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4790)
	; 15 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4797 || n = 4816)
	; 14 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4798)
	; 15 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 4799 || n = 4818 || n = 4837 || n = 4841)
	; 14..15 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4802 || n = 4823)
	; 14..15 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4806)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 4812)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4815)
	; 13 bytes
	ldy #166
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4820 || n = 4839)
	; 14 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4821 || n = 4840)
	; 15 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 4822)
	; 14 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 4824 || n = 4843)
	; 15 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4825)
	; 15 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4842)
	; 14 bytes
	delay_n 131
	delay_n (n-131)
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
; 4844 to 4882
.if(n = 4844)
	; 13 bytes
	ldy #167
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4845)
	; 15 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 4846)
	; 10 bytes
	ldy #(n-1)/19
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 4847)
	; 14 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 4848 || n = 4850 || n = 4853 || n = 4857 || n = 4863 || n = 4867 || n = 4869 || n = 4870 || n = 4872 || n = 4875 || n = 4878 || n = 4879)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4849 || n = 4855 || n = 4868 || n = 4874 || n = 4880)
	; 12..14 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 4851 || n = 4876)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4852 || n = 4873)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4854 || n = 4877)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 4856)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4858)
	; 14 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4859)
	; 15 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 4860)
	; 15 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 4861)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 4862)
	; 14 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4864)
	; 14 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 4865)
	; 10 bytes
	ldy #0
	nop
	delay_n 12
	dey
	Jne *-6
.elseif(n = 4866)
	; 15 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4871)
	; 10 bytes
	ldy #24 ;hides 'clc'
	nop
	delay_n 12
	iny
	Jne *-7
.elseif(n = 4881)
	; 14 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4882)
	; 14 bytes
	delay_n 11
	delay_n (n-11)
.endif
.else
; 4883 to 4921
.if(n = 4883)
	; 14 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4884)
	; 15 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 4885)
	; 14 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 4886)
	; 15 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 4887)
	; 14 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 4888)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 4889 || n = 4908)
	; 15 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4890 || n = 4896 || n = 4898 || n = 4903 || n = 4904 || n = 4917 || (n > 4918 && n < 4922))
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4891 || n = 4897 || n = 4905 || n = 4916 || n = 4918)
	; 12..15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 4892)
	; 14 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 4893 || n = 4912)
	; 15 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 4894 || n = 4915)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4895 || n = 4914)
	; 14 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 4899)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 4900)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 4901)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4902)
	; 13 bytes
	ldy #169
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4906)
	; 15 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 4907)
	; 14 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 4909)
	; 15 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 4910)
	; 15 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 4911)
	; 15 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4913)
	; 10 bytes
	ldy #234 ;hides 'nop'
	nop
	delay_n 12
	dey
	Jne *-7
.endif
.endif
.else
.if((n >= 4922) && (n < 4961))
; 4922 to 4960
.if(n = 4922)
	; 15 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 4923 || n = 4946)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 4924)
	; 14 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 4925 || n = 4927 || n = 4928 || n = 4933 || n = 4938 || n = 4940 || n = 4941 || n = 4944 || n = 4948 || n = 4950 || n = 4953 || n = 4955 || n = 4959)
	; 12..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 4926 || n = 4951)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4929 || n = 4934 || n = 4939 || n = 4945 || n = 4949 || n = 4954)
	; 13..15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 4930)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 4931)
	; 13 bytes
	ldy #170
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4932)
	; 15 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 4935)
	; 15 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 4936 || n = 4957)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4937 || n = 4956)
	; 15 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4942)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 4943)
	; 15 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 4947)
	; 15 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 4952)
	; 15 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4958)
	; 15 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 4960)
	; 13 bytes
	ldy #171
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.endif
.else
; 4961 to 5000
.if((n > 4960 && n < 4964) || n = 4971 || n = 4973 || n = 4980 || (n > 4981 && n < 4985) || n = 4991 || n = 4994 || n = 4998 || n = 5000)
	; 12..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 4964)
	; 15 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 4965)
	; 15 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 4966)
	; 15 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 4967)
	; 11 bytes
	ldy #216 ;hides 'cld'
	delay_n 4
	delay_n 12
	dey
	Jne *-8
.elseif(n = 4968 || n = 4987)
	; 15 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4969 || n = 4992)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 4970 || n = 4972 || n = 4977 || n = 4979 || n = 4981 || n = 4995)
	; 13..15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 4974)
	; 12 bytes
	ldy #200 ;hides 'iny'
	delay_n 4
	delay_n 12
	dey
	dey
	Jne *-9
.elseif(n = 4975)
	; 15 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 4976)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4978 || n = 4999)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 4985)
	; 15 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 4986)
	; 15 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 4988)
	; 15 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4989)
	; 13 bytes
	ldy #172
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 4990)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4993)
	; 15 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 4996)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 4997)
	; 15 bytes
	delay_n 151
	delay_n (n-151)
.endif
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
.if((n >= 5001) && (n < 5040))
; 5001 to 5039
.if(n = 5001 || (n > 5002 && n < 5006) || n = 5017 || n = 5019 || n = 5022 || n = 5024 || n = 5025 || n = 5027 || n = 5028 || n = 5030 || n = 5033)
	; 12..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5002 || n = 5021 || n = 5029)
	; 13..15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 5006)
	; 15 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5007)
	; 15 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 5008)
	; 15 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 5009)
	; 15 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5010)
	; 15 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5011)
	; 15 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 5012 || n = 5031)
	; 15 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 5013 || n = 5032)
	; 15 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 5014)
	; 15 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 5015 || n = 5038)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 5016)
	; 15 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5018)
	; 13 bytes
	ldy #173
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5020)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 5023)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5026)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5034)
	; 15 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 5035)
	; 16 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5036)
	; 15 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 5037)
	; 15 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 5039)
	; 15 bytes
	delay_n 174
	delay_n (n-174)
.endif
.else
; 5040 to 5078
.if(n = 5040 || n = 5042 || n = 5043 || n = 5045 || n = 5046 || n = 5048 || n = 5049 || n = 5052 || n = 5053 || n = 5055 || n = 5058 || n = 5063 || n = 5064 || (n > 5065 && n < 5070) || n = 5078)
	; 12..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5041 || n = 5062)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 5044 || n = 5065)
	; 13 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 5047)
	; 13 bytes
	ldy #174
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5050 || n = 5077)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5051 || n = 5076)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5054)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 5056)
	; 15 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 5057)
	; 15 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 5059)
	; 15 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 5060)
	; 16 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 5061)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 5070)
	; 15 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5071)
	; 16 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 5072)
	; 15 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5073)
	; 15 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5074)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5075)
	; 16 bytes
	delay_n 76
	delay_n (n-76)
.endif
.endif
.else
; 5079 to 5156
.if(n = 5079 || n = 5081 || n = 5082 || n = 5085 || (n > 5086 && n < 5091) || n = 5103 || n = 5106 || n = 5108 || n = 5109 || (n > 5110 && n < 5114) || n = 5118 || n = 5127 || n = 5129 || n = 5132 || n = 5133 || n = 5135 || n = 5136 || n = 5138 || n = 5139 || n = 5148 || n = 5150 || n = 5152 || n = 5155)
	; 12..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5080 || n = 5086 || n = 5110 || n = 5119 || n = 5128 || n = 5137 || n = 5149 || n = 5154 || n = 5156)
	; 13..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 5083 || n = 5104 || n = 5125 || n = 5146)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 5084 || n = 5107 || n = 5130 || n = 5153)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 5091)
	; 15 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5092)
	; 16 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 5093 || n = 5114 || n = 5117)
	; 15..16 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5094)
	; 15 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 5095)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5096)
	; 16 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 5097)
	; 16 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 5098 || n = 5140)
	; 16 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5099)
	; 15 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 5100 || n = 5121 || n = 5142)
	; 16 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 5101 || n = 5126 || n = 5151)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5102 || n = 5123 || n = 5144)
	; 16 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 5105)
	; 13 bytes
	ldy #176
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5115)
	; 15 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5116 || n = 5147)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 5120 || n = 5141)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5122 || n = 5143)
	; 16 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5124 || n = 5145)
	; 16 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 5131)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5134)
	; 13 bytes
	ldy #177
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
; 5157 to 5234
.if(n = 5157 || n = 5159 || n = 5160 || n = 5162 || n = 5164 || n = 5165 || n = 5168 || n = 5169 || (n > 5170 && n < 5175) || n = 5178 || n = 5180 || n = 5181 || n = 5187 || n = 5189 || n = 5190 || n = 5193 || n = 5194 || n = 5196 || n = 5197 || n = 5203 || n = 5205 || n = 5206 || n = 5211 || (n > 5212 && n < 5217) || n = 5223 || n = 5224 || n = 5227 || n = 5228 || n = 5231 || n = 5232 || n = 5234)
	; 12..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5158 || n = 5185 || n = 5212)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5161 || n = 5166 || n = 5170 || n = 5179 || n = 5191 || n = 5195 || n = 5202 || n = 5204 || n = 5210 || n = 5225 || n = 5229 || n = 5233)
	; 12..15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 5163)
	; 13 bytes
	ldy #178
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5167 || n = 5188 || n = 5209 || n = 5230)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 5175)
	; 15 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5176 || n = 5199 || n = 5222)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 5177 || n = 5186 || n = 5198 || n = 5217)
	; 14..16 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5182)
	; 15 bytes
	ldy #157
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 5183)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5184)
	; 16 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 5192)
	; 13 bytes
	ldy #179
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5200 || n = 5219)
	; 14..15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5201 || n = 5226)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5207)
	; 10 bytes
	ldy #248 ;hides 'sed'
	nop
	delay_n 12
	dey
	Jne *-7
.elseif(n = 5208)
	; 16 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 5218 || n = 5220)
	; 14..15 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5221)
	; 13 bytes
	ldy #180
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.endif
.else
; 5235 to 5312
.if((n > 5234 && n < 5238) || n = 5241 || n = 5243 || n = 5244 || n = 5247 || n = 5249 || n = 5252 || n = 5253 || (n > 5254 && n < 5259) || n = 5270 || n = 5273 || n = 5274 || n = 5277 || n = 5278 || n = 5280 || n = 5281 || n = 5283 || n = 5295 || (n > 5296 && n < 5301) || n = 5303 || (n > 5304 && n < 5308) || n = 5310 || n = 5312)
	; 12..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5238 || n = 5259)
	; 15 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5239 || n = 5266)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5240 || n = 5271 || n = 5302)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 5242 || n = 5248 || n = 5254 || n = 5269 || n = 5275 || n = 5282 || n = 5294 || n = 5296 || n = 5304 || n = 5311)
	; 13..15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 5245 || n = 5268 || n = 5291)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 5246 || n = 5267 || n = 5309)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5250)
	; 13 bytes
	ldy #181
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5251 || n = 5272 || n = 5293)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 5260)
	; 16 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 5261)
	; 15 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5262)
	; 15 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5263 || n = 5284)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5264 || n = 5285)
	; 16 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 5265 || n = 5286)
	; 16 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 5276 || n = 5301)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5279)
	; 13 bytes
	ldy #182
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5287)
	; 16 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5288 || n = 5290)
	; 15..16 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5289)
	; 16 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 5292)
	; 16 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 5308)
	; 13 bytes
	ldy #183
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.endif
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
; 5313 to 5390
.if(n = 5313 || n = 5315)
	; 15..16 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5314 || n = 5335 || n = 5356)
	; 11 bytes
	ldy #(n-1)/21
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 5316 || n = 5318 || n = 5319 || n = 5321 || n = 5322 || n = 5324 || n = 5328 || (n > 5329 && n < 5333) || (n > 5339 && n < 5343) || n = 5349 || n = 5352 || n = 5353 || n = 5355 || n = 5357 || n = 5358 || n = 5361 || n = 5362 || n = 5365 || n = 5368 || n = 5370 || n = 5371 || n = 5378 || n = 5379 || n = 5382 || n = 5385 || n = 5387 || n = 5388)
	; 12..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5317 || n = 5323 || n = 5329 || n = 5336 || n = 5338 || n = 5350 || n = 5354 || n = 5359 || n = 5363 || n = 5369 || n = 5375 || n = 5380 || n = 5384 || n = 5386)
	; 13..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 5320 || n = 5347 || n = 5374)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5325 || n = 5346 || n = 5367)
	; 15 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5326 || n = 5351 || n = 5376)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5327 || n = 5390)
	; 16 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 5333 || n = 5364)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 5334)
	; 16 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 5337 || n = 5360 || n = 5383)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 5339)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 5343)
	; 15 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5344)
	; 16 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 5345)
	; 15 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5348)
	; 15 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5366)
	; 13 bytes
	ldy #185
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5372)
	; 13 bytes
	ldy #200 ;hides 'iny'
	delay_n 6
	delay_n 12
	dey
	dey
	Jne *-10
.elseif(n = 5373)
	; 15 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 5377)
	; 11 bytes
	ldy #0
	delay_n 4
	delay_n 12
	dey
	Jne *-7
.elseif(n = 5381)
	; 11 bytes
	ldy #234 ;hides 'nop'
	delay_n 4
	delay_n 12
	dey
	Jne *-8
.elseif(n = 5389)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
.if((n >= 5391) && (n < 5430))
; 5391 to 5429
.if(n = 5391 || n = 5416)
	; 15..16 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5392)
	; 15 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5393)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5394 || n = 5415)
	; 16 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 5395)
	; 13 bytes
	ldy #186
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5396 || n = 5417)
	; 16 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 5397 || n = 5400 || n = 5403 || n = 5405 || n = 5407 || n = 5408 || (n > 5409 && n < 5413))
	; 13..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5398 || n = 5402 || n = 5404 || n = 5409 || n = 5427)
	; 14..15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 5399)
	; 12 bytes
	ldy #216 ;hides 'cld'
	delay_n 6
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5401 || n = 5426)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5406 || n = 5429)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 5413)
	; 15 bytes
	ldy #164
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 5414)
	; 16 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 5418)
	; 16 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 5419)
	; 16 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 5420)
	; 16 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5421)
	; 16 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 5422)
	; 16 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 5423)
	; 16 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 5424)
	; 13 bytes
	ldy #187
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5425)
	; 16 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 5428)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.endif
.else
; 5430 to 5468
.if(n = 5430 || n = 5431 || (n > 5432 && n < 5436) || n = 5454 || n = 5456 || n = 5457 || (n > 5458 && n < 5462))
	; 13..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5432 || n = 5458)
	; 14..15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 5436)
	; 16 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 5437)
	; 16 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5438)
	; 16 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 5439 || n = 5462)
	; 16 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5440)
	; 16 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 5441)
	; 16 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5442 || n = 5463)
	; 16 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 5443)
	; 16 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 5444 || n = 5465)
	; 16 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5445 || n = 5466)
	; 16 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 5446)
	; 15 bytes
	ldy #165
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 5447 || n = 5468)
	; 16 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5448)
	; 16 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 5449)
	; 16 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 5450)
	; 16 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 5451)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5452)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 5453)
	; 13 bytes
	ldy #188
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5455)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5464)
	; 16 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 5467)
	; 16 bytes
	delay_n 111
	delay_n (n-111)
.endif
.endif
.endif
.else
.if((n >= 5469) && (n < 5547))
; 5469 to 5546
.if(n = 5469)
	; 16 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 5470 || n = 5533)
	; 16 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 5471 || n = 5492)
	; 16 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 5472 || n = 5493)
	; 16 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 5473)
	; 16 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5474 || n = 5495 || n = 5537)
	; 16 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 5475 || n = 5498 || n = 5521 || n = 5544)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 5476 || n = 5501 || n = 5526)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif((n > 5476 && n < 5482) || n = 5484 || n = 5486 || n = 5487 || n = 5490 || n = 5499 || n = 5500 || n = 5502 || n = 5503 || (n > 5504 && n < 5508) || n = 5513 || n = 5515 || n = 5516 || n = 5520 || n = 5523 || n = 5525 || n = 5527 || n = 5528 || (n > 5529 && n < 5533) || n = 5535 || n = 5538 || n = 5541 || n = 5542 || n = 5546)
	; 13..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 5482 || n = 5509 || n = 5536)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5483)
	; 16 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5485 || n = 5491 || n = 5504 || n = 5512 || n = 5514 || n = 5522 || n = 5524 || n = 5529 || n = 5539 || n = 5543)
	; 14..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 5488 || n = 5519)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 5489 || n = 5510)
	; 16 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 5494)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5496 || n = 5517)
	; 16 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 5497 || n = 5518)
	; 16 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5508)
	; 16 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 5511)
	; 13 bytes
	ldy #190
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5534)
	; 16 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 5540)
	; 13 bytes
	ldy #191
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5545)
	; 15 bytes
	ldy #168
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.endif
.else
; 5547 to 5625
.if(n = 5547 || n = 5554 || n = 5566 || n = 5570 || n = 5572 || n = 5579 || n = 5584 || n = 5593 || n = 5604 || n = 5616 || n = 5620)
	; 14..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 5548 || n = 5549 || n = 5552 || n = 5553 || (n > 5554 && n < 5558) || n = 5560 || n = 5565 || n = 5568 || n = 5571 || (n > 5572 && n < 5576) || n = 5578 || n = 5580 || n = 5582 || n = 5583 || n = 5585 || n = 5592 || (n > 5593 && n < 5598) || n = 5600 || n = 5602 || n = 5603 || (n > 5604 && n < 5608) || n = 5614 || n = 5615 || n = 5618 || n = 5619 || (n > 5620 && n < 5624))
	; 13..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 5550 || n = 5581 || n = 5612)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 5551 || n = 5576 || n = 5601)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5558 || n = 5562)
	; 16 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 5559 || n = 5609)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5561 || n = 5577 || n = 5586 || n = 5608)
	; 16..17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5563 || n = 5617)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5564)
	; 17 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 5567 || n = 5590 || n = 5613)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 5569)
	; 13 bytes
	ldy #192
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5587 || n = 5624)
	; 16..17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5588 || n = 5610 || n = 5625)
	; 16..17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5589)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 5591)
	; 17 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 5598)
	; 13 bytes
	ldy #193
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5599)
	; 17 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 5611)
	; 15 bytes
	ldy #170
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
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
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5627)
	; 13 bytes
	ldy #194
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5628 || n = 5629 || (n > 5630 && n < 5635) || n = 5638 || (n > 5639 && n < 5643) || n = 5645 || n = 5646 || (n > 5647 && n < 5651) || n = 5653 || n = 5655 || n = 5657 || n = 5658 || n = 5660 || n = 5661 || (n > 5662 && n < 5667) || n = 5673 || n = 5675 || n = 5678 || n = 5680 || n = 5681 || n = 5684 || n = 5686 || n = 5687 || (n > 5688 && n < 5692) || n = 5700 || n = 5702)
	; 13..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 5630 || n = 5639 || n = 5647 || n = 5654 || n = 5662 || n = 5679 || n = 5688)
	; 14..15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 5635)
	; 17 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5636 || n = 5659 || n = 5682)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 5637 || n = 5669 || n = 5692 || n = 5695)
	; 16..17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5643 || n = 5674)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 5644 || n = 5671 || n = 5698)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5652)
	; 17 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5656)
	; 13 bytes
	ldy #195
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5667)
	; 16 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5668 || n = 5683 || n = 5694 || n = 5697)
	; 16..17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5670 || n = 5693)
	; 16 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5672)
	; 17 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5677)
	; 15 bytes
	ldy #172
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 5685)
	; 13 bytes
	ldy #196
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5696)
	; 17 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5699)
	; 17 bytes
	ldy #154
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 5703)
	; 11 bytes
	ldy #248 ;hides 'sed'
	delay_n 4
	delay_n 12
	dey
	Jne *-8
.endif
.else
; 5704 to 5781
.if(n = 5704 || n = 5706 || n = 5708 || n = 5717 || n = 5729 || n = 5731 || n = 5739 || n = 5746 || n = 5773 || n = 5775 || n = 5777)
	; 13..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 5705 || n = 5728 || n = 5751 || n = 5774)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 5707 || n = 5709 || n = 5711 || n = 5712 || n = 5716 || (n > 5717 && n < 5722) || n = 5727 || n = 5730 || (n > 5731 && n < 5736) || n = 5738 || n = 5745 || (n > 5746 && n < 5750) || (n > 5752 && n < 5759) || n = 5769 || n = 5771 || n = 5778 || n = 5780 || n = 5781)
	; 13..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 5710)
	; 15 bytes
	ldy #173
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 5713 || n = 5761)
	; 15..16 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5714)
	; 13 bytes
	ldy #197
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5715 || n = 5740 || n = 5763)
	; 15..16 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5722 || n = 5768)
	; 17 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 5723 || n = 5744)
	; 16..17 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 5724)
	; 16 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 5725 || n = 5752 || n = 5779)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5726 || n = 5776)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5736 || n = 5767)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 5737)
	; 16 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 5741)
	; 16 bytes
	ldy #164
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 5742 || n = 5765)
	; 17 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5743)
	; 13 bytes
	ldy #198
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5750)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 5759 || n = 5760)
	; 16..17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5762)
	; 16 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5764 || n = 5766)
	; 16..17 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 5770)
	; 14 bytes
	ldy #200 ;hides 'iny'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	dey
	Jne *-11
.elseif(n = 5772)
	; 13 bytes
	ldy #199
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
; 5782 to 5859
.if(n = 5782 || n = 5800 || n = 5802 || n = 5804 || n = 5809 || n = 5823 || n = 5836 || n = 5846 || n = 5852 || n = 5854)
	; 14..15 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 5782 && n < 5786) || n = 5803 || n = 5805 || n = 5807 || n = 5808 || (n > 5809 && n < 5813) || n = 5822 || n = 5824 || n = 5825 || n = 5827 || n = 5828 || n = 5832 || n = 5834 || n = 5835 || (n > 5836 && n < 5840) || n = 5844 || n = 5845 || n = 5847 || n = 5848 || n = 5850 || n = 5853 || (n > 5854 && n < 5859))
	; 13..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 5786 || n = 5788 || n = 5791 || n = 5813 || n = 5818)
	; 16..17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5787 || n = 5817)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5789)
	; 16 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 5790)
	; 17 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5792 || n = 5815)
	; 17 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 5793 || n = 5814)
	; 16..17 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 5794 || n = 5796)
	; 16..17 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 5795 || n = 5841)
	; 17 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 5797 || n = 5820 || n = 5843)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 5798 || n = 5829)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 5799)
	; 12 bytes
	ldy #24 ;hides 'clc'
	delay_n 6
	delay_n 12
	iny
	Jne *-9
.elseif(n = 5801 || n = 5826 || n = 5851)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5806 || n = 5833)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5816 || n = 5840)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5819)
	; 16 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 5821)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 5830)
	; 13 bytes
	ldy #201
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5831)
	; 13 bytes
	ldy #216 ;hides 'cld'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-10
.elseif(n = 5842)
	; 15 bytes
	ldy #177
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 5849)
	; 12 bytes
	ldy #234 ;hides 'nop'
	delay_n 6
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5859)
	; 13 bytes
	ldy #202
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.endif
.else
; 5860 to 5937
.if(n = 5860 || n = 5887 || n = 5914)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5861 || n = 5862 || n = 5864 || n = 5865 || n = 5867 || n = 5868 || (n > 5869 && n < 5874) || n = 5877 || n = 5878 || (n > 5879 && n < 5884) || n = 5890 || n = 5891 || (n > 5892 && n < 5897) || n = 5903 || (n > 5904 && n < 5908) || n = 5910 || n = 5916 || n = 5918 || n = 5919 || n = 5921 || n = 5923 || n = 5924 || n = 5928 || (n > 5929 && n < 5934))
	; 13..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 5863 || n = 5869 || n = 5879 || n = 5892 || n = 5904 || n = 5920 || n = 5925 || n = 5929)
	; 14..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 5866)
	; 12 bytes
	ldy #(n-1)/23
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 5874 || n = 5897 || n = 5909 || n = 5934)
	; 16..17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5875)
	; 15 bytes
	ldy #178
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 5876 || n = 5901 || n = 5926)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5884)
	; 16 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 5885)
	; 17 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 5886 || n = 5899 || n = 5911 || n = 5927 || n = 5936)
	; 16..17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5888)
	; 13 bytes
	ldy #203
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5889)
	; 12 bytes
	ldy #0
	delay_n 6
	delay_n 12
	dey
	Jne *-8
.elseif(n = 5898 || n = 5915)
	; 16 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 5900 || n = 5912 || n = 5937)
	; 16..17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5902)
	; 17 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5908)
	; 15 bytes
	ldy #179
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 5913)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5917)
	; 13 bytes
	ldy #204
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5922)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 5935)
	; 17 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
.if((n >= 5938) && (n < 6016))
.if((n >= 5938) && (n < 5977))
; 5938 to 5976
.if(n = 5938 || n = 5966)
	; 16..17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 5939)
	; 17 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 5940)
	; 17 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 5941 || n = 5968)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5942 || n = 5965)
	; 17 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 5943 || n = 5945 || n = 5947 || n = 5948 || n = 5950 || n = 5952 || n = 5953 || (n > 5954 && n < 5958) || n = 5970 || n = 5972 || n = 5973)
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5944 || n = 5949 || n = 5954 || n = 5971)
	; 15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 5946)
	; 13 bytes
	ldy #205
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5951 || n = 5976)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 5958)
	; 17 bytes
	ldy #161
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 5959)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5960)
	; 17 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 5961)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5962)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5963)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5964)
	; 17 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 5967)
	; 17 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 5969)
	; 17 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 5974)
	; 15 bytes
	ldy #181
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 5975)
	; 13 bytes
	ldy #206
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.endif
.else
; 5977 to 6015
.if(n = 5977 || n = 5978 || (n > 5979 && n < 5984) || n = 5986 || n = 5988 || n = 5989 || n = 5997 || n = 5999 || n = 6000 || n = 6002 || n = 6003 || n = 6005 || n = 6006 || (n > 6007 && n < 6011))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 5979 || n = 5987 || n = 5998 || n = 6007)
	; 15..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 5984 || n = 6015)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 5985 || n = 6011 || n = 6014)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5990)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 5991)
	; 17 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 5992)
	; 17 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 5993)
	; 17 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 5994)
	; 17 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5995)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 5996)
	; 17 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 6001)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6004)
	; 13 bytes
	ldy #207
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6012)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6013)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.else
; 6016 to 6093
.if(n = 6016 || n = 6034 || n = 6045 || n = 6063 || n = 6074 || n = 6088)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6017 || n = 6019 || n = 6020 || n = 6023 || n = 6024 || n = 6027 || n = 6028 || (n > 6029 && n < 6033) || n = 6035 || (n > 6036 && n < 6040) || n = 6042 || n = 6048 || n = 6050 || n = 6053 || (n > 6054 && n < 6059) || n = 6064 || (n > 6065 && n < 6069) || n = 6075 || n = 6078 || (n > 6079 && n < 6084) || n = 6093)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6018 || n = 6025 || n = 6029 || n = 6036 || n = 6052 || n = 6054 || n = 6065 || n = 6079)
	; 15..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 6021)
	; 16 bytes
	ldy #172
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 6022 || n = 6049)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 6026 || n = 6051 || n = 6076)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6033)
	; 13 bytes
	ldy #208
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6040)
	; 15 bytes
	ldy #183
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6041 || n = 6084)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6043 || n = 6059 || n = 6061 || n = 6072 || n = 6086)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6044 || n = 6071)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 6046 || n = 6077)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6047 || n = 6070)
	; 17 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 6060)
	; 17 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 6062)
	; 13 bytes
	ldy #209
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6069)
	; 17 bytes
	ldy #164
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6073)
	; 15 bytes
	ldy #184
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6085)
	; 17 bytes
	ldy #156
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6087)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6089)
	; 18 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 6090)
	; 18 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 6091)
	; 13 bytes
	ldy #210
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6092)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.endif
.endif
.else
.if((n >= 6094) && (n < 6172))
; 6094 to 6171
.if(n = 6094 || n = 6104 || n = 6106 || n = 6111 || n = 6123 || n = 6129 || n = 6133 || n = 6142 || n = 6152 || n = 6154 || n = 6160 || n = 6171)
	; 15..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 6094 && n < 6099) || n = 6105 || n = 6107 || n = 6109 || n = 6110 || n = 6112 || n = 6122 || n = 6124 || n = 6125 || n = 6127 || n = 6128 || n = 6131 || n = 6132 || (n > 6133 && n < 6138) || n = 6141 || (n > 6142 && n < 6146) || n = 6153 || n = 6155 || n = 6156 || n = 6158 || n = 6159 || (n > 6160 && n < 6165))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6099 || n = 6165)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6100 || n = 6150)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 6101 || n = 6126 || n = 6151)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6102 || n = 6114)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6103 || n = 6130 || n = 6157)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 6108 || n = 6139 || n = 6170)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6113 || n = 6140 || n = 6167)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6115 || n = 6138 || n = 6169)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6116)
	; 18 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 6117 || n = 6119 || n = 6146)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6118)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6120)
	; 13 bytes
	ldy #211
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6121 || n = 6148)
	; 18 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 6147)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 6149)
	; 13 bytes
	ldy #212
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6166)
	; 18 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6168)
	; 14 bytes
	ldy #200 ;hides 'iny'
	delay_n 10
	delay_n 12
	dey
	dey
	Jne *-11
.endif
.else
; 6172 to 6250
.if(n = 6172)
	; 15 bytes
	ldy #187
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6173 || n = 6179 || n = 6181 || n = 6187 || n = 6202 || n = 6204 || n = 6210 || n = 6214 || n = 6229 || n = 6235 || n = 6239 || n = 6241)
	; 14..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 6174)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 6175 || n = 6198 || n = 6200 || n = 6225 || n = 6227 || n = 6248)
	; 16..17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 6176 || n = 6201 || n = 6226)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6177)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6178)
	; 13 bytes
	ldy #213
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6180 || n = 6182 || n = 6183 || n = 6185 || n = 6186 || (n > 6187 && n < 6192) || n = 6203 || n = 6205 || n = 6206 || n = 6208 || n = 6209 || n = 6212 || n = 6213 || (n > 6214 && n < 6219) || n = 6228 || n = 6230 || n = 6231 || n = 6233 || n = 6234 || n = 6237 || n = 6240 || (n > 6241 && n < 6246))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6184 || n = 6211 || n = 6238)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 6192)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6193)
	; 18 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6194 || n = 6221 || n = 6246)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6195 || n = 6222 || n = 6247 || n = 6249)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6196)
	; 16 bytes
	ldy #177
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 6197 || n = 6220)
	; 17..18 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 6199)
	; 12 bytes
	ldy #248 ;hides 'sed'
	delay_n 6
	delay_n 12
	dey
	Jne *-9
.elseif(n = 6207)
	; 13 bytes
	ldy #214
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6219 || n = 6223 || n = 6250)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6224)
	; 18 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 6232)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6236)
	; 13 bytes
	ldy #215
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
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
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6252 || n = 6264 || n = 6318)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6253 || (n > 6254 && n < 6259) || n = 6267 || n = 6269 || n = 6270 || n = 6272 || n = 6273 || n = 6278 || (n > 6279 && n < 6284) || n = 6296 || (n > 6297 && n < 6301) || n = 6303 || (n > 6304 && n < 6309) || n = 6321 || n = 6324 || n = 6325 || n = 6327 || n = 6328)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6254 || n = 6266 || n = 6268 || n = 6279 || n = 6295 || n = 6297 || n = 6304 || n = 6320 || n = 6322)
	; 15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 6259 || n = 6284 || n = 6309)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6260 || n = 6285 || n = 6314)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 6261 || n = 6275 || n = 6286 || n = 6302 || n = 6311)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6262 || n = 6287 || n = 6312)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6263)
	; 13 bytes
	ldy #24 ;hides 'clc'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	iny
	Jne *-10
.elseif(n = 6265 || n = 6292 || n = 6319)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 6271)
	; 15 bytes
	ldy #190
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6274 || n = 6315)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6277 || n = 6288 || n = 6313)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6289 || n = 6316)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 6290)
	; 17 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6291)
	; 17 bytes
	ldy #170
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6293)
	; 18 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 6294)
	; 13 bytes
	ldy #217
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6310)
	; 17 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 6317)
	; 13 bytes
	ldy #234 ;hides 'nop'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6323)
	; 13 bytes
	ldy #218
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.endif
.else
; 6329 to 6406
.if(n = 6329 || n = 6340 || n = 6349 || n = 6355 || n = 6359 || n = 6379 || n = 6384 || n = 6390 || n = 6404)
	; 15..17 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 6329 && n < 6333) || n = 6339 || n = 6348 || n = 6350 || n = 6353 || n = 6354 || n = 6357 || n = 6358 || n = 6360 || n = 6372 || n = 6374 || n = 6375 || n = 6377 || n = 6378 || n = 6380 || n = 6382 || n = 6383 || n = 6385 || n = 6386 || n = 6389 || n = 6391 || n = 6392 || n = 6402 || n = 6403 || n = 6405 || n = 6406)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6333 || n = 6361 || n = 6362)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6334)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6335 || n = 6338 || n = 6363 || n = 6364 || n = 6388 || n = 6393)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6336)
	; 16 bytes
	ldy #181
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 6337)
	; 15 bytes
	ldy #192
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6341 || n = 6343 || n = 6366 || n = 6368 || n = 6395)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 6342 || n = 6367)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6344 || n = 6398)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6345)
	; 17 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 6346 || n = 6373 || n = 6400)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 6347 || n = 6394)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 6351 || n = 6376)
	; 13 bytes
	ldy #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6352)
	; 13 bytes
	ldy #219
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6356 || n = 6387)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6365)
	; 17 bytes
	ldy #172
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6369 || n = 6396)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 6370)
	; 15 bytes
	ldy #193
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6371)
	; 16 bytes
	ldy #182
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 6381)
	; 13 bytes
	ldy #220
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6397)
	; 17 bytes
	ldy #164
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6399)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6401)
	; 13 bytes
	ldy #0
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-9
.endif
.endif
.else
.if((n >= 6407) && (n < 6485))
; 6407 to 6484
.if(n = 6407 || n = 6408 || n = 6412 || (n > 6413 && n < 6418) || n = 6420 || (n > 6421 && n < 6425) || n = 6429 || (n > 6430 && n < 6435) || n = 6438 || n = 6440 || n = 6441 || (n > 6442 && n < 6447) || n = 6451 || n = 6453 || n = 6455 || n = 6456 || (n > 6457 && n < 6462) || n = 6470 || (n > 6471 && n < 6475) || n = 6482 || n = 6483)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6409 || n = 6435 || n = 6447 || n = 6462)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6410)
	; 13 bytes
	ldy #221
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6411 || n = 6437 || n = 6464 || n = 6478)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6413 || n = 6421 || n = 6430 || n = 6442 || n = 6452 || n = 6457 || n = 6471 || n = 6484)
	; 15..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 6418 || n = 6449 || n = 6480)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6419)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6425 || n = 6450)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 6426)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6427 || n = 6454 || n = 6481)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 6428)
	; 18 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 6436)
	; 15 bytes
	ldy #195
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6439)
	; 13 bytes
	ldy #222
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6448)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 6463)
	; 18 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6465)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6466)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6467)
	; 18 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 6468)
	; 13 bytes
	ldy #223
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6469)
	; 15 bytes
	ldy #196
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6475)
	; 17 bytes
	ldy #166
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6476)
	; 16 bytes
	ldy #185
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 6477)
	; 18 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 6479)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.endif
.else
; 6485 to 6562
.if((n > 6484 && n < 6489) || n = 6499 || n = 6501 || n = 6503 || n = 6504 || n = 6506 || n = 6510 || n = 6512 || n = 6513 || n = 6515 || n = 6516 || n = 6528 || (n > 6529 && n < 6534) || n = 6537 || (n > 6538 && n < 6542) || n = 6544 || n = 6546 || n = 6548 || n = 6557 || (n > 6558 && n < 6562))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6489 || n = 6534 || n = 6543)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6490)
	; 18 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6491 || n = 6507 || n = 6518 || n = 6536)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6492 || n = 6519)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6493 || n = 6509 || n = 6520 || n = 6547)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6494 || n = 6521 || n = 6522)
	; 18 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 6495 || n = 6549)
	; 18 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 6496 || n = 6523)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6497)
	; 13 bytes
	ldy #224
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6498 || n = 6525)
	; 18 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 6500 || n = 6505 || n = 6514 || n = 6529 || n = 6538 || n = 6545 || n = 6558)
	; 15..17 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 6502)
	; 15 bytes
	ldy #197
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6508 || n = 6535 || n = 6562)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 6511 || n = 6542)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6517)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 6524 || n = 6551)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6526)
	; 13 bytes
	ldy #225
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6527)
	; 18 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 6550)
	; 17 bytes
	ldy #177
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6552)
	; 18 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6553)
	; 17 bytes
	ldy #168
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6554)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6555)
	; 13 bytes
	ldy #226
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6556)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.endif
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
.if((n >= 6563) && (n < 6602))
; 6563 to 6601
.if(n = 6563 || n = 6597)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6564 || n = 6567 || n = 6570 || n = 6575 || (n > 6576 && n < 6580) || n = 6583 || n = 6586 || n = 6588 || n = 6590 || n = 6591 || (n > 6592 && n < 6596))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6565 || n = 6569 || n = 6571 || n = 6576 || n = 6587 || n = 6592)
	; 15..17 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 6566)
	; 14 bytes
	ldy #200 ;hides 'iny'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	dey
	Jne *-11
.elseif(n = 6568)
	; 15 bytes
	ldy #199
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6572)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6573)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6574 || n = 6596)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6580)
	; 18 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 6581)
	; 16 bytes
	ldy #188
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 6582)
	; 18 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 6584)
	; 13 bytes
	ldy #227
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6585)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 6589)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 6598)
	; 18 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6599)
	; 14 bytes
	ldy #56 ;hides 'sec'
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-11
.elseif(n = 6600)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6601)
	; 15 bytes
	ldy #200
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.endif
.else
; 6602 to 6640
.if(n = 6602 || n = 6607 || n = 6619 || n = 6638)
	; 15..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 6603 || n = 6605 || n = 6606 || (n > 6607 && n < 6611) || n = 6615 || n = 6617 || n = 6618 || (n > 6619 && n < 6623) || n = 6636 || n = 6637 || n = 6639 || n = 6640)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6604 || n = 6635)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6611 || n = 6633)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 6612)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 6613)
	; 13 bytes
	ldy #228
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6614)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6616)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 6623 || n = 6626)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6624)
	; 17 bytes
	ldy #179
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6625 || n = 6628)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6627)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6629)
	; 18 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 6630)
	; 18 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 6631)
	; 17 bytes
	ldy #170
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6632)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6634)
	; 15 bytes
	ldy #201
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.endif
.endif
.else
.if((n >= 6641) && (n < 6680))
; 6641 to 6679
.if(n = 6641 || n = 6644 || n = 6645 || (n > 6646 && n < 6651) || n = 6668 || n = 6672 || n = 6673 || (n > 6674 && n < 6679))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6642)
	; 13 bytes
	ldy #229
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6643 || n = 6670)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 6646 || n = 6669 || n = 6674)
	; 15..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 6651)
	; 16 bytes
	ldy #190
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 6652 || n = 6653)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6654 || n = 6655)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6656)
	; 18 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 6657)
	; 18 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 6658)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6659)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6660)
	; 18 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 6661)
	; 17 bytes
	ldy #180
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6662 || n = 6665)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 6663)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 6664)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 6666)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6667)
	; 15 bytes
	ldy #202
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6671)
	; 13 bytes
	ldy #230
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6679)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.endif
.else
; 6680 to 6718
.if(n = 6680 || n = 6681 || n = 6707 || n = 6710)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6682 || n = 6683 || n = 6709 || n = 6712)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6684)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6685)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6686)
	; 16 bytes
	ldy #191
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 6687 || n = 6714)
	; 18 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 6688 || n = 6699 || n = 6701 || n = 6702 || (n > 6703 && n < 6707))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6689 || n = 6716)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6690 || n = 6717)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 6691 || n = 6692)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 6693)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 6694)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 6695)
	; 13 bytes
	ldy #248 ;hides 'sed'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6696)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6697)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 6698 || n = 6703)
	; 15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 6700)
	; 13 bytes
	ldy #231
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6708 || n = 6711)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6713)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6715)
	; 18 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 6718)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.endif
.endif
.endif
.else
.if((n >= 6719) && (n < 6797))
.if((n >= 6719) && (n < 6758))
; 6719 to 6757
.if(n = 6719 || n = 6720 || n = 6744 || n = 6746 || n = 6749)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 6721)
	; 16 bytes
	ldy #192
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 6722)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6723 || n = 6726 || n = 6731 || (n > 6732 && n < 6736) || n = 6753 || (n > 6754 && n < 6758))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6724 || n = 6751)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 6725)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6727)
	; 13 bytes
	ldy #24 ;hides 'clc'
	delay_n 10
	delay_n 12
	iny
	Jne *-10
.elseif(n = 6728)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6729)
	; 13 bytes
	ldy #232
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6730 || n = 6732 || n = 6754)
	; 15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 6736 || n = 6741)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6737 || n = 6739)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6738 || n = 6740)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6742)
	; 18 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 6743)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6745)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 6747)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 6748)
	; 17 bytes
	ldy #173
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6750)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6752)
	; 18 bytes
	delay_n 136
	delay_n (n-136)
.endif
.else
; 6758 to 6796
.if(n = 6758)
	; 13 bytes
	ldy #233
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6759 || n = 6790)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6760 || (n > 6761 && n < 6766) || n = 6768 || n = 6780 || (n > 6781 && n < 6785) || n = 6789 || n = 6791 || n = 6792 || (n > 6793 && n < 6797))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6761 || n = 6769 || n = 6781 || n = 6788 || n = 6793)
	; 15..17 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 6766)
	; 15 bytes
	ldy #205
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6767)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6770)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6771)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 6772)
	; 17 bytes
	ldy #183
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6773 || n = 6776)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 6774)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 6775)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 6777)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6778)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 6779)
	; 18 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 6785)
	; 13 bytes
	ldy #234 ;hides 'nop'
	delay_n 10
	delay_n 12
	dey
	Jne *-10
.elseif(n = 6786)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6787)
	; 13 bytes
	ldy #234
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.endif
.endif
.else
; 6797 to 6875
.if(n = 6797 || n = 6815 || n = 6842 || n = 6869)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6798 || n = 6843 || n = 6870)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6799)
	; 15 bytes
	ldy #206
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6800 || n = 6827)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 6801 || n = 6803 || n = 6807 || (n > 6808 && n < 6813) || n = 6818 || n = 6820 || n = 6822 || n = 6823 || n = 6825 || n = 6834 || (n > 6835 && n < 6840) || n = 6847 || (n > 6848 && n < 6852) || n = 6854 || n = 6856 || n = 6858 || n = 6861 || n = 6863 || n = 6864 || n = 6866 || n = 6867)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6802 || n = 6808 || n = 6819 || n = 6824 || n = 6835 || n = 6848 || n = 6855 || n = 6862 || n = 6868)
	; 15..17 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 6804 || n = 6831)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6805 || n = 6832 || n = 6859)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 6806 || n = 6833 || n = 6860)
	; 18 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 6813 || n = 6840 || n = 6853)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6814 || n = 6841)
	; 18 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6816)
	; 13 bytes
	ldy #235
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6817 || n = 6828 || n = 6844 || n = 6857 || n = 6871)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6821 || n = 6852)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6826)
	; 16 bytes
	ldy #195
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 6829)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 6830)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6845)
	; 13 bytes
	ldy #236
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6846)
	; 17 bytes
	ldy #185
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6865)
	; 15 bytes
	ldy #208
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6872)
	; 18 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 6873)
	; 18 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 6874)
	; 13 bytes
	ldy #237
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6875)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.endif
.endif
.endif
.endif
.else
.if((n >= 6876) && (n < 7188))
.if((n >= 6876) && (n < 7032))
.if((n >= 6876) && (n < 6954))
.if((n >= 6876) && (n < 6915))
; 6876 to 6914
.if(n = 6876 || (n > 6877 && n < 6882) || n = 6885 || n = 6887 || n = 6888 || (n > 6889 && n < 6894) || n = 6900 || n = 6902 || n = 6905 || (n > 6906 && n < 6911))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6877 || n = 6889 || n = 6901 || n = 6906)
	; 15..17 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 6882 || n = 6894 || n = 6911)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6883 || n = 6914)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6884)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6886)
	; 13 bytes
	ldy #(n-1)/27
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.elseif(n = 6895)
	; 18 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6896)
	; 16 bytes
	ldy #197
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 6897)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6898)
	; 15 bytes
	ldy #209
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6899)
	; 18 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 6903)
	; 13 bytes
	ldy #238
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6904)
	; 17 bytes
	ldy #177
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6912)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6913)
	; 13 bytes
	ldy #0
	delay_n 10
	delay_n 12
	dey
	Jne *-9
.endif
.else
; 6915 to 6953
.if(n = 6915 || (n > 6916 && n < 6920) || n = 6933 || n = 6934 || (n > 6935 && n < 6940) || n = 6947 || (n > 6948 && n < 6952))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6916 || n = 6935 || n = 6948)
	; 15..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 6920)
	; 17 bytes
	ldy #187
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6921 || n = 6940)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6922)
	; 18 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6923 || n = 6942)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6924)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6925 || n = 6944)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6926 || n = 6953)
	; 18 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 6927)
	; 18 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 6928)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6929)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6930)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6931)
	; 15 bytes
	ldy #210
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6932)
	; 13 bytes
	ldy #239
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6941)
	; 18 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 6943)
	; 17 bytes
	ldy #178
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6945)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6946)
	; 18 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6952)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.endif
.endif
.else
.if((n >= 6954) && (n < 6993))
.if((n >= 6954) && (n < 6973))
; 6954 to 6972
.if(n = 6954)
	; 18 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 6955)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6956)
	; 18 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6957)
	; 17 bytes
	ldy #188
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6958)
	; 18 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 6959)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6960)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 6961)
	; 13 bytes
	ldy #240
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6962)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 6963 || (n > 6964 && n < 6969))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6964)
	; 15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 6969)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6970)
	; 18 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 6971)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6972)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.endif
.else
; 6973 to 6992
.if(n = 6973)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6974)
	; 18 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 6975)
	; 18 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 6976)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 6977)
	; 18 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 6978 || n = 6980 || n = 6981 || n = 6992)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6979)
	; 16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 6982)
	; 17 bytes
	ldy #179
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 6983)
	; 18 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 6984)
	; 18 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 6985)
	; 18 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 6986)
	; 18 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 6987)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 6988)
	; 18 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 6989)
	; 18 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 6990)
	; 13 bytes
	ldy #241
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 6991)
	; 18 bytes
	delay_n 186
	delay_n (n-186)
.endif
.endif
.else
; 6993 to 7031
.if(n = 6993 || n = 7010 || n = 7022)
	; 15..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 6993 && n < 6997) || n = 7003 || n = 7009 || n = 7011 || n = 7013 || n = 7021 || (n > 7022 && n < 7027))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6997)
	; 15 bytes
	ldy #212
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 6998 || n = 7027)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6999)
	; 15 bytes
	ldy #56 ;hides 'sec'
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-12
.elseif(n = 7000 || n = 7029)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7001)
	; 16 bytes
	ldy #200
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7002)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7004)
	; 18 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7005)
	; 18 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 7006)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7007)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7008)
	; 18 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 7012)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7014)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 7015)
	; 18 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 7016)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7017)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 7018)
	; 18 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 7019)
	; 13 bytes
	ldy #242
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 7020)
	; 18 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 7028)
	; 18 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 7030)
	; 15 bytes
	ldy #213
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7031)
	; 17 bytes
	ldy #190
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.endif
.endif
.endif
.else
.if((n >= 7032) && (n < 7110))
.if((n >= 7032) && (n < 7071))
; 7032 to 7070
.if(n = 7032 || n = 7034 || n = 7040 || (n > 7041 && n < 7045) || n = 7050 || (n > 7051 && n < 7056) || n = 7065 || n = 7067 || n = 7070)
	; 14..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7033 || n = 7041 || n = 7051 || n = 7066)
	; 15..17 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7035 || n = 7062)
	; 18 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 7036)
	; 16 bytes
	ldy #201
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7037 || n = 7064)
	; 18 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7038 || n = 7069)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7039)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7045)
	; 18 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 7046)
	; 18 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7047)
	; 18 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 7048)
	; 13 bytes
	ldy #243
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 7049)
	; 18 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 7056)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7057)
	; 18 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 7058)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7059)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7060)
	; 17 bytes
	ldy #181
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7061)
	; 18 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 7063)
	; 15 bytes
	ldy #214
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7068)
	; 17 bytes
	ldy #191
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.endif
.else
; 7071 to 7109
.if(n = 7071 || (n > 7072 && n < 7076) || n = 7079 || (n > 7080 && n < 7085) || n = 7098 || n = 7102 || n = 7104 || n = 7107 || n = 7108)
	; 14..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7072 || n = 7080 || n = 7103 || n = 7109)
	; 15..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7076)
	; 18 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7077)
	; 13 bytes
	ldy #244
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 7078)
	; 18 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 7085)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7086)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7087)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7088)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7089)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7090)
	; 18 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 7091)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 7092)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 7093)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7094)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7095)
	; 18 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 7096)
	; 15 bytes
	ldy #215
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7097)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7099)
	; 17 bytes
	ldy #182
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7100)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7101)
	; 18 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7105)
	; 17 bytes
	ldy #192
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7106)
	; 13 bytes
	ldy #245
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.endif
.endif
.else
.if((n >= 7110) && (n < 7149))
; 7110 to 7148
.if((n > 7109 && n < 7114) || n = 7133 || n = 7136 || n = 7137 || (n > 7138 && n < 7142) || n = 7144)
	; 14..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7114 || n = 7143)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7115)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7116 || n = 7145)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7117 || n = 7146)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7118 || n = 7147)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7119 || n = 7148)
	; 18 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 7120)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 7121)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 7122)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7123)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7124)
	; 18 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 7125)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7126)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7127)
	; 14 bytes
	ldy #216 ;hides 'cld'
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7128)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7129)
	; 15 bytes
	ldy #216
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7130 || n = 7132 || n = 7134 || n = 7138)
	; 15..17 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7131)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7135)
	; 13 bytes
	ldy #246
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 7142)
	; 17 bytes
	ldy #193
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.endif
.else
; 7149 to 7187
.if(n = 7149)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 7150)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 7151 || n = 7180)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7152)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7153 || n = 7182)
	; 18 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 7154 || n = 7183)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7155 || n = 7184)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7156 || n = 7185)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 7157 || n = 7186)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7158 || n = 7187)
	; 18 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7159)
	; 18 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7160)
	; 18 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7161)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7162)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7163)
	; 18 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7164)
	; 13 bytes
	ldy #247
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 7165 || n = 7167)
	; 15..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7166 || (n > 7167 && n < 7172) || n = 7178 || n = 7181)
	; 14..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7172)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7173)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7174)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7175)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7176)
	; 16 bytes
	ldy #205
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7177)
	; 17 bytes
	ldy #184
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7179)
	; 17 bytes
	ldy #194
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.endif
.endif
.endif
.endif
.else
.if((n >= 7188) && (n < 7344))
.if((n >= 7188) && (n < 7266))
.if((n >= 7188) && (n < 7227))
.if((n >= 7188) && (n < 7207))
; 7188 to 7206
.if(n = 7188)
	; 18 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7189)
	; 18 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7190)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7191)
	; 13 bytes
	ldy #24 ;hides 'clc'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-10
.elseif(n = 7192)
	; 18 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7193)
	; 13 bytes
	ldy #248
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 7194 || n = 7196)
	; 15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7195 || (n > 7196 && n < 7201))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7201 || n = 7203)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7202 || n = 7204)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7205)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7206)
	; 18 bytes
	delay_n 71
	delay_n (n-71)
.endif
.else
; 7207 to 7226
.if(n = 7207)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 7208)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 7209)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7210)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7211)
	; 16 bytes
	ldy #206
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7212)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7213)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7214)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 7215)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7216)
	; 17 bytes
	ldy #195
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7217)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7218 || n = 7224 || n = 7226)
	; 14..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7219)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7220)
	; 18 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7221)
	; 18 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7222)
	; 13 bytes
	ldy #249
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 7223)
	; 18 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 7225)
	; 15 bytes
	delay_n (n-3)
	ldy $A4
.endif
.endif
.else
; 7227 to 7265
.if(n = 7227 || n = 7229 || n = 7230 || n = 7248 || n = 7257 || n = 7259 || n = 7260 || n = 7263)
	; 15..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7228)
	; 15 bytes
	ldy #219
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7231 || n = 7254 || n = 7256 || n = 7258 || n = 7264)
	; 15..17 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7232)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7233 || n = 7262)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7234 || n = 7265)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7235)
	; 18 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 7236)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 7237)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 7238)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7239)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7240 || n = 7242)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7241)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7243)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 7244)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7245)
	; 18 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7246)
	; 16 bytes
	ldy #207
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7247)
	; 18 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7249)
	; 18 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7250)
	; 18 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7251)
	; 13 bytes
	ldy #250
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 7252)
	; 18 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 7253)
	; 13 bytes
	ldy #234 ;hides 'nop'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7255)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7261)
	; 15 bytes
	ldy #220
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.endif
.endif
.else
.if((n >= 7266) && (n < 7305))
; 7266 to 7304
.if(n = 7266 || n = 7295)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 7267)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7268)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7269)
	; 18 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 7270)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7271 || n = 7300 || n = 7302)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7272 || n = 7301)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 7273)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7274 || n = 7303)
	; 18 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7275 || n = 7304)
	; 18 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7276)
	; 18 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7277)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7278)
	; 18 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7279)
	; 18 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7280)
	; 13 bytes
	ldy #251
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 7281)
	; 16 bytes
	ldy #208
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7282 || n = 7284 || n = 7285 || n = 7287 || n = 7288 || n = 7290 || n = 7291 || n = 7296 || n = 7298)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7283 || n = 7289 || n = 7297)
	; 15..17 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7286)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7292)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7293)
	; 18 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 7294)
	; 15 bytes
	ldy #221
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7299)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.endif
.else
; 7305 to 7343
.if(n = 7305 || n = 7334)
	; 18 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7306 || n = 7335)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7307 || n = 7336)
	; 18 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7308 || n = 7337)
	; 18 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7309)
	; 13 bytes
	ldy #252
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 7310 || n = 7339)
	; 18 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 7311 || (n > 7312 && n < 7316) || n = 7318 || n = 7319 || (n > 7320 && n < 7324) || n = 7329 || n = 7331 || n = 7340 || n = 7342 || n = 7343)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7312 || n = 7320 || n = 7330 || n = 7341)
	; 15..17 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7316)
	; 16 bytes
	ldy #209
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7317)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7324)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 7325)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7326)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7327)
	; 15 bytes
	ldy #222
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7328)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7332)
	; 18 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7333)
	; 17 bytes
	ldy #188
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7338)
	; 13 bytes
	ldy #253
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.endif
.endif
.endif
.else
.if((n >= 7344) && (n < 7422))
.if((n >= 7344) && (n < 7383))
; 7344 to 7382
.if(n = 7344 || n = 7345 || n = 7350 || (n > 7351 && n < 7355) || n = 7362 || n = 7366 || n = 7369 || (n > 7370 && n < 7375) || n = 7381)
	; 14..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7346 || n = 7375)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7347 || n = 7376)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7348 || n = 7379)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7349 || n = 7378)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7351 || n = 7363 || n = 7370 || n = 7382)
	; 15..17 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7355)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7356)
	; 18 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 7357)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7358)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7359)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 7360)
	; 15 bytes
	ldy #223
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7361)
	; 18 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7364)
	; 17 bytes
	ldy #199
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7365)
	; 18 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7367)
	; 13 bytes
	ldy #254
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 7368)
	; 18 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 7377)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7380)
	; 18 bytes
	delay_n 71
	delay_n (n-71)
.endif
.else
; 7383 to 7421
.if((n > 7382 && n < 7386) || n = 7388 || n = 7395 || n = 7397 || n = 7398 || (n > 7399 && n < 7404) || n = 7412 || n = 7414 || n = 7415)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7386)
	; 16 bytes
	ldy #211
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7387 || n = 7416)
	; 17 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7389 || n = 7418)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7390 || n = 7419)
	; 18 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7391 || n = 7420)
	; 18 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7392)
	; 18 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7393)
	; 15 bytes
	ldy #224
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7394)
	; 18 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7396)
	; 13 bytes
	ldy #255
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 7399 || n = 7413)
	; 15..16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7404)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7405)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7406)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7407)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7408)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7409)
	; 18 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 7410)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7411)
	; 17 bytes
	ldy #190
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7417)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 7421)
	; 16 bytes
	ldy #212
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.endif
.endif
.else
.if((n >= 7422) && (n < 7461))
.if((n >= 7422) && (n < 7441))
; 7422 to 7440
.if(n = 7422)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7423 || n = 7427 || (n > 7428 && n < 7433) || n = 7440)
	; 14..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7424)
	; 18 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7425)
	; 13 bytes
	ldy #0
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-9
.elseif(n = 7426)
	; 15 bytes
	ldy #225
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7428)
	; 15 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7433)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7434)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7435)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7436)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7437)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7438)
	; 17 bytes
	ldy #201
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7439)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.endif
.else
; 7441 to 7460
.if(n = 7441)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7442)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7443 || (n > 7444 && n < 7448) || n = 7458)
	; 15..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7444)
	; 16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7448)
	; 18 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7449)
	; 18 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7450)
	; 17 bytes
	ldy #191
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7451)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7452)
	; 18 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7453)
	; 18 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7454)
	; 18 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 7455)
	; 18 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 7456)
	; 16 bytes
	ldy #213
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7457)
	; 18 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 7459)
	; 15 bytes
	ldy #226
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7460)
	; 18 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.else
.if((n >= 7461) && (n < 7481))
; 7461 to 7480
.if(n = 7461 || n = 7474 || (n > 7475 && n < 7479))
	; 15..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7462 || n = 7475)
	; 16..17 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7463)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7464)
	; 18 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 7465)
	; 18 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 7466)
	; 18 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 7467)
	; 18 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 7468)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 7469)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 7470)
	; 18 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 7471)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7472)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7473)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7479)
	; 18 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7480)
	; 18 bytes
	delay_n 171
	delay_n (n-171)
.endif
.else
; 7481 to 7500
.if(n = 7481)
	; 18 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7482)
	; 18 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7483)
	; 18 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 7484)
	; 18 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 7485)
	; 18 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 7486)
	; 18 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 7487)
	; 18 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7488)
	; 18 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 7489)
	; 17 bytes
	ldy #192
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7490)
	; 18 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 7491)
	; 16 bytes
	ldy #214
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7492)
	; 15 bytes
	ldy #227
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7493 || n = 7494 || n = 7496)
	; 16..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7495)
	; 17 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7497)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 7498)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 7499)
	; 18 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 7500)
	; 18 bytes
	delay_n 191
	delay_n (n-191)
.endif
.endif
.endif
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
.if((n >= 7501) && (n < 7579))
.if((n >= 7501) && (n < 7540))
; 7501 to 7539
.if(n = 7501)
	; 18 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 7502 || n = 7531)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7503 || n = 7534)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7504)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7505 || (n > 7506 && n < 7511) || n = 7514 || n = 7527 || n = 7529 || n = 7530 || n = 7535 || n = 7536 || n = 7538 || n = 7539)
	; 15..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 7506 || n = 7537)
	; 16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7511)
	; 18 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7512)
	; 17 bytes
	ldy #203
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7513)
	; 18 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 7515)
	; 18 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7516)
	; 18 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7517)
	; 18 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 7518)
	; 18 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7519)
	; 18 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 7520)
	; 18 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 7521)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7522)
	; 18 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 7523)
	; 18 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 7524)
	; 18 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 7525)
	; 15 bytes
	ldy #228
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7526)
	; 16 bytes
	ldy #215
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7528)
	; 17 bytes
	ldy #193
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7532)
	; 18 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 7533)
	; 18 bytes
	delay_n 166
	delay_n (n-166)
.endif
.else
; 7540 to 7578
.if(n = 7540 || n = 7550 || n = 7551 || n = 7554 || n = 7555 || n = 7560 || n = 7562 || n = 7563 || n = 7567 || (n > 7568 && n < 7572) || n = 7574)
	; 15..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 7541)
	; 18 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 7542)
	; 18 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 7543 || n = 7572)
	; 18 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 7544 || n = 7575)
	; 18 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7545)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7546)
	; 18 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 7547 || n = 7576)
	; 18 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7548 || n = 7577)
	; 18 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 7549)
	; 17 bytes
	ldy #204
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7552)
	; 18 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 7553)
	; 18 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 7556)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 7557)
	; 18 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 7558)
	; 15 bytes
	ldy #229
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7559)
	; 15 bytes
	ldy #216 ;hides 'cld'
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7561)
	; 16 bytes
	ldy #216
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7564 || n = 7568)
	; 16..18 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7565)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7566)
	; 18 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7573)
	; 18 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7578)
	; 18 bytes
	delay_n 153
	delay_n (n-153)
.endif
.endif
.else
.if((n >= 7579) && (n < 7618))
; 7579 to 7617
.if(n = 7579 || n = 7584 || n = 7588 || n = 7590 || n = 7593 || n = 7595 || n = 7597 || n = 7598 || (n > 7599 && n < 7604) || n = 7608 || n = 7610 || n = 7613)
	; 15..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 7580)
	; 19 bytes
	delay_n 155
	delay_n (n-155)
.elseif(n = 7581)
	; 18 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 7582 || n = 7611)
	; 18 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 7583 || n = 7614)
	; 18 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7585)
	; 19 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7586)
	; 17 bytes
	ldy #205
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7587 || n = 7616)
	; 18 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 7589 || n = 7594 || n = 7599 || n = 7609)
	; 16..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7591)
	; 15 bytes
	ldy #230
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7592)
	; 18 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 7596)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7604)
	; 18 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7605)
	; 19 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 7606)
	; 17 bytes
	ldy #195
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7607)
	; 18 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7612)
	; 18 bytes
	ldy #177
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 7615)
	; 19 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 7617)
	; 19 bytes
	delay_n 21
	delay_n (n-21)
.endif
.else
.if((n >= 7618) && (n < 7637))
; 7618 to 7636
.if(n = 7618 || n = 7625 || n = 7626 || n = 7628 || n = 7629 || (n > 7630 && n < 7635))
	; 15..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 7619)
	; 19 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 7620)
	; 19 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7621)
	; 18 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 7622)
	; 19 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 7623)
	; 17 bytes
	ldy #206
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7624)
	; 15 bytes
	ldy #231
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7627)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7630)
	; 16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7635)
	; 18 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7636)
	; 19 bytes
	delay_n 71
	delay_n (n-71)
.endif
.else
; 7637 to 7656
.if(n = 7637)
	; 18 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7638)
	; 18 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7639)
	; 18 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7640)
	; 19 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7641)
	; 19 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 7642)
	; 19 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7643)
	; 19 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7644)
	; 19 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7645)
	; 17 bytes
	ldy #196
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7646)
	; 19 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7647 || n = 7649)
	; 18..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 7648)
	; 19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7650)
	; 19 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 7651)
	; 19 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7652)
	; 19 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7653)
	; 19 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7654)
	; 19 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7655)
	; 14 bytes
	ldy #24 ;hides 'clc'
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-11
.elseif(n = 7656)
	; 19 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.endif
.else
.if((n >= 7657) && (n < 7735))
.if((n >= 7657) && (n < 7696))
; 7657 to 7695
.if(n = 7657)
	; 15 bytes
	ldy #232
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7658 || n = 7689)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7659 || n = 7660 || (n > 7661 && n < 7666) || n = 7670 || n = 7686 || n = 7688 || n = 7691 || (n > 7692 && n < 7696))
	; 15..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 7661 || n = 7669 || n = 7671 || n = 7692)
	; 16..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7666)
	; 16 bytes
	ldy #219
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7667)
	; 18 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7668)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7672)
	; 19 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 7673)
	; 19 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7674)
	; 19 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7675)
	; 19 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7676)
	; 18 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7677)
	; 19 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7678)
	; 19 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7679)
	; 19 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 7680)
	; 19 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 7681)
	; 19 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 7682)
	; 19 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7683)
	; 19 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7684)
	; 17 bytes
	ldy #197
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7685)
	; 19 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7687)
	; 13 bytes
	ldy #248 ;hides 'sed'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7690)
	; 15 bytes
	ldy #233
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.endif
.else
; 7696 to 7734
.if(n = 7696 || n = 7703 || n = 7705 || n = 7711 || n = 7722 || n = 7724 || n = 7725 || (n > 7726 && n < 7730))
	; 15..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7697)
	; 17 bytes
	ldy #208
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7698 || n = 7700)
	; 17..18 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7699 || n = 7732 || n = 7733)
	; 17..18 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7701)
	; 16 bytes
	ldy #220
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7702)
	; 19 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7704 || n = 7712 || n = 7726)
	; 17..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7706)
	; 19 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7707)
	; 18 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7708 || n = 7710)
	; 18..19 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 7709)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7713 || n = 7715)
	; 18..19 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7714)
	; 19 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7716)
	; 19 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7717)
	; 19 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 7718)
	; 18 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 7719)
	; 19 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 7720)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7721)
	; 14 bytes
	ldy #234 ;hides 'nop'
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7723)
	; 15 bytes
	ldy #234
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7730 || n = 7731)
	; 18 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7734)
	; 17 bytes
	ldy #209
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.endif
.endif
.else
.if((n >= 7735) && (n < 7774))
; 7735 to 7773
.if(n = 7735 || n = 7766)
	; 19 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7736)
	; 16 bytes
	ldy #221
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7737 || n = 7739 || n = 7754 || n = 7759 || n = 7765)
	; 16..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7738 || n = 7740 || n = 7752 || n = 7753 || n = 7755 || n = 7757 || n = 7758 || n = 7760 || n = 7764 || n = 7773)
	; 15..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7741)
	; 18 bytes
	ldy #180
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 7742)
	; 18 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 7743 || n = 7745)
	; 18..19 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7744)
	; 19 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7746)
	; 19 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7747)
	; 19 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7748)
	; 18 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 7749)
	; 19 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7750)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7751)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7756)
	; 15 bytes
	ldy #235
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7761)
	; 18 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7762)
	; 17 bytes
	ldy #199
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7763)
	; 18 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7767)
	; 19 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7768)
	; 18 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7769 || n = 7770)
	; 18 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7771)
	; 16 bytes
	ldy #222
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7772)
	; 19 bytes
	delay_n 21
	delay_n (n-21)
.endif
.else
; 7774 to 7812
.if(n = 7774 || n = 7785 || n = 7792 || n = 7802 || n = 7804 || n = 7809 || n = 7811)
	; 16..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7775 || n = 7784 || (n > 7785 && n < 7789) || n = 7791 || n = 7793 || n = 7795 || n = 7803 || n = 7805 || n = 7810 || n = 7812)
	; 15..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 7776 || n = 7807)
	; 19 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7777)
	; 19 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7778 || n = 7780)
	; 18..19 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7779)
	; 19 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 7781)
	; 19 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 7782)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7783)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7789)
	; 15 bytes
	ldy #236
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7790)
	; 18 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7794)
	; 18 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7796)
	; 19 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 7797)
	; 19 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7798)
	; 18 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7799)
	; 17 bytes
	ldy #56 ;hides 'sec'
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	iny
	Jne *-14
.elseif(n = 7800)
	; 18 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7801)
	; 17 bytes
	ldy #200
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7806)
	; 16 bytes
	ldy #223
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7808)
	; 17 bytes
	ldy #211
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.endif
.endif
.endif
.endif
.else
.if((n >= 7813) && (n < 7969))
.if((n >= 7813) && (n < 7891))
.if((n >= 7813) && (n < 7852))
; 7813 to 7851
.if(n = 7813 || n = 7844)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7814)
	; 19 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 7815 || (n > 7816 && n < 7821) || n = 7824 || n = 7826 || n = 7828 || n = 7834 || n = 7842 || n = 7843 || n = 7846 || (n > 7847 && n < 7852))
	; 15..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7816 || n = 7825 || n = 7835 || n = 7847)
	; 16..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7821)
	; 18 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7822)
	; 15 bytes
	ldy #237
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7823)
	; 18 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7827)
	; 18 bytes
	ldy #182
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 7829)
	; 19 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7830)
	; 19 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7831)
	; 18 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7832)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7833)
	; 18 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 7836)
	; 19 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 7837)
	; 19 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7838)
	; 18 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7839)
	; 19 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7840)
	; 17 bytes
	ldy #201
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7841)
	; 16 bytes
	ldy #224
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7845)
	; 17 bytes
	ldy #212
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.endif
.else
.if((n >= 7852) && (n < 7871))
; 7852 to 7870
.if(n = 7852)
	; 18 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7853)
	; 18 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 7854)
	; 18 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7855)
	; 15 bytes
	ldy #238
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7856)
	; 18 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7857 || (n > 7858 && n < 7862))
	; 16..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7858)
	; 17 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7862)
	; 18 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7863)
	; 18 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 7864)
	; 19 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7865)
	; 19 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 7866)
	; 19 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 7867)
	; 19 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 7868)
	; 18 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 7869)
	; 19 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7870)
	; 18 bytes
	ldy #183
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.endif
.else
; 7871 to 7890
.if(n = 7871)
	; 19 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7872)
	; 19 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 7873)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7874)
	; 19 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 7875)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7876)
	; 16 bytes
	ldy #225
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7877 || (n > 7878 && n < 7882) || n = 7884 || n = 7890)
	; 15..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7878)
	; 16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7882)
	; 17 bytes
	ldy #213
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7883)
	; 18 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7885)
	; 18 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7886)
	; 18 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7887)
	; 18 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7888)
	; 15 bytes
	ldy #239
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7889)
	; 19 bytes
	delay_n 76
	delay_n (n-76)
.endif
.endif
.endif
.else
.if((n >= 7891) && (n < 7930))
; 7891 to 7929
.if(n = 7891 || n = 7909 || n = 7922 || n = 7924)
	; 16..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 7891 && n < 7895) || n = 7908 || n = 7910 || n = 7912 || n = 7913 || n = 7915 || n = 7920 || n = 7923 || (n > 7924 && n < 7928))
	; 15..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7895)
	; 19 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7896)
	; 19 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 7897 || n = 7928)
	; 19 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 7898 || n = 7916)
	; 18..19 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7899)
	; 19 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7900)
	; 19 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7901)
	; 19 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7902)
	; 19 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7903)
	; 19 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 7904)
	; 19 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7905)
	; 19 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 7906)
	; 14 bytes
	ldy #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7907)
	; 19 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 7911)
	; 16 bytes
	ldy #226
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7914)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7917)
	; 18 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7918)
	; 17 bytes
	ldy #203
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7919)
	; 17 bytes
	ldy #214
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7921)
	; 15 bytes
	ldy #240
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7929)
	; 19 bytes
	delay_n 116
	delay_n (n-116)
.endif
.else
; 7930 to 7968
.if(n = 7930)
	; 19 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7931 || n = 7947 || n = 7964)
	; 18..19 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7932 || n = 7963)
	; 19 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7933)
	; 19 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7934 || n = 7965)
	; 19 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 7935 || n = 7966)
	; 19 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7936 || n = 7967)
	; 19 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 7937)
	; 14 bytes
	ldy #0
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-10
.elseif(n = 7938)
	; 19 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 7939 || (n > 7940 && n < 7945) || n = 7948 || n = 7950 || n = 7956 || (n > 7957 && n < 7962))
	; 15..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 7940)
	; 16 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7945)
	; 18 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7946)
	; 16 bytes
	ldy #227
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7949)
	; 18 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7951)
	; 19 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 7952)
	; 19 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7953)
	; 19 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7954)
	; 15 bytes
	ldy #241
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7955)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7957)
	; 17 bytes
	ldy #204
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7962)
	; 19 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7968)
	; 19 bytes
	delay_n 31
	delay_n (n-31)
.endif
.endif
.endif
.else
.if((n >= 7969) && (n < 8047))
.if((n >= 7969) && (n < 8008))
; 7969 to 8007
.if(n = 7969)
	; 19 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 7970 || n = 8001)
	; 19 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 7971 || n = 8002)
	; 19 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 7972 || n = 8003)
	; 19 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 7973)
	; 19 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 7974 || n = 8005)
	; 19 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 7975 || n = 8006)
	; 19 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 7976 || n = 8007)
	; 19 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 7977)
	; 19 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 7978)
	; 19 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 7979)
	; 19 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 7980)
	; 19 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 7981)
	; 16 bytes
	ldy #228
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 7982)
	; 19 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 7983 || n = 7985 || n = 7989 || n = 7992 || n = 7995 || n = 7997 || n = 7998 || n = 8000)
	; 16..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 7984 || n = 7990 || n = 7994)
	; 17..18 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 7986)
	; 18 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 7987)
	; 15 bytes
	ldy #242
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 7988)
	; 19 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7991)
	; 16 bytes
	ldy #216 ;hides 'cld'
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7993)
	; 17 bytes
	ldy #216
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7996)
	; 17 bytes
	ldy #205
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 7999)
	; 18 bytes
	ldy #186
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8004)
	; 19 bytes
	delay_n 191
	delay_n (n-191)
.endif
.else
; 8008 to 8046
.if(n = 8008)
	; 19 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 8009)
	; 19 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 8010 || n = 8041)
	; 19 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 8011)
	; 19 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 8012 || n = 8043)
	; 19 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8013)
	; 19 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 8014 || n = 8018 || n = 8022 || (n > 8023 && n < 8027) || n = 8032 || n = 8034 || n = 8039 || n = 8044)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8015 || n = 8046)
	; 19 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 8016)
	; 16 bytes
	ldy #229
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8017)
	; 19 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 8019 || n = 8023 || n = 8033 || n = 8038 || n = 8040 || n = 8045)
	; 17..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8020)
	; 15 bytes
	ldy #243
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 8021)
	; 19 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 8027)
	; 19 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 8028)
	; 19 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8029)
	; 20 bytes
	delay_n 92
	delay_n (n-92)
.elseif(n = 8030)
	; 17 bytes
	ldy #217
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8031)
	; 19 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 8035)
	; 17 bytes
	ldy #206
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8036)
	; 19 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 8037)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8042)
	; 18 bytes
	ldy #187
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.endif
.endif
.else
.if((n >= 8047) && (n < 8086))
; 8047 to 8085
.if(n = 8047)
	; 19 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8048)
	; 19 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 8049)
	; 19 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 8050 || n = 8055 || (n > 8056 && n < 8061) || n = 8069 || n = 8071 || n = 8075 || n = 8076 || n = 8079 || n = 8080)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8051)
	; 16 bytes
	ldy #230
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8052 || n = 8083)
	; 19 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 8053)
	; 15 bytes
	ldy #244
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 8054 || n = 8056 || n = 8070 || n = 8077 || n = 8081)
	; 17..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8061)
	; 19 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 8062)
	; 19 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 8063)
	; 19 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 8064)
	; 19 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8065)
	; 19 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8066)
	; 19 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 8067)
	; 17 bytes
	ldy #218
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8068)
	; 19 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 8072)
	; 19 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 8073)
	; 19 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 8074)
	; 17 bytes
	ldy #207
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8078)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8082)
	; 19 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 8084)
	; 20 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8085)
	; 18 bytes
	ldy #188
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.endif
.else
; 8086 to 8125
.if(n = 8086)
	; 15 bytes
	ldy #245
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 8087 || n = 8118)
	; 19 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 8088 || (n > 8089 && n < 8093) || n = 8095 || n = 8100 || n = 8105 || n = 8106 || (n > 8107 && n < 8111) || n = 8115 || n = 8117 || n = 8120 || n = 8124 || n = 8125)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8089 || n = 8107 || n = 8116 || n = 8122)
	; 17..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8093)
	; 19 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 8094)
	; 19 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8096)
	; 19 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8097)
	; 19 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 8098)
	; 19 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 8099)
	; 20 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 8101)
	; 19 bytes
	ldy #180
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 8102)
	; 19 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 8103)
	; 19 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 8104)
	; 17 bytes
	ldy #219
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8111)
	; 19 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 8112)
	; 20 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8113)
	; 17 bytes
	ldy #208
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8114)
	; 20 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 8119)
	; 15 bytes
	ldy #246
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 8121)
	; 16 bytes
	ldy #232
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8123)
	; 17 bytes
	delay_n (n-4)
	delay_n 4
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 8126) && (n < 8438))
.if((n >= 8126) && (n < 8282))
.if((n >= 8126) && (n < 8204))
.if((n >= 8126) && (n < 8165))
.if((n >= 8126) && (n < 8145))
; 8126 to 8144
.if(n = 8126 || n = 8130 || n = 8143)
	; 18..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 8127)
	; 19 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8128)
	; 18 bytes
	ldy #189
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8129)
	; 19 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8131)
	; 19 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8132)
	; 20 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8133)
	; 19 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 8134)
	; 20 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 8135)
	; 19 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 8136)
	; 20 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 8137)
	; 20 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 8138)
	; 20 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8139)
	; 20 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 8140)
	; 20 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 8141)
	; 17 bytes
	ldy #220
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8142)
	; 20 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 8144)
	; 19 bytes
	delay_n (n-3)
	ldy $A4
.endif
.else
; 8145 to 8164
.if(n = 8145 || n = 8147 || n = 8154 || n = 8157 || n = 8158 || n = 8161 || n = 8162 || n = 8164)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8146)
	; 19 bytes
	ldy #181
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 8148)
	; 20 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 8149)
	; 20 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8150)
	; 20 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8151)
	; 20 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 8152)
	; 15 bytes
	ldy #247
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 8153)
	; 20 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 8155 || n = 8159 || n = 8163)
	; 17..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8156)
	; 16 bytes
	ldy #233
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8160)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.endif
.endif
.else
; 8165 to 8203
.if(n = 8165 || n = 8173 || n = 8180 || n = 8182 || n = 8184 || n = 8187 || n = 8190 || n = 8193 || n = 8195 || n = 8196 || n = 8203)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8166)
	; 20 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8167)
	; 20 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8168)
	; 19 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 8169)
	; 20 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 8170)
	; 20 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 8171)
	; 18 bytes
	ldy #190
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8172)
	; 20 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 8174 || n = 8181 || n = 8186 || n = 8188 || n = 8192 || n = 8194 || n = 8202)
	; 16..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8175)
	; 20 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 8176)
	; 20 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 8177)
	; 20 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8178)
	; 17 bytes
	ldy #221
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8179)
	; 20 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 8183)
	; 14 bytes
	ldy #248 ;hides 'sed'
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 8185)
	; 15 bytes
	ldy #248
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 8189)
	; 15 bytes
	ldy #234 ;hides 'nop'
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8191)
	; 16 bytes
	ldy #234
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8197)
	; 19 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8198)
	; 20 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8199)
	; 17 bytes
	ldy #56 ;hides 'sec'
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-14
.elseif(n = 8200)
	; 19 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8201)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.endif
.endif
.else
.if((n >= 8204) && (n < 8243))
; 8204 to 8242
.if(n = 8204 || n = 8221 || n = 8229 || n = 8233)
	; 17..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8205 || n = 8206 || n = 8216 || n = 8217 || n = 8219 || n = 8220 || (n > 8221 && n < 8226) || n = 8228 || n = 8231 || n = 8232 || n = 8234 || n = 8235)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8207 || n = 8240)
	; 20 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 8208 || n = 8239 || n = 8241)
	; 19..20 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 8209 || n = 8211)
	; 19..20 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8210)
	; 20 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8212)
	; 20 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 8213)
	; 20 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 8214)
	; 18 bytes
	ldy #191
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8215)
	; 17 bytes
	ldy #222
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8218)
	; 15 bytes
	ldy #249
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 8226)
	; 16 bytes
	ldy #235
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8227)
	; 20 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8230)
	; 17 bytes
	ldy #211
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8236)
	; 19 bytes
	ldy #183
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 8237)
	; 20 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8238)
	; 19 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 8242)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.endif
.else
; 8243 to 8281
.if(n = 8243 || n = 8274 || n = 8276)
	; 19..20 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8244 || n = 8246 || n = 8247 || n = 8253 || n = 8255 || n = 8256 || n = 8258 || n = 8259 || n = 8263 || n = 8265 || n = 8266 || n = 8271 || n = 8273 || n = 8275)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8245 || n = 8254 || n = 8260 || n = 8264 || n = 8272)
	; 17..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8248 || n = 8279)
	; 19..20 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8249)
	; 19 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 8250)
	; 20 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 8251)
	; 15 bytes
	ldy #250
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 8252)
	; 17 bytes
	ldy #223
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8257)
	; 18 bytes
	ldy #192
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8261)
	; 16 bytes
	ldy #236
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8262)
	; 19 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8267)
	; 19 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 8268)
	; 20 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 8269)
	; 17 bytes
	ldy #212
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8270)
	; 20 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8277)
	; 20 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8278)
	; 20 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 8280)
	; 20 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 8281)
	; 19 bytes
	ldy #184
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.endif
.endif
.endif
.else
.if((n >= 8282) && (n < 8360))
.if((n >= 8282) && (n < 8321))
; 8282 to 8320
.if(n = 8282 || n = 8315)
	; 20 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8283)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8284)
	; 15 bytes
	ldy #251
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 8285 || n = 8286 || n = 8288 || n = 8290 || n = 8291 || n = 8293 || n = 8298 || n = 8301 || n = 8302 || n = 8310 || n = 8312 || n = 8313 || n = 8319)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8287 || n = 8292 || n = 8299 || n = 8303 || n = 8311 || n = 8320)
	; 17..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8289)
	; 17 bytes
	ldy #224
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8294 || n = 8306)
	; 19..20 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8295)
	; 19 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8296)
	; 16 bytes
	ldy #237
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8297)
	; 20 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8300)
	; 18 bytes
	ldy #193
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8304)
	; 19 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 8305)
	; 20 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 8307)
	; 20 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 8308)
	; 17 bytes
	ldy #213
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8309)
	; 19 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 8314 || n = 8316)
	; 19..20 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 8317)
	; 15 bytes
	ldy #252
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 8318)
	; 20 bytes
	delay_n 166
	delay_n (n-166)
.endif
.else
; 8321 to 8359
.if((n > 8320 && n < 8324) || n = 8328 || n = 8330 || n = 8332 || n = 8333 || (n > 8334 && n < 8338) || n = 8345 || n = 8349 || n = 8351 || n = 8352 || (n > 8353 && n < 8358))
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8324)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8325 || n = 8358)
	; 19 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8326)
	; 17 bytes
	ldy #225
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8327 || n = 8329 || n = 8334 || n = 8346 || n = 8353)
	; 17..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8331)
	; 16 bytes
	ldy #238
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8338)
	; 20 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 8339)
	; 19 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 8340)
	; 20 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 8341)
	; 20 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8342)
	; 20 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8343)
	; 18 bytes
	ldy #194
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8344)
	; 19 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 8347)
	; 17 bytes
	ldy #214
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8348)
	; 20 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8350)
	; 15 bytes
	ldy #253
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 8359)
	; 19 bytes
	delay_n 176
	delay_n (n-176)
.endif
.endif
.else
.if((n >= 8360) && (n < 8399))
.if((n >= 8360) && (n < 8379))
; 8360 to 8378
.if(n = 8360)
	; 19 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8361)
	; 19 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8362)
	; 19 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8363)
	; 17 bytes
	ldy #226
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8364)
	; 19 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 8365)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8366)
	; 16 bytes
	ldy #239
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8367 || n = 8368 || n = 8370 || n = 8372)
	; 17..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 8369)
	; 18 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8371)
	; 19 bytes
	ldy #186
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 8373)
	; 20 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 8374)
	; 19 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 8375)
	; 20 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8376)
	; 20 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8377)
	; 20 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 8378)
	; 20 bytes
	delay_n 61
	delay_n (n-61)
.endif
.else
; 8379 to 8398
.if(n = 8379)
	; 19 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 8380)
	; 20 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8381)
	; 20 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8382)
	; 20 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 8383)
	; 15 bytes
	ldy #254
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 8384)
	; 20 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 8385 || (n > 8386 && n < 8391) || n = 8392)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8386)
	; 17 bytes
	ldy #215
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8391)
	; 19 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8393)
	; 19 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8394)
	; 19 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8395)
	; 19 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8396)
	; 20 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8397)
	; 20 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 8398)
	; 20 bytes
	delay_n 81
	delay_n (n-81)
.endif
.endif
.else
; 8399 to 8437
.if(n = 8399 || n = 8432)
	; 19 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 8400)
	; 17 bytes
	ldy #227
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8401)
	; 16 bytes
	ldy #240
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8402 || n = 8403 || n = 8405 || n = 8407 || n = 8408 || n = 8410 || n = 8411 || n = 8418 || (n > 8419 && n < 8423) || n = 8427 || n = 8430 || n = 8431)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8404 || n = 8409 || n = 8419 || n = 8428)
	; 17..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8406)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8412)
	; 20 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 8413)
	; 20 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8414)
	; 20 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8415)
	; 20 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 8416)
	; 15 bytes
	ldy #255
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 8417)
	; 20 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 8423)
	; 17 bytes
	ldy #216 ;hides 'cld'
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8424)
	; 19 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8425)
	; 17 bytes
	ldy #216
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8426)
	; 19 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8429)
	; 18 bytes
	ldy #196
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8433)
	; 20 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 8434)
	; 20 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 8435)
	; 20 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8436)
	; 16 bytes
	ldy #241
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8437)
	; 17 bytes
	ldy #228
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.endif
.endif
.endif
.endif
.else
.if((n >= 8438) && (n < 8594))
.if((n >= 8438) && (n < 8516))
.if((n >= 8438) && (n < 8477))
; 8438 to 8476
.if((n > 8437 && n < 8444) || n = 8451 || (n > 8452 && n < 8457) || n = 8466 || n = 8468 || n = 8469 || n = 8473 || n = 8475 || n = 8476)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8444)
	; 20 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 8445)
	; 20 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 8446 || n = 8459)
	; 19..20 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8447)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8448)
	; 20 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 8449)
	; 15 bytes
	ldy #0
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-11
.elseif(n = 8450 || n = 8452 || n = 8467)
	; 17..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8457)
	; 19 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8458)
	; 20 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8460)
	; 19 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8461)
	; 19 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8462)
	; 20 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8463)
	; 20 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 8464)
	; 17 bytes
	ldy #217
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8465)
	; 19 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 8470)
	; 20 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 8471)
	; 16 bytes
	ldy #242
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8472)
	; 18 bytes
	ldy #197
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8474)
	; 17 bytes
	ldy #229
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.endif
.else
; 8477 to 8515
.if(n = 8477 || n = 8491 || n = 8509 || n = 8514)
	; 18..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 8477 && n < 8481) || n = 8490 || n = 8492 || n = 8493 || n = 8505 || n = 8507 || n = 8508 || n = 8510 || n = 8512 || n = 8513)
	; 17..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8481)
	; 20 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8482)
	; 20 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 8483)
	; 20 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 8484)
	; 20 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 8485)
	; 20 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8486)
	; 20 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 8487)
	; 20 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 8488)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8489)
	; 20 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8494)
	; 20 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 8495)
	; 20 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8496)
	; 20 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 8497)
	; 20 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8498)
	; 19 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 8499)
	; 20 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 8500)
	; 20 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 8501)
	; 20 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8502)
	; 20 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 8503)
	; 17 bytes
	ldy #218
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8504)
	; 20 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 8506)
	; 16 bytes
	ldy #243
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8511)
	; 17 bytes
	ldy #230
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8515)
	; 18 bytes
	ldy #198
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.endif
.endif
.else
.if((n >= 8516) && (n < 8555))
; 8516 to 8554
.if(n = 8516 || n = 8517 || n = 8528 || n = 8531 || n = 8533 || n = 8534 || n = 8538 || (n > 8542 && n < 8548) || n = 8550 || (n > 8551 && n < 8555))
	; 17..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8518 || n = 8532 || n = 8551)
	; 19..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8519)
	; 20 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 8520)
	; 20 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 8521)
	; 20 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 8522)
	; 20 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8523)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8524)
	; 20 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8525)
	; 20 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 8526)
	; 20 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 8527)
	; 20 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 8529)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8530)
	; 20 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8535)
	; 20 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 8536)
	; 20 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 8537)
	; 20 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 8539)
	; 20 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 8540)
	; 20 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8541)
	; 16 bytes
	ldy #244
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8542)
	; 17 bytes
	ldy #219
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8548)
	; 17 bytes
	ldy #231
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8549)
	; 20 bytes
	delay_n 166
	delay_n (n-166)
.endif
.else
; 8555 to 8593
.if(n = 8555)
	; 20 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8556)
	; 18 bytes
	ldy #200 ;hides 'iny'
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	dey
	Jne *-15
.elseif(n = 8557)
	; 20 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8558)
	; 18 bytes
	ldy #199
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8559 || n = 8592)
	; 20 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 8560 || n = 8563 || n = 8566 || n = 8571 || n = 8572 || n = 8574 || n = 8575 || n = 8578 || n = 8580 || n = 8582 || n = 8587 || (n > 8588 && n < 8592))
	; 17..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8561 || n = 8573 || n = 8579 || n = 8584 || n = 8586 || n = 8588)
	; 18..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8562)
	; 20 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 8564)
	; 20 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 8565)
	; 20 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 8567)
	; 20 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8568)
	; 21 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 8569)
	; 20 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 8570)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8576)
	; 16 bytes
	ldy #245
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8577)
	; 20 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 8581)
	; 17 bytes
	ldy #220
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8583)
	; 16 bytes
	ldy #24 ;hides 'clc'
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-13
.elseif(n = 8585)
	; 17 bytes
	ldy #232
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8593)
	; 20 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.endif
.else
.if((n >= 8594) && (n < 8672))
.if((n >= 8594) && (n < 8633))
; 8594 to 8632
.if(n = 8594 || n = 8631)
	; 20..21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8595)
	; 20 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 8596)
	; 19 bytes
	ldy #191
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 8597 || n = 8630)
	; 20 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 8598 || n = 8603 || n = 8606 || n = 8613 || (n > 8614 && n < 8619) || n = 8624 || (n > 8625 && n < 8630))
	; 17..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8599)
	; 17 bytes
	ldy #56 ;hides 'sec'
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-14
.elseif(n = 8600)
	; 20 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8601)
	; 18 bytes
	ldy #200
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8602 || n = 8604 || n = 8614 || n = 8623 || n = 8625)
	; 18..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8605)
	; 20 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 8607)
	; 20 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 8608)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 8609 || n = 8621)
	; 20..21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8610)
	; 20 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 8611)
	; 16 bytes
	ldy #246
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8612)
	; 20 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 8619)
	; 20 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8620)
	; 17 bytes
	ldy #221
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8622)
	; 17 bytes
	ldy #233
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8632)
	; 20 bytes
	delay_n 49
	delay_n (n-49)
.endif
.else
; 8633 to 8671
.if(n = 8633 || n = 8668 || n = 8670)
	; 20..21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8634 || n = 8669 || n = 8671)
	; 20..21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8635)
	; 20 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 8636)
	; 22 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 8637 || n = 8642 || n = 8643 || n = 8648 || n = 8650 || n = 8651 || n = 8653 || n = 8654 || n = 8656 || n = 8658 || n = 8661 || (n > 8662 && n < 8667))
	; 17..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8638)
	; 21 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 8639)
	; 21 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 8640)
	; 20 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 8641)
	; 19 bytes
	ldy #192
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 8644)
	; 18 bytes
	ldy #201
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8645)
	; 20 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 8646)
	; 16 bytes
	ldy #247
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8647 || n = 8649 || n = 8655 || n = 8660 || n = 8662)
	; 18..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8652)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8657)
	; 16 bytes
	ldy #234 ;hides 'nop'
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8659)
	; 17 bytes
	ldy #234
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8667)
	; 20 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 8672) && (n < 8711))
; 8672 to 8710
.if(n = 8672 || n = 8705)
	; 20..21 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8673 || n = 8675)
	; 21..22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8674)
	; 21 bytes
	ldy #177
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 8676)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 8677 || n = 8710)
	; 20..21 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8678)
	; 21 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 8679)
	; 15 bytes
	ldy #248 ;hides 'sed'
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8680)
	; 21 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 8681)
	; 16 bytes
	ldy #248
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8682 || n = 8684 || n = 8699 || n = 8701)
	; 17..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8683 || n = 8685 || n = 8688 || n = 8689 || n = 8695 || n = 8697 || n = 8700 || (n > 8701 && n < 8705))
	; 17..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8686)
	; 19 bytes
	ldy #193
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 8687)
	; 18 bytes
	ldy #202
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8690 || n = 8692 || n = 8707 || n = 8709)
	; 19..21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8691)
	; 19 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8693)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8694)
	; 21 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 8696)
	; 17 bytes
	ldy #235
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8698)
	; 17 bytes
	ldy #223
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8706)
	; 20 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 8708)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 8711 to 8750
.if(n = 8711)
	; 22 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 8712)
	; 21 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8713)
	; 20 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 8714)
	; 21 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 8715 || n = 8717)
	; 20..21 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8716)
	; 16 bytes
	ldy #249
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8718 || (n > 8719 && n < 8724) || n = 8732 || n = 8735 || n = 8736 || n = 8738 || n = 8739 || (n > 8740 && n < 8744))
	; 17..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8719 || n = 8740)
	; 18..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8724)
	; 20 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8725)
	; 20 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8726 || n = 8744 || n = 8747)
	; 20..21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8727)
	; 20 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8728)
	; 19 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 8729)
	; 21 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 8730)
	; 18 bytes
	ldy #203
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8731)
	; 19 bytes
	ldy #194
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 8733)
	; 17 bytes
	ldy #236
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8734)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8737)
	; 17 bytes
	ldy #224
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8745)
	; 20 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 8746)
	; 21 bytes
	ldy #165
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 8748)
	; 20 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 8749)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8750)
	; 20 bytes
	delay_n 71
	delay_n (n-71)
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 8751) && (n < 9376))
.if((n >= 8751) && (n < 9063))
.if((n >= 8751) && (n < 8907))
.if((n >= 8751) && (n < 8829))
.if((n >= 8751) && (n < 8790))
; 8751 to 8789
.if(n = 8751)
	; 16 bytes
	ldy #250
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8752)
	; 21 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8753 || (n > 8754 && n < 8759) || n = 8772 || n = 8774 || n = 8777 || n = 8778 || (n > 8779 && n < 8784) || n = 8788)
	; 17..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8754 || n = 8779 || n = 8789)
	; 18..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8759 || n = 8784)
	; 20..21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8760)
	; 20 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8761)
	; 20 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8762)
	; 20 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8763 || n = 8787)
	; 20..21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8764 || n = 8766)
	; 21..22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 8765)
	; 20 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 8767)
	; 21 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8768)
	; 21 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 8769)
	; 21 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 8770)
	; 17 bytes
	ldy #237
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8771)
	; 21 bytes
	delay_n 92
	delay_n (n-92)
.elseif(n = 8773)
	; 18 bytes
	ldy #204
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8775)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8776)
	; 17 bytes
	ldy #225
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8785)
	; 20 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8786)
	; 16 bytes
	ldy #251
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.endif
.else
; 8790 to 8828
.if((n > 8789 && n < 8794) || n = 8809 || (n > 8810 && n < 8815) || n = 8817 || n = 8818 || n = 8820 || n = 8822 || n = 8823 || (n > 8824 && n < 8829))
	; 17..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8794)
	; 20 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8795)
	; 20 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 8796)
	; 20 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8797)
	; 20 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8798)
	; 20 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8799)
	; 21 bytes
	ldy #166
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 8800)
	; 20 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 8801)
	; 22 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 8802)
	; 21 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8803)
	; 21 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 8804)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 8805)
	; 20 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 8806)
	; 21 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 8807)
	; 17 bytes
	ldy #238
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8808)
	; 21 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8810 || n = 8819 || n = 8824)
	; 18..19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8815)
	; 17 bytes
	ldy #226
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8816)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8821)
	; 16 bytes
	ldy #252
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.endif
.endif
.else
.if((n >= 8829) && (n < 8868))
.if((n >= 8829) && (n < 8848))
; 8829 to 8847
.if(n = 8829)
	; 20 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8830)
	; 20 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8831)
	; 20 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8832)
	; 20 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8833)
	; 20 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8834)
	; 21 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 8835)
	; 20 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 8836)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 8837)
	; 20 bytes
	ldy #188
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 8838)
	; 21 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 8839 || n = 8846)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8840)
	; 20 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 8841)
	; 21 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 8842)
	; 21 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 8843)
	; 21 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 8844)
	; 17 bytes
	ldy #239
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8845)
	; 20 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 8847)
	; 19 bytes
	delay_n (n-3)
	ldy $A4
.endif
.else
; 8848 to 8867
.if((n > 8847 && n < 8852) || n = 8858 || n = 8860 || n = 8862 || n = 8863)
	; 17..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8852)
	; 21 bytes
	ldy #167
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 8853)
	; 20 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 8854)
	; 17 bytes
	ldy #227
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8855)
	; 17 bytes
	ldy #216 ;hides 'cld'
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8856)
	; 16 bytes
	ldy #253
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8857)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8859)
	; 18 bytes
	ldy #206
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8861)
	; 19 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 8864)
	; 20 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8865)
	; 20 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 8866)
	; 19 bytes
	ldy #197
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 8867)
	; 20 bytes
	delay_n 11
	delay_n (n-11)
.endif
.endif
.else
; 8868 to 8906
.if(n = 8868 || n = 8883 || (n > 8884 && n < 8889) || n = 8895 || n = 8897 || n = 8899 || n = 8900 || n = 8903 || n = 8904 || n = 8906)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8869)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8870)
	; 20 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 8871)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 8872)
	; 21 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8873)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 8874)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8875)
	; 20 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 8876)
	; 21 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 8877)
	; 21 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 8878)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 8879)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 8880)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8881)
	; 17 bytes
	ldy #240
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8882)
	; 21 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8884 || n = 8894 || n = 8896 || n = 8901 || n = 8905)
	; 18..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8889)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8890)
	; 21 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 8891)
	; 16 bytes
	ldy #254
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8892)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8893)
	; 17 bytes
	ldy #228
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8898)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8902)
	; 18 bytes
	ldy #207
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.endif
.endif
.endif
.else
.if((n >= 8907) && (n < 8985))
; 8907 to 8984
.if(n = 8907 || n = 8913 || n = 8915 || n = 8920 || (n > 8921 && n < 8926) || n = 8928 || n = 8930 || n = 8931 || n = 8933 || n = 8934 || (n > 8935 && n < 8939) || n = 8941 || n = 8943 || n = 8944 || n = 8946 || n = 8947 || n = 8949 || n = 8950 || n = 8957 || n = 8959 || n = 8960 || n = 8962 || n = 8963 || (n > 8964 && n < 8969) || n = 8970 || n = 8973 || (n > 8974 && n < 8978) || n = 8982 || n = 8984)
	; 17..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8908 || n = 8981)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8909 || n = 8972)
	; 20..21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8910 || n = 8951)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8911)
	; 19 bytes
	ldy #198
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 8912)
	; 21 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 8914 || n = 8921 || n = 8929 || n = 8935 || n = 8942 || n = 8948 || n = 8958 || n = 8964 || n = 8974 || n = 8983)
	; 18..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8916)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 8917 || n = 8952)
	; 21 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8918)
	; 17 bytes
	ldy #241
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8919)
	; 21 bytes
	ldy #182
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 8926)
	; 16 bytes
	ldy #255
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8927)
	; 21 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8932)
	; 17 bytes
	ldy #229
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8939 || n = 8980)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8940)
	; 20 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 8945)
	; 18 bytes
	ldy #208
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8953)
	; 21 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 8954)
	; 18 bytes
	ldy #200 ;hides 'iny'
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	dey
	Jne *-15
.elseif(n = 8955)
	; 17 bytes
	ldy #242
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8956)
	; 19 bytes
	ldy #199
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 8961)
	; 16 bytes
	ldy #0
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-12
.elseif(n = 8969 || n = 8979)
	; 20..21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8971)
	; 17 bytes
	ldy #230
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8978)
	; 20 bytes
	ldy #191
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.endif
.else
.if((n >= 8985) && (n < 9024))
; 8985 to 9023
.if((n > 8984 && n < 8988) || n = 8990 || n = 8993 || n = 8994 || (n > 8995 && n < 8999) || n = 9003 || n = 9005 || n = 9012 || (n > 9013 && n < 9017) || n = 9023)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8988)
	; 18 bytes
	ldy #209
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 8989)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 8991 || n = 8995 || n = 9002 || n = 9013)
	; 19..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 8992)
	; 17 bytes
	ldy #243
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 8999)
	; 18 bytes
	ldy #56 ;hides 'sec'
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-15
.elseif(n = 9000 || n = 9018)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9001)
	; 19 bytes
	ldy #200
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 9004 || n = 9022)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9006)
	; 22 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 9007)
	; 21 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9008)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9009)
	; 21 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 9010)
	; 17 bytes
	ldy #231
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9011)
	; 21 bytes
	ldy #170
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9017)
	; 21 bytes
	ldy #184
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9019)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 9020)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9021)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.endif
.else
; 9024 to 9062
.if(n = 9024 || n = 9032 || n = 9050 || n = 9052)
	; 19 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 9024 && n < 9028) || n = 9031 || (n > 9032 && n < 9037) || n = 9048 || n = 9051 || (n > 9052 && n < 9057))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9028)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9029)
	; 17 bytes
	ldy #244
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9030)
	; 21 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 9037)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9038)
	; 22 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9039 || n = 9057 || n = 9059)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9040 || n = 9058 || n = 9060)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9041 || n = 9061)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9042)
	; 21 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 9043)
	; 22 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 9044)
	; 21 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 9045)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9046)
	; 19 bytes
	ldy #201
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 9047)
	; 17 bytes
	ldy #24 ;hides 'clc'
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	iny
	Jne *-14
.elseif(n = 9049)
	; 17 bytes
	ldy #232
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9062)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.endif
.endif
.endif
.endif
.else
.if((n >= 9063) && (n < 9219))
.if((n >= 9063) && (n < 9141))
.if((n >= 9063) && (n < 9102))
; 9063 to 9101
.if(n = 9063)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 9064 || n = 9067 || n = 9068 || (n > 9069 && n < 9074) || n = 9076 || n = 9078 || n = 9080 || n = 9090 || (n > 9091 && n < 9096))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9065 || n = 9069 || n = 9077 || n = 9091)
	; 19..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9066)
	; 17 bytes
	ldy #245
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9074)
	; 18 bytes
	ldy #211
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9075)
	; 22 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 9079)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9081 || n = 9087)
	; 21..22 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 9082)
	; 21 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 9083)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9084)
	; 22 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 9085)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 9086)
	; 22 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 9088)
	; 17 bytes
	ldy #233
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9089)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 9096)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9097)
	; 21 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 9098)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9099)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9100)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9101)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.endif
.else
; 9102 to 9140
.if(n = 9102)
	; 21 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9103)
	; 17 bytes
	ldy #246
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9104)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 9105 || (n > 9106 && n < 9111) || n = 9119 || (n > 9120 && n < 9124) || n = 9129 || (n > 9130 && n < 9135) || n = 9138)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9106 || n = 9120 || n = 9128 || n = 9130)
	; 19..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9111)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9112)
	; 21 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9113 || n = 9135 || n = 9137)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9114)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9115)
	; 21 bytes
	ldy #186
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9116)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 9117)
	; 18 bytes
	ldy #212
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9118)
	; 22 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 9124)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 9125)
	; 17 bytes
	ldy #234 ;hides 'nop'
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9126)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 9127)
	; 17 bytes
	ldy #234
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9136)
	; 19 bytes
	ldy #203
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 9139)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9140)
	; 17 bytes
	ldy #247
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.endif
.endif
.else
.if((n >= 9141) && (n < 9180))
; 9141 to 9179
.if(n = 9141)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 9142 || n = 9145 || n = 9146 || (n > 9147 && n < 9152) || n = 9162 || n = 9164 || n = 9165 || n = 9168 || (n > 9169 && n < 9174) || n = 9179)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9143 || n = 9147 || n = 9163 || n = 9169 || n = 9178)
	; 18..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9144)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9152 || n = 9156)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9153)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 9154 || n = 9176)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9155)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9157)
	; 21 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 9158)
	; 22 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 9159)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 9160)
	; 18 bytes
	ldy #213
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9161)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 9166)
	; 17 bytes
	ldy #235
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9167)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 9174)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9175)
	; 16 bytes
	ldy #248 ;hides 'sed'
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9177)
	; 17 bytes
	ldy #248
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.endif
.else
; 9180 to 9218
.if(n = 9180 || n = 9188 || n = 9206 || n = 9208 || n = 9217)
	; 19..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 9180 && n < 9185) || n = 9187 || (n > 9188 && n < 9193) || n = 9207 || (n > 9208 && n < 9213) || n = 9216 || n = 9218)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9185)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9186 || n = 9196)
	; 20..21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9193)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 9194)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9195 || n = 9215)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9197)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9198)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 9199)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9200)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 9201)
	; 21 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9202)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 9203)
	; 18 bytes
	ldy #214
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9204)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 9205)
	; 17 bytes
	ldy #236
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9213)
	; 20 bytes
	ldy #196
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 9214)
	; 17 bytes
	ldy #249
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.endif
.endif
.endif
.else
.if((n >= 9219) && (n < 9297))
.if((n >= 9219) && (n < 9258))
; 9219 to 9257
.if((n > 9218 && n < 9222) || n = 9228 || (n > 9229 && n < 9234) || n = 9246 || (n > 9247 && n < 9251) || n = 9253 || (n > 9254 && n < 9258))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9222 || n = 9252)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9223)
	; 21 bytes
	ldy #174
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9224 || n = 9234)
	; 20..21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 9225 || n = 9237)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9226)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9227)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 9229 || n = 9247 || n = 9254)
	; 19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9235)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 9236)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9238)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9239)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 9240)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9241 || n = 9243)
	; 21..22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 9242)
	; 22 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 9244)
	; 17 bytes
	ldy #237
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9245)
	; 22 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9251)
	; 17 bytes
	ldy #250
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.endif
.else
; 9258 to 9296
.if(n = 9258 || n = 9269 || (n > 9270 && n < 9275) || n = 9285 || n = 9290 || (n > 9291 && n < 9295))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9259 || n = 9296)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9260)
	; 20 bytes
	ldy #197
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 9261 || n = 9277)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9262)
	; 21 bytes
	ldy #189
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9263 || n = 9279 || n = 9295)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9264)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 9265)
	; 22 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 9266 || n = 9268)
	; 21..22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9267)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9270 || n = 9286 || n = 9291)
	; 19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9275)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 9276)
	; 21 bytes
	ldy #175
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9278)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9280)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 9281)
	; 21 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9282)
	; 22 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9283)
	; 17 bytes
	ldy #238
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9284)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 9287)
	; 17 bytes
	ldy #216 ;hides 'cld'
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9288)
	; 17 bytes
	ldy #251
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9289)
	; 18 bytes
	ldy #216
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.endif
.endif
.else
.if((n >= 9297) && (n < 9336))
; 9297 to 9335
.if(n = 9297 || n = 9298)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9299)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9300 || n = 9320)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9301 || n = 9303)
	; 21..22 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 9302)
	; 22 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 9304)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9305)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9306)
	; 21 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 9307)
	; 20 bytes
	ldy #198
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 9308)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9309 || n = 9310 || (n > 9311 && n < 9316) || n = 9318 || n = 9324 || n = 9326 || n = 9327 || (n > 9328 && n < 9332) || n = 9334)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9311 || n = 9319 || n = 9328 || n = 9335)
	; 19..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9316)
	; 19 bytes
	ldy #207
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 9317)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 9321 || n = 9323)
	; 21..22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 9322)
	; 17 bytes
	ldy #239
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9325)
	; 17 bytes
	ldy #252
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9332)
	; 18 bytes
	ldy #217
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9333)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.endif
.else
; 9336 to 9375
.if(n = 9336 || n = 9338 || n = 9351 || (n > 9352 && n < 9357) || n = 9363 || n = 9364 || (n > 9365 && n < 9370))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9337 || n = 9373 || n = 9374)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9339)
	; 22 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 9340)
	; 22 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 9341 || n = 9343)
	; 21..22 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 9342)
	; 21 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 9344)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 9345)
	; 22 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 9346 || n = 9348)
	; 21..22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 9347)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9349)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9350)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 9352 || n = 9365)
	; 19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9357)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 9358)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 9359 || n = 9371 || n = 9372)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9360)
	; 21 bytes
	ldy #191
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9361)
	; 17 bytes
	ldy #240
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9362)
	; 17 bytes
	ldy #253
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9370)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9375)
	; 18 bytes
	ldy #218
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 9376) && (n < 9688))
.if((n >= 9376) && (n < 9532))
.if((n >= 9376) && (n < 9454))
.if((n >= 9376) && (n < 9415))
; 9376 to 9414
.if(n = 9376 || n = 9413)
	; 22 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 9377 || (n > 9378 && n < 9382) || n = 9392 || (n > 9393 && n < 9398) || n = 9401 || n = 9402 || n = 9404 || n = 9405 || n = 9407 || n = 9408)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9378 || n = 9393 || n = 9403)
	; 19..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9382)
	; 21 bytes
	ldy #177
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9383)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 9384)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9385)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9386)
	; 22 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 9387)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 9388)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9389)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 9390)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9391)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 9398)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 9399)
	; 17 bytes
	ldy #254
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9400)
	; 17 bytes
	ldy #241
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9406)
	; 19 bytes
	ldy #209
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 9409 || n = 9410)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9411 || n = 9412)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9414)
	; 22 bytes
	delay_n 126
	delay_n (n-126)
.endif
.else
; 9415 to 9453
.if(n = 9415 || n = 9452)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9416)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9417)
	; 22 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 9418)
	; 18 bytes
	ldy #219
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9419)
	; 22 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 9420 || (n > 9421 && n < 9425) || n = 9433 || n = 9435 || n = 9437 || n = 9438 || n = 9440 || n = 9441 || (n > 9442 && n < 9446) || n = 9453)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9421 || n = 9434 || n = 9442)
	; 19..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9425)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9426)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 9427)
	; 22 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 9428)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 9429)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9430)
	; 22 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9431)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9432)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 9436)
	; 17 bytes
	ldy #255
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9439)
	; 17 bytes
	ldy #242
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9446 || n = 9449)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9447 || n = 9450)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9448)
	; 20 bytes
	ldy #201
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 9451)
	; 19 bytes
	ldy #210
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.endif
.endif
.else
.if((n >= 9454) && (n < 9493))
; 9454 to 9492
.if(n = 9454 || n = 9464 || n = 9476 || n = 9481)
	; 19..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9455 || n = 9463 || (n > 9464 && n < 9468) || n = 9474 || n = 9475 || n = 9477 || n = 9479 || n = 9480 || (n > 9481 && n < 9485))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9456)
	; 22 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 9457)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 9458)
	; 21 bytes
	ldy #193
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9459)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 9460)
	; 22 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 9461)
	; 18 bytes
	ldy #220
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9462)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9468)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9469)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 9470)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 9471)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 9472)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9473)
	; 17 bytes
	ldy #0
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9478)
	; 17 bytes
	ldy #243
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9485 || n = 9490)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9486)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9487)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9488)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9489)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9491)
	; 22 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 9492)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.endif
.else
; 9493 to 9531
.if(n = 9493 || n = 9530 || n = 9531)
	; 22 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 9494)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 9495)
	; 20 bytes
	ldy #202
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 9496)
	; 19 bytes
	ldy #211
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 9497 || n = 9498 || n = 9500 || n = 9506 || (n > 9507 && n < 9511) || n = 9515 || n = 9518 || n = 9519 || (n > 9520 && n < 9525))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9499 || n = 9507 || n = 9514 || n = 9516 || n = 9520)
	; 19..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9501)
	; 22 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 9502)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 9503)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9504)
	; 18 bytes
	ldy #221
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9505)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9511)
	; 17 bytes
	ldy #24 ;hides 'clc'
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-14
.elseif(n = 9512)
	; 22 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 9513)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9517)
	; 17 bytes
	ldy #244
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9525 || n = 9529)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9526)
	; 22 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 9527)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9528)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.endif
.endif
.endif
.else
.if((n >= 9532) && (n < 9610))
.if((n >= 9532) && (n < 9571))
; 9532 to 9570
.if(n = 9532 || n = 9569)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9533 || n = 9570)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 9534)
	; 22 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 9535 || n = 9536)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 9537)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 9538)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9539)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 9540)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9541)
	; 19 bytes
	ldy #212
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 9542)
	; 20 bytes
	ldy #203
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 9543 || n = 9545 || n = 9549 || (n > 9550 && n < 9554) || n = 9558 || (n > 9559 && n < 9564))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9544 || n = 9550 || n = 9557 || n = 9559)
	; 19..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9546)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 9547)
	; 18 bytes
	ldy #222
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9548)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 9554)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9555)
	; 22 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 9556)
	; 17 bytes
	ldy #245
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9564 || n = 9566)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9565 || n = 9567)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9568)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
.if((n >= 9571) && (n < 9590))
; 9571 to 9589
.if(n = 9571)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 9572)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 9573)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 9574)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 9575 || n = 9576)
	; 22 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 9577)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9578)
	; 22 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 9579)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9580 || n = 9581)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 9582)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 9583)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 9584)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 9585)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 9586)
	; 19 bytes
	ldy #213
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 9587)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9588)
	; 20 bytes
	delay_n (n-2)
	nop
.elseif(n = 9589)
	; 20 bytes
	ldy #204
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.endif
.else
; 9590 to 9609
.if(n = 9590)
	; 18 bytes
	ldy #223
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9591 || n = 9592 || n = 9594 || n = 9597 || (n > 9598 && n < 9603))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9593)
	; 17 bytes
	ldy #234 ;hides 'nop'
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9595)
	; 17 bytes
	ldy #246
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9596 || n = 9598)
	; 19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9603)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9604 || n = 9606)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9605)
	; 21 bytes
	ldy #196
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9607)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9608)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9609)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.endif
.endif
.endif
.else
.if((n >= 9610) && (n < 9649))
.if((n >= 9610) && (n < 9629))
; 9610 to 9628
.if(n = 9610)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 9611)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9612 || n = 9615)
	; 22 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 9613)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9614)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9616)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 9617 || n = 9620)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 9618)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 9619)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 9621)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9622 || n = 9625)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 9623)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9624)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9626)
	; 22 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9627)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 9628)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.endif
.else
; 9629 to 9648
.if(n = 9629)
	; 22 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 9630)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 9631)
	; 19 bytes
	ldy #214
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 9632)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 9633)
	; 18 bytes
	ldy #224
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9634)
	; 17 bytes
	ldy #247
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9635 || n = 9638 || (n > 9639 && n < 9644))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9636)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9637 || n = 9639)
	; 19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9644 || n = 9646)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9645)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9647)
	; 21 bytes
	ldy #182
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9648)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.else
.if((n >= 9649) && (n < 9668))
; 9649 to 9667
.if(n = 9649)
	; 22 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 9650)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9651)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 9652)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9653)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 9654)
	; 21 bytes
	ldy #197
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9655)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 9656)
	; 22 bytes
	delay_n (n-2)
	nop
.elseif(n = 9657)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 9658)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9659)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 9660)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9661)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 9662)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9663)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 9664)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 9665)
	; 22 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9666)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 9667)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.endif
.else
; 9668 to 9687
.if(n = 9668)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9669)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 9670)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9671)
	; 17 bytes
	ldy #248 ;hides 'sed'
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9672)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 9673)
	; 17 bytes
	ldy #248
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9674 || n = 9680)
	; 19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9675 || n = 9678 || n = 9679 || (n > 9680 && n < 9685))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9676)
	; 18 bytes
	ldy #225
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9677)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9685)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9686)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9687)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 9688) && (n < 9844))
.if((n >= 9688) && (n < 9766))
.if((n >= 9688) && (n < 9727))
.if((n >= 9688) && (n < 9707))
; 9688 to 9706
.if(n = 9688)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9689)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9690)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 9691)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9692)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 9693)
	; 22 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 9694)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 9695)
	; 22 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 9696)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 9697)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9698)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 9699)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9700)
	; 21 bytes
	ldy #183
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9701)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9702)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 9703)
	; 21 bytes
	ldy #198
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9704)
	; 22 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9705)
	; 22 bytes
	delay_n (n-2)
	nop
.elseif(n = 9706)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.endif
.else
; 9707 to 9726
.if(n = 9707)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9708)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 9709)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9710)
	; 22 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 9711)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 9712)
	; 17 bytes
	ldy #249
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9713)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 9714 || n = 9716 || n = 9717 || n = 9720 || (n > 9721 && n < 9726))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9715 || n = 9721)
	; 19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9718)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9719)
	; 18 bytes
	ldy #226
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9726)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.endif
.endif
.else
.if((n >= 9727) && (n < 9746))
; 9727 to 9745
.if(n = 9727)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 9728)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9729)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9730)
	; 20 bytes
	ldy #207
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 9731)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 9732)
	; 21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9733)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 9734)
	; 22 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 9735)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 9736)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9737)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 9738)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9739)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 9740)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9741)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 9742)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9743)
	; 22 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9744)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 9745)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.endif
.else
; 9746 to 9765
.if(n = 9746)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9747)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 9748)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9749)
	; 22 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 9750)
	; 20 bytes
	ldy #200 ;hides 'iny'
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	dey
	Jne *-17
.elseif(n = 9751)
	; 17 bytes
	ldy #250
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9752)
	; 21 bytes
	ldy #199
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9753 || (n > 9754 && n < 9759) || n = 9761 || n = 9763 || n = 9764)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9754 || n = 9765)
	; 19..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9759)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9760)
	; 22 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 9762)
	; 18 bytes
	ldy #227
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.endif
.endif
.endif
.else
.if((n >= 9766) && (n < 9805))
.if((n >= 9766) && (n < 9785))
; 9766 to 9784
.if(n = 9766)
	; 19 bytes
	ldy #217
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 9767 || n = 9768 || n = 9770 || n = 9779)
	; 20..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9769)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9771)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9772)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 9773)
	; 22 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 9774)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 9775)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9776)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 9777)
	; 20 bytes
	ldy #208
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 9778)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 9780)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 9781)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 9782)
	; 22 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9783)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 9784)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.endif
.else
; 9785 to 9804
.if(n = 9785)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9786)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 9787)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9788)
	; 22 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 9789)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 9790)
	; 17 bytes
	ldy #251
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9791)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 9792 || (n > 9793 && n < 9798) || n = 9802 || n = 9804)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9793 || n = 9803)
	; 19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9798)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9799)
	; 20 bytes
	ldy #56 ;hides 'sec'
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-17
.elseif(n = 9800)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9801)
	; 21 bytes
	ldy #200
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.endif
.endif
.else
; 9805 to 9843
.if(n = 9805)
	; 18 bytes
	ldy #228
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9806 || n = 9807 || n = 9809 || n = 9813 || n = 9815 || n = 9826 || n = 9831 || (n > 9832 && n < 9837) || n = 9843)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9808 || n = 9814 || n = 9832)
	; 19..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9810 || n = 9839)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9811)
	; 19 bytes
	ldy #218
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 9812)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9816)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9817)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 9818)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9819)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 9820)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 9821)
	; 22 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9822)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 9823)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 9824)
	; 20 bytes
	ldy #209
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 9825)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 9827)
	; 22 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 9828)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 9829)
	; 17 bytes
	ldy #252
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9830)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 9837)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9838)
	; 22 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 9840)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9841)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9842)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.else
.if((n >= 9844) && (n < 9922))
.if((n >= 9844) && (n < 9883))
; 9844 to 9882
.if(n = 9844 || n = 9851 || n = 9871)
	; 19..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 9844 && n < 9848) || n = 9850 || n = 9852 || n = 9854 || n = 9858 || n = 9860 || n = 9870 || (n > 9871 && n < 9876))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9848)
	; 18 bytes
	ldy #229
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9849)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 9853 || n = 9880)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9855)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9856)
	; 19 bytes
	ldy #219
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 9857)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9859)
	; 21 bytes
	ldy #186
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9861)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 9862)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 9863)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9864)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 9865)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9866)
	; 22 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 9867)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 9868)
	; 17 bytes
	ldy #253
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9869)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 9876)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9877)
	; 22 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 9878)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9879)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9881)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9882)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.endif
.else
; 9883 to 9921
.if(n = 9883)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 9884 || (n > 9885 && n < 9890) || n = 9893 || (n > 9894 && n < 9898) || n = 9903 || n = 9905 || n = 9909 || (n > 9910 && n < 9915) || n = 9920)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9885 || n = 9894 || n = 9904 || n = 9910)
	; 19..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9890)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 9891)
	; 18 bytes
	ldy #230
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9892 || n = 9917)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9898)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 9899)
	; 21 bytes
	ldy #202
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9900)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 9901)
	; 19 bytes
	ldy #220
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 9902)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9906)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 9907)
	; 17 bytes
	ldy #254
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9908)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 9915)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9916)
	; 22 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 9918)
	; 20 bytes
	ldy #211
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 9919)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9921)
	; 22 bytes
	delay_n 131
	delay_n (n-131)
.endif
.endif
.else
.if((n >= 9922) && (n < 9961))
; 9922 to 9960
.if(n = 9922)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 9923)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9924)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 9925 || (n > 9926 && n < 9931) || n = 9936 || (n > 9937 && n < 9941) || n = 9948 || (n > 9949 && n < 9954))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9926 || n = 9937 || n = 9949)
	; 19..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9931)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 9932)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 9933 || n = 9956)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9934)
	; 18 bytes
	ldy #231
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9935 || n = 9958)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9941)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9942)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 9943)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9944)
	; 22 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 9945)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 9946)
	; 17 bytes
	ldy #255
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9947)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 9954)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9955)
	; 22 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 9957)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9959)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9960)
	; 22 bytes
	delay_n 131
	delay_n (n-131)
.endif
.else
; 9961 to 10000
.if(n = 9961 || n = 10000)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 9962)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9963)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 9964)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9965)
	; 20 bytes
	ldy #212
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 9966 || (n > 9967 && n < 9972) || n = 9979 || (n > 9980 && n < 9984) || n = 9987 || n = 9989 || n = 9990 || n = 9992 || n = 9993 || n = 9999)
	; 18..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 9967 || n = 9978 || n = 9980 || n = 9988 || n = 9994)
	; 19..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 9972)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 9973)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 9974 || n = 9995)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9975)
	; 17 bytes
	ldy #24 ;hides 'clc'
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-14
.elseif(n = 9976)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9977)
	; 18 bytes
	ldy #232
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 9984)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 9985)
	; 17 bytes
	ldy #0
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 9986 || n = 9996)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9991)
	; 19 bytes
	ldy #222
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 9997)
	; 21 bytes
	ldy #204
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 9998)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
.if((n >= 10001) && (n < 12501))
.if((n >= 10001) && (n < 11251))
.if((n >= 10001) && (n < 10626))
.if((n >= 10001) && (n < 10313))
.if((n >= 10001) && (n < 10157))
.if((n >= 10001) && (n < 10079))
.if((n >= 10001) && (n < 10040))
.if((n >= 10001) && (n < 10020))
; 10001 to 10019
.if(n = 10001)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10002)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 10003)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10004)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 10005)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 10006)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10007 || (n > 10008 && n < 10012) || n = 10014)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10008)
	; 19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10012)
	; 20 bytes
	ldy #213
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10013)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 10015)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10016)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10017)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10018)
	; 21 bytes
	ldy #189
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10019)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 10020 to 10039
.if(n = 10020)
	; 18 bytes
	ldy #233
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10021)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10022 || (n > 10023 && n < 10027) || n = 10038)
	; 19..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10023 || n = 10039)
	; 20..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10027)
	; 22 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10028)
	; 22 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 10029)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10030)
	; 22 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10031)
	; 22 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10032)
	; 22 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 10033)
	; 22 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 10034)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 10035)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 10036)
	; 19 bytes
	ldy #223
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10037)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
; 10040 to 10078
.if(n = 10040 || n = 10048 || (n > 10049 && n < 10054) || n = 10065 || (n > 10066 && n < 10070))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10041)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 10042 || n = 10060)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10043)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 10044)
	; 22 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 10045)
	; 22 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10046)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 10047)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10049 || n = 10064 || n = 10066)
	; 19..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10054)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 10055 || n = 10076)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10056)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10057 || n = 10072)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10058 || n = 10073)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10059)
	; 20 bytes
	ldy #214
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10061)
	; 17 bytes
	ldy #234 ;hides 'nop'
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10062)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10063)
	; 18 bytes
	ldy #234
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10070)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10071)
	; 21 bytes
	ldy #190
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10074)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10075)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 10077)
	; 22 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 10078)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.endif
.endif
.else
.if((n >= 10079) && (n < 10118))
; 10079 to 10117
.if(n = 10079 || n = 10102)
	; 22 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10080 || n = 10101)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10081)
	; 19 bytes
	ldy #224
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10082 || n = 10100)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 10083 || n = 10085 || n = 10089 || (n > 10090 && n < 10095) || n = 10108 || (n > 10109 && n < 10113))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10084 || n = 10090 || n = 10109)
	; 19..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10086)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10087)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 10088 || n = 10104)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10095)
	; 21 bytes
	ldy #206
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10096)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 10097)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10098)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10099)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10103)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10105)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10106)
	; 18 bytes
	ldy #235
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10107)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10113)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10114)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10115)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10116)
	; 22 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 10117)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.endif
.else
; 10118 to 10156
.if(n = 10118)
	; 22 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10119 || n = 10142)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10120 || n = 10141)
	; 22 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10121)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 10122)
	; 22 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 10123)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10124)
	; 21 bytes
	ldy #191
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10125 || n = 10146)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10126)
	; 19 bytes
	ldy #225
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10127 || n = 10145)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10128)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 10129 || n = 10131 || n = 10152)
	; 19..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10130 || (n > 10131 && n < 10136) || n = 10151 || (n > 10152 && n < 10156))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10136)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 10137)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10138)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10139)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10140)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10143)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 10144)
	; 21 bytes
	ldy #207
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10147)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10148)
	; 21 bytes
	ldy #200 ;hides 'iny'
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	dey
	Jne *-18
.elseif(n = 10149)
	; 18 bytes
	ldy #236
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10150)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10156)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.endif
.endif
.endif
.else
.if((n >= 10157) && (n < 10235))
.if((n >= 10157) && (n < 10196))
.if((n >= 10157) && (n < 10176))
; 10157 to 10175
.if(n = 10157)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 10158)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 10159)
	; 22 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10160)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10161)
	; 22 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 10162)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10163)
	; 22 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 10164)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10165)
	; 23 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10166)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10167)
	; 17 bytes
	ldy #248 ;hides 'sed'
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10168)
	; 22 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10169)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 10170 || n = 10172)
	; 19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10171 || (n > 10172 && n < 10176))
	; 18..20 bytes
	delay_n (n-2)
	nop
.endif
.else
; 10176 to 10195
.if(n = 10176 || n = 10194)
	; 19..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10177)
	; 21 bytes
	ldy #192
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10178 || n = 10180)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10179)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10181)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10182)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 10183)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10184)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 10185)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10186)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10187)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10188)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10189)
	; 22 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 10190)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10191)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 10192)
	; 18 bytes
	ldy #237
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10193)
	; 21 bytes
	ldy #208
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10195)
	; 20 bytes
	delay_n (n-3)
	ldy $A4
.endif
.endif
.else
.if((n >= 10196) && (n < 10215))
; 10196 to 10214
.if((n > 10195 && n < 10199) || n = 10202 || n = 10203 || n = 10206 || n = 10211 || n = 10212 || n = 10214)
	; 18..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10199)
	; 21 bytes
	ldy #56 ;hides 'sec'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-18
.elseif(n = 10200)
	; 20 bytes
	ldy #217
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10201)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10204)
	; 22 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 10205)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10207)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10208)
	; 22 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 10209)
	; 22 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10210)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 10213)
	; 19 bytes
	delay_n (n-3)
	ldy $A4
.endif
.else
; 10215 to 10234
.if(n = 10215 || n = 10217 || n = 10218)
	; 20..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10216)
	; 19 bytes
	ldy #227
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10219)
	; 21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10220)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10221)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10222)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10223)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 10224)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10225)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 10226)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10227)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10228)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10229)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10230)
	; 21 bytes
	ldy #193
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10231)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10232)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 10233)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 10234)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.endif
.else
.if((n >= 10235) && (n < 10274))
; 10235 to 10273
.if(n = 10235)
	; 18 bytes
	ldy #238
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10236)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10237 || (n > 10238 && n < 10242) || n = 10244 || n = 10249 || n = 10253 || (n > 10254 && n < 10259) || n = 10263 || n = 10265)
	; 18..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 10238 || n = 10254 || n = 10264)
	; 19..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10242)
	; 21 bytes
	ldy #209
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10243)
	; 22 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10245)
	; 22 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10246)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10247)
	; 20 bytes
	ldy #218
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10248)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10250)
	; 22 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10251)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 10252)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10259)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 10260)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10261)
	; 19 bytes
	ldy #228
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10262)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10266)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 10267)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10268)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10269)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10270)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10271)
	; 22 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 10272)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10273)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.endif
.else
; 10274 to 10312
.if(n = 10274)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 10275)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10276)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 10277)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10278)
	; 18 bytes
	ldy #239
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10279)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10280 || n = 10282 || n = 10284 || n = 10293 || n = 10294 || (n > 10295 && n < 10300) || n = 10305 || n = 10308 || n = 10310)
	; 18..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 10281 || n = 10295 || n = 10309)
	; 19..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10283)
	; 21 bytes
	ldy #194
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10285)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10286)
	; 22 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 10287)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10288 || n = 10302)
	; 21..22 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10289)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10290)
	; 22 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 10291)
	; 21 bytes
	ldy #210
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10292)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 10300)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 10301)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10303)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10304)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10306)
	; 19 bytes
	ldy #229
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10307)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 10311)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10312)
	; 22 bytes
	delay_n 61
	delay_n (n-61)
.endif
.endif
.endif
.endif
.else
.if((n >= 10313) && (n < 10469))
.if((n >= 10313) && (n < 10391))
.if((n >= 10313) && (n < 10352))
.if((n >= 10313) && (n < 10332))
; 10313 to 10331
.if(n = 10313)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10314)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 10315)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 10316)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10317)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 10318)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10319)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10320)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10321)
	; 18 bytes
	ldy #240
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10322)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 10323 || (n > 10324 && n < 10328))
	; 19..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10324)
	; 20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10328)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10329)
	; 22 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10330)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10331)
	; 22 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 10332 to 10351
.if(n = 10332)
	; 22 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10333)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 10334)
	; 22 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 10335 || (n > 10336 && n < 10340) || n = 10342)
	; 18..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 10336)
	; 19 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10340)
	; 21 bytes
	ldy #211
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10341)
	; 20 bytes
	ldy #220
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10343)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10344)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10345)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10346)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 10347)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10348)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 10349)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10350)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10351)
	; 19 bytes
	ldy #230
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.endif
.endif
.else
; 10352 to 10390
.if(n = 10352)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10353 || n = 10355 || n = 10356 || n = 10366 || (n > 10367 && n < 10371) || n = 10376 || (n > 10377 && n < 10382) || n = 10390)
	; 18..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 10354)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10357)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10358)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 10359)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10360)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10361)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10362)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 10363)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 10364)
	; 18 bytes
	ldy #241
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10365)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10367 || n = 10377)
	; 19..20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10371)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10372)
	; 22 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 10373)
	; 22 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10374)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 10375 || n = 10385)
	; 21..22 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10382)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 10383)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10384)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10386)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10387)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 10388)
	; 20 bytes
	ldy #221
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10389)
	; 21 bytes
	ldy #212
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.endif
.endif
.else
.if((n >= 10391) && (n < 10430))
; 10391 to 10429
.if(n = 10391 || n = 10398 || n = 10400 || n = 10409 || (n > 10410 && n < 10414) || n = 10416 || n = 10417 || (n > 10418 && n < 10423))
	; 18..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10392)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10393)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10394)
	; 22 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 10395)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10396)
	; 19 bytes
	ldy #231
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10397)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 10399 || n = 10410 || n = 10418)
	; 19..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10401)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10402)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10403)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 10404)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 10405)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10406)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10407)
	; 18 bytes
	ldy #242
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10408)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10414)
	; 22 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10415)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 10423)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 10424)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10425)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10426)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10427)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10428)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 10429)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 10430 to 10468
.if(n = 10430)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 10431)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10432)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10433)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10434)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10435)
	; 20 bytes
	ldy #222
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10436)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10437 || n = 10440 || n = 10443 || n = 10445 || n = 10452 || n = 10454 || n = 10455 || n = 10457 || n = 10458 || (n > 10459 && n < 10464))
	; 18..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 10438)
	; 21 bytes
	ldy #213
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10439)
	; 18 bytes
	ldy #24 ;hides 'clc'
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-15
.elseif(n = 10441)
	; 19 bytes
	ldy #232
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10442 || n = 10444 || n = 10453 || n = 10459)
	; 19..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10446)
	; 22 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10447)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10448)
	; 22 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10449)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10450)
	; 18 bytes
	ldy #243
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10451)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10456)
	; 17 bytes
	ldy #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 10464)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 10465)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10466)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10467)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10468)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.endif
.else
.if((n >= 10469) && (n < 10547))
.if((n >= 10469) && (n < 10508))
.if((n >= 10469) && (n < 10488))
; 10469 to 10487
.if(n = 10469)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 10470)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10471)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 10472)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10473)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10474)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10475)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10476)
	; 22 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 10477)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10478)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 10479)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 10480)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10481)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 10482)
	; 20 bytes
	ldy #223
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10483)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10484)
	; 21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10485)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 10486)
	; 19 bytes
	ldy #233
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10487)
	; 21 bytes
	ldy #214
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.endif
.else
; 10488 to 10507
.if(n = 10488 || n = 10490 || n = 10495 || n = 10498 || n = 10499 || (n > 10500 && n < 10505))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10489 || n = 10496 || n = 10500)
	; 19..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10491)
	; 22 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 10492)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10493)
	; 18 bytes
	ldy #244
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10494)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10497)
	; 17 bytes
	ldy #0
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-13
.elseif(n = 10505)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 10506)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10507)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 10508) && (n < 10527))
; 10508 to 10526
.if(n = 10508)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10509)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10510)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 10511)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10512)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 10513)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10514)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10515)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10516)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10517)
	; 22 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 10518)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10519)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 10520)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 10521)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10522)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 10523)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10524)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10525)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10526)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.endif
.else
; 10527 to 10546
.if(n = 10527)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 10528)
	; 22 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10529)
	; 18 bytes
	ldy #234 ;hides 'nop'
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10530)
	; 22 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10531)
	; 19 bytes
	ldy #234
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10532 || n = 10534 || n = 10539)
	; 20..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10533 || n = 10535 || n = 10538 || (n > 10539 && n < 10543))
	; 19..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10536)
	; 18 bytes
	ldy #245
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10537)
	; 22 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10543)
	; 22 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 10544)
	; 22 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10545)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 10546)
	; 21 bytes
	delay_n 49
	delay_n (n-49)
.endif
.endif
.endif
.else
.if((n >= 10547) && (n < 10586))
.if((n >= 10547) && (n < 10566))
; 10547 to 10565
.if(n = 10547)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10548)
	; 21 bytes
	ldy #199
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10549 || n = 10559 || n = 10560 || n = 10564)
	; 22..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10550)
	; 22 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 10551)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 10552)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10553)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 10554)
	; 23 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 10555)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10556)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10557)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10558)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10561)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 10562)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10563)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 10565)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.endif
.else
; 10566 to 10585
.if(n = 10566)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10567)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 10568)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 10569 || n = 10574 || n = 10575 || n = 10578 || n = 10580 || n = 10581 || n = 10583 || n = 10584)
	; 19..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10570)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10571)
	; 22 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10572)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10573)
	; 22 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 10576)
	; 19 bytes
	ldy #235
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10577)
	; 22 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 10579)
	; 18 bytes
	ldy #246
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10582)
	; 20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10585)
	; 21 bytes
	ldy #216
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.endif
.endif
.else
.if((n >= 10586) && (n < 10606))
; 10586 to 10605
.if(n = 10586)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 10587 || n = 10594 || n = 10595 || n = 10600 || n = 10605)
	; 22..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10588)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10589)
	; 22 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10590)
	; 22 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10591)
	; 22 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 10592)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 10593)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10596)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10597)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10598)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10599)
	; 21 bytes
	ldy #56 ;hides 'sec'
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-18
.elseif(n = 10601)
	; 21 bytes
	ldy #200
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10602)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 10603)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10604)
	; 23 bytes
	delay_n (n-3)
	ldy $A4
.endif
.else
; 10606 to 10625
.if(n = 10606)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10607)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10608)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 10609)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10610 || n = 10611 || n = 10614 || n = 10615 || n = 10619 || n = 10620 || n = 10624)
	; 19..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10612)
	; 22 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10613)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10616)
	; 22 bytes
	ldy #193
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 10617)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10618)
	; 22 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 10621)
	; 19 bytes
	ldy #236
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10622)
	; 18 bytes
	ldy #247
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10623)
	; 20 bytes
	ldy #226
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10625)
	; 20 bytes
	delay_n (n-3)
	ldy $A4
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 10626) && (n < 10938))
.if((n >= 10626) && (n < 10782))
.if((n >= 10626) && (n < 10704))
.if((n >= 10626) && (n < 10665))
; 10626 to 10664
.if((n > 10625 && n < 10630) || n = 10635 || n = 10636 || n = 10639 || n = 10640 || n = 10644 || n = 10645 || n = 10649 || n = 10655 || n = 10656 || n = 10662)
	; 20..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10630)
	; 22 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 10631)
	; 23 bytes
	delay_n 134
	delay_n (n-134)
.elseif(n = 10632)
	; 22 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10633)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 10634)
	; 21 bytes
	ldy #217
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10637)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10638)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10641 || n = 10647)
	; 22..23 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10642)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 10643)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 10646)
	; 23 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 10648)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10650)
	; 22 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 10651)
	; 23 bytes
	ldy #213
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 10652)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10653)
	; 22 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10654)
	; 21 bytes
	ldy #201
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10657)
	; 23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10658)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10659)
	; 23 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 10660)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10661)
	; 24 bytes
	ldy #205
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 10663)
	; 17 bytes
	ldy #248 ;hides 'sed'
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10664)
	; 22 bytes
	delay_n 167
	delay_n (n-167)
.endif
.else
.if((n >= 10665) && (n < 10684))
; 10665 to 10683
.if(n = 10665)
	; 18 bytes
	ldy #248
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10666)
	; 19 bytes
	ldy #237
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10667 || n = 10669 || n = 10671 || n = 10672)
	; 19..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10668)
	; 20 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10670)
	; 20 bytes
	ldy #227
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10673)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10674 || n = 10676)
	; 21..22 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10675 || n = 10677)
	; 21..22 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10678)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10679 || n = 10681)
	; 22..23 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10680)
	; 23 bytes
	ldy #181
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 10682)
	; 23 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 10683)
	; 21 bytes
	ldy #218
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.endif
.else
; 10684 to 10703
.if(n = 10684)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10685 || n = 10687 || n = 10690 || n = 10695)
	; 22..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10686)
	; 23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10688)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10689 || n = 10691)
	; 22..23 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10692)
	; 23 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10693)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10694 || n = 10696)
	; 22..23 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10697)
	; 22 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10698)
	; 23 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 10699)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10700)
	; 23 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 10701)
	; 23 bytes
	ldy #214
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 10702)
	; 23 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 10703)
	; 23 bytes
	delay_n 81
	delay_n (n-81)
.endif
.endif
.endif
.else
.if((n >= 10704) && (n < 10743))
; 10704 to 10742
.if(n = 10704 || n = 10706)
	; 22..23 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 10705)
	; 23 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 10707)
	; 21 bytes
	ldy #202
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10708)
	; 18 bytes
	ldy #249
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10709 || n = 10710 || n = 10712 || n = 10713 || n = 10715 || n = 10716 || n = 10719 || n = 10721 || n = 10734 || n = 10736 || n = 10737)
	; 19..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 10711)
	; 19 bytes
	ldy #238
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10714 || n = 10735)
	; 21..23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10717)
	; 20 bytes
	ldy #228
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10718)
	; 22 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10720)
	; 22 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10722)
	; 23 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 10723)
	; 23 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10724)
	; 22 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 10725)
	; 23 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 10726)
	; 22 bytes
	ldy #195
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 10727)
	; 23 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10728)
	; 23 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10729 || n = 10731)
	; 22..23 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 10730)
	; 23 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10732)
	; 21 bytes
	ldy #219
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10733)
	; 23 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 10738)
	; 23 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10739 || n = 10741)
	; 22..23 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 10740)
	; 23 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10742)
	; 23 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
.if((n >= 10743) && (n < 10762))
; 10743 to 10761
.if(n = 10743)
	; 23 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 10744 || n = 10746)
	; 22..23 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10745)
	; 23 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 10747)
	; 24 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10748)
	; 23 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 10749)
	; 22 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 10750)
	; 23 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 10751)
	; 18 bytes
	ldy #250
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10752)
	; 23 bytes
	delay_n 89
	delay_n (n-89)
.elseif(n = 10753 || n = 10755 || n = 10757 || n = 10758)
	; 19..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10754 || n = 10759)
	; 20..21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10756)
	; 19 bytes
	ldy #239
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10760)
	; 21 bytes
	ldy #203
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10761)
	; 22 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 10762 to 10781
.if(n = 10762)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10763)
	; 22 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10764)
	; 20 bytes
	ldy #229
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10765 || n = 10767)
	; 22..23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10766 || n = 10768)
	; 21..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 10769 || n = 10771)
	; 22..23 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10770)
	; 23 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10772)
	; 23 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10773)
	; 23 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10774 || n = 10776)
	; 22..23 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 10775)
	; 23 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10777)
	; 23 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10778)
	; 23 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10779)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10780)
	; 24 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 10781)
	; 21 bytes
	ldy #220
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.endif
.endif
.endif
.endif
.else
.if((n >= 10782) && (n < 10860))
.if((n >= 10782) && (n < 10821))
; 10782 to 10820
.if(n = 10782)
	; 23 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10783 || n = 10785 || n = 10795 || n = 10796 || (n > 10797 && n < 10801) || n = 10803 || n = 10805 || n = 10807 || n = 10815 || n = 10817)
	; 19..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 10784 || n = 10786)
	; 22..23 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 10787)
	; 23 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10788)
	; 23 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 10789 || n = 10791)
	; 22..23 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 10790)
	; 23 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 10792)
	; 23 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 10793)
	; 23 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 10794)
	; 18 bytes
	ldy #251
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10797 || n = 10804 || n = 10814)
	; 20..22 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10801)
	; 19 bytes
	ldy #240
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10802)
	; 22 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10806)
	; 22 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10808)
	; 23 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 10809)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 10810)
	; 23 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10811)
	; 20 bytes
	ldy #230
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10812)
	; 23 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 10813)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10816)
	; 22 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 10818)
	; 23 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10819)
	; 22 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10820)
	; 23 bytes
	delay_n 26
	delay_n (n-26)
.endif
.else
; 10821 to 10859
.if(n = 10821)
	; 23 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10822)
	; 23 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 10823)
	; 24 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 10824 || n = 10826)
	; 22..23 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10825)
	; 23 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10827)
	; 23 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 10828)
	; 23 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10829 || n = 10831)
	; 22..23 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 10830)
	; 21 bytes
	ldy #221
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10832 || n = 10838 || n = 10839 || (n > 10840 && n < 10845) || n = 10848 || (n > 10849 && n < 10853))
	; 19..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10833 || n = 10840 || n = 10849)
	; 20..23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10834)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 10835)
	; 23 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 10836)
	; 22 bytes
	ldy #197
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 10837)
	; 18 bytes
	ldy #252
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10845)
	; 22 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10846)
	; 19 bytes
	ldy #241
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10847 || n = 10856)
	; 22..23 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10853)
	; 23 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10854)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10855)
	; 23 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 10857)
	; 23 bytes
	ldy #184
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 10858)
	; 20 bytes
	ldy #231
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10859)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.endif
.endif
.else
.if((n >= 10860) && (n < 10899))
; 10860 to 10898
.if(n = 10860 || n = 10862 || n = 10868 || n = 10876 || n = 10881 || n = 10882 || (n > 10883 && n < 10888) || n = 10893 || (n > 10894 && n < 10898))
	; 19..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 10861 || n = 10867 || n = 10869 || n = 10883 || n = 10894)
	; 20..23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10863)
	; 23 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10864)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10865)
	; 23 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 10866)
	; 21 bytes
	ldy #205
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10870)
	; 23 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 10871)
	; 23 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10872)
	; 23 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 10873)
	; 23 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10874)
	; 23 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 10875)
	; 23 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10877)
	; 23 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 10878)
	; 23 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 10879)
	; 21 bytes
	ldy #222
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10880)
	; 18 bytes
	ldy #253
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10888)
	; 22 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10889)
	; 23 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10890)
	; 22 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10891)
	; 19 bytes
	ldy #242
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10892)
	; 22 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10898)
	; 23 bytes
	delay_n 61
	delay_n (n-61)
.endif
.else
.if((n >= 10899) && (n < 10918))
; 10899 to 10917
.if(n = 10899)
	; 23 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10900)
	; 23 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10901)
	; 23 bytes
	ldy #218
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 10902)
	; 23 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10903)
	; 19 bytes
	ldy #24 ;hides 'clc'
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-16
.elseif(n = 10904)
	; 23 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10905)
	; 20 bytes
	ldy #232
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10906 || n = 10908)
	; 21..22 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10907 || n = 10909 || n = 10917)
	; 21..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 10910)
	; 23 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10911)
	; 23 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10912)
	; 23 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10913)
	; 23 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 10914)
	; 23 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 10915)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10916)
	; 23 bytes
	ldy #185
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.endif
.else
; 10918 to 10937
.if(n = 10918 || n = 10922 || n = 10926)
	; 20..23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10919)
	; 21 bytes
	ldy #206
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10920)
	; 23 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 10921 || n = 10924 || n = 10925 || n = 10927 || n = 10929 || n = 10930 || n = 10932)
	; 19..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 10923)
	; 18 bytes
	ldy #254
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10928)
	; 21 bytes
	ldy #223
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10931)
	; 22 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10933)
	; 22 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10934)
	; 22 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10935)
	; 22 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10936)
	; 19 bytes
	ldy #243
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10937)
	; 23 bytes
	delay_n 186
	delay_n (n-186)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 10938) && (n < 11094))
.if((n >= 10938) && (n < 11016))
.if((n >= 10938) && (n < 10977))
; 10938 to 10976
.if(n = 10938 || (n > 10939 && n < 10943) || n = 10954 || n = 10956 || n = 10957 || n = 10968 || n = 10970 || n = 10971 || n = 10973 || n = 10974)
	; 19..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10939 || n = 10955 || n = 10969 || n = 10975)
	; 20..23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10943)
	; 23 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10944)
	; 22 bytes
	ldy #200 ;hides 'iny'
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	dey
	Jne *-19
.elseif(n = 10945)
	; 23 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10946)
	; 22 bytes
	ldy #199
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 10947)
	; 23 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10948)
	; 23 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 10949 || n = 10962)
	; 23..24 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10950)
	; 23 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10951)
	; 23 bytes
	ldy #219
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 10952)
	; 20 bytes
	ldy #233
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10953)
	; 23 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10958)
	; 23 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 10959)
	; 23 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10960)
	; 23 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 10961)
	; 23 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10963)
	; 23 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 10964)
	; 23 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 10965)
	; 23 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 10966)
	; 18 bytes
	ldy #255
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.elseif(n = 10967)
	; 24 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10972)
	; 21 bytes
	ldy #207
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10976)
	; 22 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 10977 to 11015
.if(n = 10977)
	; 21 bytes
	ldy #224
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 10978)
	; 22 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10979 || n = 10983 || (n > 10984 && n < 10988) || n = 11001 || n = 11003 || n = 11005 || n = 11010 || n = 11011 || (n > 11012 && n < 11016))
	; 19..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 10980 || n = 10984 || n = 11000 || n = 11002 || n = 11012)
	; 20..23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 10981)
	; 19 bytes
	ldy #244
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 10982)
	; 23 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10988)
	; 23 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10989)
	; 23 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 10990)
	; 23 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10991)
	; 23 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10992)
	; 23 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10993)
	; 23 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10994)
	; 23 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 10995)
	; 24 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 10996)
	; 23 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10997)
	; 19 bytes
	ldy #234 ;hides 'nop'
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 10998)
	; 23 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10999)
	; 20 bytes
	ldy #234
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11004)
	; 23 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11006)
	; 23 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 11007)
	; 23 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 11008)
	; 23 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 11009)
	; 18 bytes
	ldy #0
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-14
.endif
.endif
.else
.if((n >= 11016) && (n < 11055))
.if((n >= 11016) && (n < 11035))
; 11016 to 11034
.if(n = 11016 || n = 11027 || n = 11028 || (n > 11029 && n < 11033))
	; 20..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 11017)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11018)
	; 23 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 11019)
	; 22 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11020)
	; 22 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11021)
	; 22 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11022)
	; 23 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 11023)
	; 23 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 11024)
	; 23 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 11025)
	; 21 bytes
	ldy #208
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11026)
	; 19 bytes
	ldy #245
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 11029)
	; 21 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11033)
	; 23 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 11034)
	; 23 bytes
	ldy #187
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.endif
.else
; 11035 to 11054
.if(n = 11035)
	; 23 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11036)
	; 23 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11037)
	; 23 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 11038)
	; 23 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11039)
	; 23 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 11040)
	; 23 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11041)
	; 23 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 11042)
	; 23 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 11043)
	; 23 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11044)
	; 23 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 11045)
	; 23 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11046)
	; 20 bytes
	ldy #235
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11047)
	; 23 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11048 || n = 11050 || n = 11053)
	; 21..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11049)
	; 22 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11051)
	; 23 bytes
	ldy #221
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11052)
	; 23 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 11054)
	; 23 bytes
	delay_n 131
	delay_n (n-131)
.endif
.endif
.else
.if((n >= 11055) && (n < 11074))
; 11055 to 11073
.if(n = 11055)
	; 23 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11056)
	; 22 bytes
	ldy #201
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11057)
	; 23 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11058)
	; 22 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 11059)
	; 23 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 11060)
	; 23 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 11061)
	; 23 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 11062)
	; 23 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11063 || n = 11070 || n = 11072 || n = 11073)
	; 20..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11064)
	; 23 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11065)
	; 23 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 11066)
	; 23 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 11067)
	; 23 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 11068)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11069)
	; 23 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 11071)
	; 19 bytes
	ldy #246
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.endif
.else
; 11074 to 11093
.if(n = 11074 || n = 11081)
	; 21..23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11075)
	; 21 bytes
	ldy #226
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11076 || n = 11077 || n = 11079 || n = 11080 || n = 11086 || n = 11091)
	; 22..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11078)
	; 21 bytes
	ldy #209
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11082)
	; 23 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 11083)
	; 23 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11084)
	; 23 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 11085)
	; 23 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 11087)
	; 23 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 11088)
	; 24 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11089)
	; 23 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 11090)
	; 23 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11092)
	; 23 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 11093)
	; 20 bytes
	ldy #236
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.endif
.endif
.endif
.endif
.else
.if((n >= 11094) && (n < 11172))
.if((n >= 11094) && (n < 11133))
.if((n >= 11094) && (n < 11113))
; 11094 to 11112
.if(n = 11094)
	; 23 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 11095 || n = 11097 || n = 11098 || n = 11103 || n = 11106)
	; 21..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11096)
	; 22 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11099)
	; 23 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 11100)
	; 23 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11101)
	; 23 bytes
	ldy #222
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11102)
	; 23 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 11104)
	; 23 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 11105)
	; 23 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11107)
	; 23 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11108)
	; 24 bytes
	delay_n 99
	delay_n (n-99)
.elseif(n = 11109)
	; 23 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 11110)
	; 23 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 11111)
	; 22 bytes
	ldy #202
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11112)
	; 23 bytes
	delay_n 146
	delay_n (n-146)
.endif
.else
; 11113 to 11132
.if(n = 11113 || n = 11118 || (n > 11119 && n < 11124) || n = 11126 || n = 11128)
	; 20..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 11114)
	; 23 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 11115)
	; 23 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11116)
	; 19 bytes
	ldy #247
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 11117)
	; 23 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11119)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11124)
	; 21 bytes
	ldy #227
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11125)
	; 23 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 11127)
	; 23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11129)
	; 24 bytes
	ldy #214
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 11130)
	; 23 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 11131)
	; 21 bytes
	ldy #210
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11132)
	; 23 bytes
	delay_n 166
	delay_n (n-166)
.endif
.endif
.else
; 11133 to 11171
.if(n = 11133 || n = 11136 || n = 11139 || n = 11142 || n = 11144 || n = 11146 || n = 11149 || n = 11153 || n = 11154 || n = 11163 || n = 11165 || n = 11167 || n = 11168)
	; 20..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11134 || n = 11143 || n = 11162 || n = 11164)
	; 20..23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11135)
	; 23 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 11137)
	; 23 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 11138 || n = 11150)
	; 23..24 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11140)
	; 20 bytes
	ldy #237
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11141 || n = 11169)
	; 22..25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11145)
	; 23 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 11147)
	; 23 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 11148 || n = 11160)
	; 23..24 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11151)
	; 23 bytes
	ldy #223
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11152)
	; 23 bytes
	ldy #189
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11155)
	; 23 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 11156)
	; 24 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 11157)
	; 23 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 11158)
	; 24 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 11159)
	; 18 bytes
	ldy #248 ;hides 'sed'
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 11161)
	; 19 bytes
	ldy #248
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 11166)
	; 22 bytes
	ldy #203
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11170)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11171)
	; 22 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.else
.if((n >= 11172) && (n < 11211))
; 11172 to 11210
.if(n = 11172 || n = 11174 || n = 11175 || (n > 11176 && n < 11180) || n = 11182 || n = 11186 || n = 11188 || n = 11189 || (n > 11190 && n < 11195) || n = 11197 || n = 11202 || n = 11203 || n = 11207 || n = 11208 || n = 11210)
	; 20..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11173)
	; 21 bytes
	ldy #228
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11176 || n = 11190 || n = 11204 || n = 11209)
	; 21..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11180)
	; 23 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 11181)
	; 24 bytes
	ldy #215
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 11183)
	; 23 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 11184)
	; 21 bytes
	ldy #211
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11185)
	; 23 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 11187)
	; 20 bytes
	ldy #238
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11195)
	; 23 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 11196)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11198)
	; 24 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11199)
	; 24 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 11200)
	; 23 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 11201)
	; 23 bytes
	ldy #224
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11205)
	; 23 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 11206)
	; 19 bytes
	ldy #249
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.endif
.else
; 11211 to 11250
.if((n > 11210 && n < 11214) || n = 11219 || n = 11223 || n = 11224 || n = 11226 || n = 11227 || n = 11229 || n = 11236 || n = 11238 || n = 11239 || (n > 11240 && n < 11244))
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11214)
	; 23 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11215)
	; 23 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 11216 || n = 11244)
	; 23..24 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11217)
	; 23 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11218 || n = 11246 || n = 11249)
	; 23..25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11220)
	; 23 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 11221)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11222)
	; 21 bytes
	ldy #229
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11225 || n = 11240)
	; 23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11228)
	; 23 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11230)
	; 23 bytes
	ldy #197
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11231)
	; 23 bytes
	ldy #216 ;hides 'cld'
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 11232)
	; 24 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11233)
	; 24 bytes
	ldy #216
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 11234)
	; 20 bytes
	ldy #239
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11235)
	; 23 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 11237)
	; 21 bytes
	ldy #212
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11245)
	; 23 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 11247)
	; 24 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 11248)
	; 24 bytes
	delay_n 89
	delay_n (n-89)
.elseif(n = 11250)
	; 23 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 11251) && (n < 11876))
.if((n >= 11251) && (n < 11563))
.if((n >= 11251) && (n < 11407))
.if((n >= 11251) && (n < 11329))
.if((n >= 11251) && (n < 11290))
; 11251 to 11289
.if(n = 11251)
	; 19 bytes
	ldy #250
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 11252)
	; 24 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11253 || (n > 11254 && n < 11259) || n = 11264 || n = 11273 || n = 11274 || n = 11277 || n = 11278 || n = 11283 || n = 11285 || n = 11286 || n = 11288 || n = 11289)
	; 20..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11254 || n = 11275 || n = 11279 || n = 11284)
	; 21..24 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11259)
	; 23 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11260)
	; 23 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 11261)
	; 23 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11262)
	; 23 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11263)
	; 23 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11265)
	; 23 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11266)
	; 24 bytes
	delay_n 107
	delay_n (n-107)
.elseif(n = 11267)
	; 24 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11268)
	; 25 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11269)
	; 24 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 11270)
	; 23 bytes
	ldy #191
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11271)
	; 21 bytes
	ldy #230
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11272)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11276)
	; 22 bytes
	ldy #205
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11280)
	; 23 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 11281)
	; 20 bytes
	ldy #240
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11282)
	; 24 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11287)
	; 23 bytes
	ldy #198
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.endif
.else
; 11290 to 11328
.if(n = 11290)
	; 21 bytes
	ldy #213
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11291 || n = 11306)
	; 23..24 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11292 || n = 11294 || n = 11298 || (n > 11299 && n < 11304) || n = 11309 || n = 11319 || n = 11322 || n = 11324 || n = 11325 || n = 11327)
	; 20..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11293 || n = 11299 || n = 11326)
	; 21..23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11295)
	; 23 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 11296)
	; 19 bytes
	ldy #251
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 11297)
	; 24 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11304)
	; 23 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11305)
	; 23 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 11307)
	; 23 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11308)
	; 23 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11310)
	; 23 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11311)
	; 25 bytes
	ldy #174
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 11312 || n = 11314)
	; 23..24 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 11313)
	; 25 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 11315)
	; 23 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 11316)
	; 25 bytes
	delay_n 155
	delay_n (n-155)
.elseif(n = 11317)
	; 24 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 11318)
	; 25 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11320)
	; 21 bytes
	ldy #231
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11321)
	; 24 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 11323)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11328)
	; 20 bytes
	ldy #241
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.endif
.endif
.else
.if((n >= 11329) && (n < 11368))
.if((n >= 11329) && (n < 11348))
; 11329 to 11347
.if(n = 11329)
	; 23 bytes
	ldy #192
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11330 || (n > 11331 && n < 11335) || n = 11343 || (n > 11344 && n < 11348))
	; 20..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 11331)
	; 22 bytes
	ldy #206
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11335)
	; 23 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 11336)
	; 24 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11337)
	; 24 bytes
	ldy #218
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 11338)
	; 24 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11339)
	; 24 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11340)
	; 23 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 11341)
	; 19 bytes
	ldy #252
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 11342)
	; 22 bytes
	ldy #200 ;hides 'iny'
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	dey
	Jne *-19
.elseif(n = 11344)
	; 21 bytes
	delay_n (n-3)
	ldy $A4
.endif
.else
; 11348 to 11367
.if(n = 11348 || n = 11354 || n = 11364)
	; 23..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11349)
	; 23 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11350)
	; 23 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 11351)
	; 23 bytes
	ldy #227
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11352)
	; 23 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11353)
	; 23 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11355)
	; 23 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 11356)
	; 24 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 11357)
	; 24 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11358)
	; 25 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 11359)
	; 24 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 11360)
	; 25 bytes
	delay_n 64
	delay_n (n-64)
.elseif(n = 11361)
	; 25 bytes
	delay_n 155
	delay_n (n-155)
.elseif(n = 11362)
	; 24 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 11363)
	; 25 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11365)
	; 24 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11366)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11367)
	; 20 bytes
	ldy #24 ;hides 'clc'
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-17
.endif
.endif
.else
; 11368 to 11406
.if(n = 11368)
	; 25 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 11369)
	; 21 bytes
	ldy #232
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11370 || n = 11372 || n = 11378 || n = 11389)
	; 21..23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11371 || n = 11373 || n = 11376 || n = 11377 || (n > 11378 && n < 11382) || n = 11388 || (n > 11389 && n < 11394) || n = 11395 || n = 11398 || n = 11400 || n = 11403 || n = 11405)
	; 20..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11374)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11375)
	; 20 bytes
	ldy #242
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11382)
	; 24 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 11383 || n = 11394)
	; 23..24 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11384 || n = 11385 || n = 11406)
	; 24..25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11386)
	; 19 bytes
	ldy #253
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 11387)
	; 24 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11396)
	; 21 bytes
	ldy #215
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11397)
	; 23 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11399)
	; 22 bytes
	ldy #56 ;hides 'sec'
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-19
.elseif(n = 11401)
	; 23 bytes
	ldy #228
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11402)
	; 24 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11404)
	; 24 bytes
	delay_n 153
	delay_n (n-153)
.endif
.endif
.endif
.else
.if((n >= 11407) && (n < 11485))
.if((n >= 11407) && (n < 11446))
; 11407 to 11445
.if(n = 11407)
	; 24 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 11408)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 11409 || n = 11414 || n = 11419 || n = 11420 || n = 11423 || n = 11424 || n = 11426 || n = 11427 || n = 11429 || n = 11430 || n = 11433 || (n > 11434 && n < 11439) || n = 11440 || n = 11443 || n = 11445)
	; 20..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11410)
	; 24 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11411)
	; 25 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11412)
	; 24 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11413)
	; 25 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 11415)
	; 25 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 11416)
	; 24 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 11417)
	; 24 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11418)
	; 21 bytes
	ldy #233
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11421 || n = 11428 || n = 11434 || n = 11444)
	; 21..24 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11422)
	; 20 bytes
	ldy #243
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11425)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11431)
	; 19 bytes
	ldy #254
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 11432)
	; 24 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11439)
	; 23 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11441)
	; 22 bytes
	ldy #208
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11442)
	; 23 bytes
	delay_n 11
	delay_n (n-11)
.endif
.else
; 11446 to 11484
.if(n = 11446 || n = 11451 || (n > 11452 && n < 11457) || n = 11460 || n = 11464 || n = 11471 || (n > 11472 && n < 11476) || n = 11478 || (n > 11479 && n < 11484))
	; 20..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11447)
	; 21 bytes
	ldy #216 ;hides 'cld'
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11448 || n = 11457)
	; 24..25 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11449)
	; 21 bytes
	ldy #216
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11450 || n = 11452 || n = 11468 || n = 11470 || n = 11472 || n = 11479)
	; 21..23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11458)
	; 23 bytes
	ldy #201
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11459)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11461)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11462)
	; 24 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11463)
	; 24 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 11465)
	; 20 bytes
	ldy #234 ;hides 'nop'
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11466)
	; 25 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11467)
	; 21 bytes
	ldy #234
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11469)
	; 20 bytes
	ldy #244
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11476)
	; 19 bytes
	ldy #255
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.elseif(n = 11477)
	; 24 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11484)
	; 23 bytes
	delay_n (n-8)
	delay_n 8
.endif
.endif
.else
.if((n >= 11485) && (n < 11524))
; 11485 to 11523
.if(n = 11485 || n = 11489 || n = 11490 || n = 11495 || n = 11498 || n = 11500 || n = 11503 || n = 11504 || n = 11507 || n = 11508 || n = 11518 || n = 11520 || n = 11522 || n = 11523)
	; 20..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11486)
	; 23 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11487)
	; 23 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11488)
	; 23 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11491 || n = 11499 || n = 11505 || n = 11509 || n = 11519)
	; 22..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11492)
	; 24 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11493)
	; 24 bytes
	ldy #221
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 11494)
	; 24 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 11496)
	; 22 bytes
	ldy #209
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11497)
	; 24 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 11501)
	; 23 bytes
	ldy #230
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11502)
	; 21 bytes
	ldy #217
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11506)
	; 23 bytes
	ldy #195
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11510)
	; 24 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11511)
	; 25 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 11512)
	; 24 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11513)
	; 25 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11514)
	; 24 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 11515)
	; 23 bytes
	ldy #202
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11516)
	; 20 bytes
	ldy #245
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11517)
	; 24 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 11521)
	; 19 bytes
	ldy #0
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-15
.endif
.else
; 11524 to 11562
.if(n = 11524 || n = 11530 || n = 11536 || n = 11554 || n = 11558)
	; 21..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11525 || n = 11526 || n = 11528 || n = 11529 || n = 11534 || n = 11535 || n = 11544 || n = 11549 || n = 11553 || n = 11556 || n = 11557 || (n > 11558 && n < 11562))
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11527)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11531)
	; 23 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11532 || n = 11538)
	; 23..25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11533)
	; 23 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11537)
	; 24 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11539)
	; 24 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 11540)
	; 25 bytes
	delay_n 64
	delay_n (n-64)
.elseif(n = 11541)
	; 25 bytes
	delay_n 155
	delay_n (n-155)
.elseif(n = 11542)
	; 24 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 11543)
	; 25 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 11545)
	; 24 bytes
	ldy #222
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 11546)
	; 25 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 11547)
	; 24 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11548)
	; 25 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 11550)
	; 25 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 11551)
	; 22 bytes
	ldy #210
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11552)
	; 24 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11555)
	; 21 bytes
	ldy #218
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11562)
	; 24 bytes
	delay_n 41
	delay_n (n-41)
.endif
.endif
.endif
.endif
.else
.if((n >= 11563) && (n < 11719))
.if((n >= 11563) && (n < 11641))
.if((n >= 11563) && (n < 11602))
; 11563 to 11601
.if(n = 11563)
	; 20 bytes
	ldy #246
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11564 || n = 11567 || n = 11569 || n = 11571 || n = 11574 || n = 11579 || n = 11580 || (n > 11581 && n < 11586) || n = 11589 || n = 11594 || n = 11599)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11565)
	; 21 bytes
	ldy #236
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11566 || n = 11568 || n = 11581)
	; 22..23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11570)
	; 23 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 11572)
	; 23 bytes
	ldy #203
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11573 || n = 11588)
	; 24..25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11575)
	; 24 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11576)
	; 24 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 11577)
	; 24 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 11578)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11586)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11587)
	; 24 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 11590)
	; 24 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11591)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 11592)
	; 24 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 11593)
	; 25 bytes
	ldy #184
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 11595)
	; 25 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 11596)
	; 25 bytes
	delay_n 120
	delay_n (n-120)
.elseif(n = 11597)
	; 24 bytes
	ldy #223
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 11598)
	; 24 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 11600)
	; 25 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 11601)
	; 23 bytes
	ldy #232
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.endif
.else
; 11602 to 11640
.if(n = 11602)
	; 24 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 11603 || n = 11612 || n = 11615 || n = 11616 || (n > 11617 && n < 11621) || n = 11623 || n = 11626 || n = 11628 || n = 11631 || (n > 11632 && n < 11636))
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11604 || n = 11609 || n = 11611 || n = 11613 || n = 11617 || n = 11632)
	; 22..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11605)
	; 24 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 11606)
	; 22 bytes
	ldy #211
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11607)
	; 24 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 11608)
	; 21 bytes
	ldy #219
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11610)
	; 20 bytes
	ldy #247
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11614)
	; 21 bytes
	ldy #237
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11621 || n = 11625 || n = 11640)
	; 24..25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11622)
	; 24 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 11624)
	; 23 bytes
	ldy #197
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11627)
	; 24 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11629)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11630)
	; 25 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 11636)
	; 25 bytes
	ldy #179
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 11637)
	; 24 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 11638)
	; 25 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 11639)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 11641) && (n < 11680))
; 11641 to 11679
.if(n = 11641 || n = 11675)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11642)
	; 24 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 11643)
	; 24 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 11644 || n = 11653 || n = 11659 || n = 11662 || n = 11665 || (n > 11666 && n < 11671) || n = 11679)
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11645)
	; 25 bytes
	delay_n 169
	delay_n (n-169)
.elseif(n = 11646)
	; 25 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11647)
	; 24 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 11648)
	; 25 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 11649)
	; 24 bytes
	ldy #224
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 11650)
	; 24 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 11651)
	; 23 bytes
	ldy #233
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11652)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 11654 || n = 11658 || n = 11660 || n = 11664 || n = 11666)
	; 21..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11655)
	; 19 bytes
	ldy #248 ;hides 'sed'
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11656)
	; 25 bytes
	ldy #185
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 11657)
	; 20 bytes
	ldy #248
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11661)
	; 21 bytes
	ldy #220
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11663)
	; 21 bytes
	ldy #238
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11671)
	; 24 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11672)
	; 24 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11673)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11674)
	; 24 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 11676)
	; 24 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 11677)
	; 24 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 11678)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.endif
.else
; 11680 to 11718
.if(n = 11680)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11681)
	; 24 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11682 || n = 11684 || n = 11685 || n = 11688 || n = 11694 || n = 11699 || n = 11703 || n = 11705 || n = 11706 || (n > 11707 && n < 11712))
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11683 || n = 11707 || n = 11715 || n = 11717)
	; 22..23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11686)
	; 23 bytes
	ldy #205
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11687)
	; 24 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 11689)
	; 24 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 11690)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11691 || n = 11693)
	; 24..25 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11692)
	; 24 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 11695)
	; 24 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 11696 || n = 11698)
	; 24..25 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 11697)
	; 24 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 11700)
	; 25 bytes
	delay_n 179
	delay_n (n-179)
.elseif(n = 11701)
	; 23 bytes
	ldy #234
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11702)
	; 24 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 11704)
	; 20 bytes
	ldy #249
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11712)
	; 21 bytes
	ldy #239
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11713)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 11714)
	; 21 bytes
	ldy #221
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11716)
	; 22 bytes
	ldy #213
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11718)
	; 23 bytes
	delay_n (n-4)
	delay_n 4
.endif
.endif
.endif
.else
.if((n >= 11719) && (n < 11797))
.if((n >= 11719) && (n < 11758))
; 11719 to 11757
.if(n = 11719 || n = 11720 || n = 11733 || (n > 11734 && n < 11739) || n = 11744 || n = 11745 || n = 11747 || n = 11753 || (n > 11754 && n < 11758))
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11721 || n = 11727)
	; 24..26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 11722)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11723)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11724)
	; 24 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 11725 || n = 11752)
	; 25..26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 11726 || n = 11728 || n = 11732)
	; 24..26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 11729)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 11730)
	; 25 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11731)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11734 || n = 11754)
	; 22..23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11739)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11740)
	; 23 bytes
	ldy #200 ;hides 'iny'
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	dey
	Jne *-20
.elseif(n = 11741)
	; 24 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 11742)
	; 23 bytes
	ldy #199
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11743)
	; 23 bytes
	ldy #206
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11746 || n = 11748)
	; 24..25 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11749)
	; 25 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 11750)
	; 25 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11751)
	; 20 bytes
	ldy #250
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.endif
.else
; 11758 to 11796
.if(n = 11758 || n = 11763 || n = 11765 || n = 11766 || n = 11768 || n = 11769 || n = 11772 || n = 11773 || n = 11775 || n = 11784 || (n > 11785 && n < 11790))
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11759 || n = 11790)
	; 24..25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11760)
	; 25 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 11761)
	; 21 bytes
	ldy #240
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11762 || n = 11793)
	; 24..25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11764 || n = 11770 || n = 11774 || n = 11785)
	; 23..24 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11767)
	; 21 bytes
	ldy #222
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11771)
	; 22 bytes
	ldy #214
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11776 || n = 11778)
	; 24..25 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 11777 || n = 11792)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11779 || n = 11794)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11780)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 11781 || n = 11783)
	; 24..25 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 11782)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11791)
	; 24 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 11795)
	; 25 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11796)
	; 24 bytes
	delay_n 141
	delay_n (n-141)
.endif
.endif
.else
.if((n >= 11797) && (n < 11836))
; 11797 to 11835
.if(n = 11797)
	; 25 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11798)
	; 20 bytes
	ldy #251
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11799)
	; 23 bytes
	ldy #56 ;hides 'sec'
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-20
.elseif(n = 11800 || (n > 11801 && n < 11805) || n = 11807 || n = 11812 || (n > 11813 && n < 11818) || n = 11822 || n = 11824 || n = 11825 || n = 11828 || n = 11830 || n = 11835)
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11801 || n = 11813 || n = 11823 || n = 11829 || n = 11834)
	; 22..24 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11805)
	; 24 bytes
	ldy #227
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 11806 || n = 11818)
	; 24..25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11808)
	; 24 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11809)
	; 24 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11810)
	; 21 bytes
	ldy #241
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11811)
	; 24 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 11819)
	; 25 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 11820)
	; 21 bytes
	ldy #223
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11821)
	; 24 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 11826)
	; 22 bytes
	ldy #215
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11827)
	; 25 bytes
	ldy #219
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 11831)
	; 21 bytes
	ldy #24 ;hides 'clc'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-18
.elseif(n = 11832)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11833)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.endif
.else
; 11836 to 11875
.if(n = 11836 || n = 11848 || n = 11854 || n = 11862)
	; 22..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 11836 && n < 11841) || n = 11847 || n = 11849 || n = 11850 || n = 11852 || n = 11853 || n = 11858 || n = 11861 || (n > 11862 && n < 11867) || n = 11875)
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11841)
	; 24 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 11842)
	; 25 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11843 || n = 11855 || n = 11869)
	; 24..25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11844)
	; 25 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 11845)
	; 20 bytes
	ldy #252
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11846)
	; 24 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 11851)
	; 23 bytes
	ldy #237
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11856 || n = 11870)
	; 24..25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11857)
	; 23 bytes
	ldy #208
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11859)
	; 21 bytes
	ldy #242
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11860)
	; 23 bytes
	ldy #201
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11867)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11868)
	; 26 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 11871)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11872)
	; 25 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 11873)
	; 21 bytes
	ldy #224
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11874)
	; 25 bytes
	delay_n 76
	delay_n (n-76)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 11876) && (n < 12188))
.if((n >= 11876) && (n < 12032))
.if((n >= 11876) && (n < 11954))
.if((n >= 11876) && (n < 11915))
; 11876 to 11914
.if(n = 11876 || n = 11882 || n = 11887 || n = 11895 || n = 11911)
	; 22..23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11877 || n = 11878 || n = 11880 || n = 11883 || n = 11885 || n = 11886 || (n > 11887 && n < 11892) || n = 11894 || (n > 11895 && n < 11900) || n = 11903 || n = 11905 || n = 11910 || n = 11912 || n = 11913)
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11879)
	; 21 bytes
	ldy #216 ;hides 'cld'
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11881)
	; 22 bytes
	ldy #216
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11884)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11892)
	; 20 bytes
	ldy #253
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11893)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 11900)
	; 24 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11901)
	; 23 bytes
	ldy #238
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11902)
	; 24 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11904)
	; 24 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11906)
	; 25 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 11907)
	; 25 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 11908)
	; 21 bytes
	ldy #243
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11909)
	; 24 bytes
	ldy #229
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 11914)
	; 23 bytes
	ldy #209
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.endif
.else
; 11915 to 11953
.if(n = 11915 || n = 11916 || n = 11921 || n = 11923 || n = 11928 || (n > 11929 && n < 11933) || n = 11937 || n = 11940 || n = 11941 || (n > 11942 && n < 11947) || n = 11952 || n = 11953)
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11917 || n = 11922 || n = 11929 || n = 11938 || n = 11942)
	; 22..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11918 || n = 11949)
	; 24..25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11919)
	; 23 bytes
	ldy #202
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11920)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11924)
	; 25 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 11925)
	; 25 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 11926)
	; 21 bytes
	ldy #225
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11927)
	; 25 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 11933)
	; 21 bytes
	ldy #234 ;hides 'nop'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11934)
	; 25 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 11935)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11936)
	; 22 bytes
	ldy #217
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 11939)
	; 20 bytes
	ldy #254
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11947)
	; 24 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11948)
	; 25 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 11950)
	; 24 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11951)
	; 23 bytes
	ldy #239
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.endif
.endif
.else
.if((n >= 11954) && (n < 11993))
; 11954 to 11992
.if(n = 11954 || n = 11960 || n = 11974 || n = 11982 || n = 11989)
	; 22..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11955 || n = 11959 || (n > 11960 && n < 11965) || n = 11973 || n = 11980 || n = 11981 || (n > 11982 && n < 11986) || n = 11988 || n = 11990 || n = 11992)
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11956)
	; 25 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 11957)
	; 21 bytes
	ldy #244
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11958)
	; 25 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 11965 || n = 11987)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11966)
	; 25 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 11967)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11968)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11969)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11970)
	; 25 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11971)
	; 23 bytes
	ldy #210
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11972)
	; 25 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 11975)
	; 25 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11976)
	; 25 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 11977)
	; 26 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 11978)
	; 23 bytes
	ldy #203
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 11979)
	; 21 bytes
	ldy #226
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 11986)
	; 20 bytes
	ldy #255
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 11991)
	; 22 bytes
	ldy #218
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.endif
.else
; 11993 to 12031
.if(n = 11993 || n = 11995 || n = 11999 || n = 12003 || n = 12005 || n = 12008 || (n > 12009 && n < 12013) || n = 12015 || n = 12020 || n = 12030)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11994 || n = 12004 || n = 12009 || n = 12031)
	; 23..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 11996 || n = 12016)
	; 24..25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11997 || n = 12017)
	; 24..25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11998)
	; 24 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12000)
	; 25 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 12001)
	; 23 bytes
	ldy #240
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12002)
	; 25 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12006)
	; 21 bytes
	ldy #245
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12007)
	; 25 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 12013)
	; 24 bytes
	ldy #231
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12014)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12018)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12019)
	; 25 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 12021)
	; 25 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 12022)
	; 25 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12023)
	; 25 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 12024)
	; 26 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 12025)
	; 25 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 12026)
	; 25 bytes
	ldy #185
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12027)
	; 25 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 12028)
	; 23 bytes
	ldy #211
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12029)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.endif
.endif
.endif
.else
.if((n >= 12032) && (n < 12110))
.if((n >= 12032) && (n < 12071))
; 12032 to 12070
.if(n = 12032)
	; 21 bytes
	ldy #227
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12033)
	; 20 bytes
	ldy #0
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-16
.elseif(n = 12034 || n = 12035 || n = 12038 || n = 12039 || n = 12041 || n = 12042 || n = 12048 || n = 12050 || n = 12053 || n = 12056 || n = 12057 || (n > 12058 && n < 12063) || n = 12067)
	; 21..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 12036 || n = 12040 || n = 12049 || n = 12054 || n = 12058 || n = 12066)
	; 22..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12037)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12043 || n = 12047)
	; 24..25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12044)
	; 24 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12045)
	; 24 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12046)
	; 22 bytes
	ldy #219
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12051)
	; 23 bytes
	ldy #241
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12052)
	; 25 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 12055)
	; 21 bytes
	ldy #246
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12063)
	; 23 bytes
	ldy #24 ;hides 'clc'
	.repeat 9
	 rol $26
	.endrepeat
	iny
	Jne *-20
.elseif(n = 12064)
	; 25 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12065)
	; 24 bytes
	ldy #232
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12068)
	; 25 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 12069)
	; 25 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12070)
	; 25 bytes
	delay_n 131
	delay_n (n-131)
.endif
.else
; 12071 to 12109
.if(n = 12071)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12072)
	; 25 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 12073)
	; 25 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 12074)
	; 25 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 12075)
	; 25 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12076)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 12077)
	; 25 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12078)
	; 25 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 12079)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12080)
	; 25 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12081 || n = 12087 || n = 12089 || n = 12090 || (n > 12091 && n < 12095) || n = 12098 || n = 12103 || n = 12105 || n = 12106 || n = 12108 || n = 12109)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12082 || n = 12086)
	; 24..25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12083)
	; 25 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12084)
	; 25 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 12085)
	; 21 bytes
	ldy #228
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12088)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12091 || n = 12099 || n = 12107)
	; 23..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12095)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12096)
	; 23 bytes
	ldy #205
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12097)
	; 25 bytes
	ldy #224
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12100)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12101)
	; 22 bytes
	ldy #220
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12102)
	; 25 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 12104)
	; 21 bytes
	ldy #247
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.endif
.endif
.else
.if((n >= 12110) && (n < 12149))
.if((n >= 12110) && (n < 12129))
; 12110 to 12128
.if(n = 12110 || n = 12111 || n = 12119)
	; 24..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12112)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12113)
	; 25 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 12114)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12115)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12116)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12117)
	; 24 bytes
	ldy #233
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12118)
	; 26 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 12120)
	; 25 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 12121)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 12122)
	; 25 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12123)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 12124)
	; 25 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12125)
	; 25 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 12126)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 12127)
	; 25 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12128)
	; 26 bytes
	delay_n 171
	delay_n (n-171)
.endif
.else
; 12129 to 12148
.if(n = 12129)
	; 25 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12130)
	; 25 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12131)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 12132)
	; 25 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 12133)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 12134)
	; 25 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 12135)
	; 25 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 12136)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12137)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12138)
	; 21 bytes
	ldy #229
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12139)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12140 || n = 12141 || (n > 12142 && n < 12147))
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12142)
	; 23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12147)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12148)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 12149) && (n < 12168))
; 12149 to 12167
.if(n = 12149 || n = 12161)
	; 24..25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12150 || n = 12163 || n = 12165)
	; 24..25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12151)
	; 20 bytes
	ldy #248 ;hides 'sed'
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12152)
	; 25 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 12153)
	; 21 bytes
	ldy #248
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12154 || n = 12159)
	; 22..24 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12155 || n = 12157 || n = 12158 || n = 12160)
	; 22..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 12156)
	; 22 bytes
	ldy #221
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12162 || n = 12164)
	; 24..25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12166)
	; 26 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 12167)
	; 23 bytes
	ldy #234 ;hides 'nop'
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.endif
.else
; 12168 to 12187
.if(n = 12168)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 12169)
	; 24 bytes
	ldy #234
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12170)
	; 25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12171 || n = 12173)
	; 25..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 12172)
	; 25 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 12174 || n = 12178)
	; 25..26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12175)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 12176)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 12177)
	; 25 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12179 || n = 12183)
	; 25..26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 12180)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 12181)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 12182)
	; 25 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 12184)
	; 25 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12185)
	; 25 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 12186)
	; 25 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 12187)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 12188) && (n < 12344))
.if((n >= 12188) && (n < 12266))
.if((n >= 12188) && (n < 12227))
; 12188 to 12226
.if(n = 12188 || n = 12200)
	; 24..25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12189)
	; 25 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 12190)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12191)
	; 21 bytes
	ldy #230
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12192 || n = 12193 || (n > 12194 && n < 12199) || n = 12203 || n = 12204 || (n > 12205 && n < 12210) || n = 12213 || n = 12215 || n = 12216 || n = 12218 || n = 12223)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12194 || n = 12205 || n = 12217)
	; 23..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12199)
	; 23 bytes
	ldy #214
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12201)
	; 23 bytes
	ldy #244
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12202)
	; 21 bytes
	ldy #249
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12210)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12211)
	; 22 bytes
	ldy #222
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12212)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12214)
	; 23 bytes
	ldy #207
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12219)
	; 25 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 12220)
	; 25 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 12221)
	; 24 bytes
	ldy #235
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12222)
	; 25 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 12224)
	; 25 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12225)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 12226)
	; 26 bytes
	delay_n 171
	delay_n (n-171)
.endif
.else
; 12227 to 12265
.if(n = 12227)
	; 25 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 12228)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12229)
	; 25 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 12230)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 12231)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 12232 || n = 12234)
	; 25..26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12233)
	; 26 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12235)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 12236)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 12237)
	; 25 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 12238)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12239 || n = 12240)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12241)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12242)
	; 25 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12243 || n = 12245 || n = 12246 || (n > 12247 && n < 12251) || n = 12253 || n = 12255 || n = 12257 || n = 12258 || n = 12260 || n = 12264)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12244)
	; 21 bytes
	ldy #231
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12247 || n = 12254)
	; 23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12251)
	; 21 bytes
	ldy #250
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12252)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12256)
	; 23 bytes
	ldy #215
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12259)
	; 25 bytes
	ldy #227
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12261)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12262)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12263)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12265)
	; 26 bytes
	delay_n 161
	delay_n (n-161)
.endif
.endif
.else
.if((n >= 12266) && (n < 12305))
; 12266 to 12304
.if(n = 12266)
	; 22 bytes
	ldy #223
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12267)
	; 25 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 12268 || (n > 12269 && n < 12273) || n = 12275 || n = 12277 || n = 12294 || n = 12296 || n = 12299 || n = 12302 || n = 12304)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12269 || n = 12276 || n = 12298 || n = 12303)
	; 23..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12273)
	; 23 bytes
	ldy #208
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12274)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 12278)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 12279)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 12280)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 12281)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12282 || n = 12284)
	; 25..26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 12283)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12285)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 12286)
	; 25 bytes
	ldy #195
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12287 || n = 12289)
	; 25..26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12288)
	; 26 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 12290 || n = 12293)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12291)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 12292)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12295)
	; 21 bytes
	ldy #24 ;hides 'clc'
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-18
.elseif(n = 12297)
	; 21 bytes
	ldy #232
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12300)
	; 21 bytes
	ldy #251
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12301)
	; 23 bytes
	ldy #246
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.endif
.else
; 12305 to 12343
.if(n = 12305 || n = 12306 || n = 12315 || n = 12317 || n = 12323 || n = 12326 || n = 12327 || n = 12334 || n = 12336)
	; 23..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12307 || n = 12310)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12308)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12309 || n = 12312)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12311)
	; 22 bytes
	ldy #216 ;hides 'cld'
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12313)
	; 23 bytes
	ldy #216
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12314 || n = 12316 || n = 12324 || n = 12328 || n = 12335)
	; 24..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12318)
	; 25 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 12319)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 12320)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 12321)
	; 22 bytes
	ldy #224
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12322)
	; 25 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 12325)
	; 24 bytes
	ldy #237
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12329)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 12330 || n = 12342)
	; 25..26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12331)
	; 26 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12332)
	; 23 bytes
	ldy #209
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12333)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 12337 || n = 12339)
	; 25..26 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 12338)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12340)
	; 26 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12341)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12343)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.endif
.endif
.endif
.else
.if((n >= 12344) && (n < 12422))
.if((n >= 12344) && (n < 12383))
; 12344 to 12382
.if(n = 12344)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12345 || n = 12347 || n = 12348 || n = 12351 || n = 12352 || (n > 12353 && n < 12358) || n = 12369 || n = 12372 || n = 12374 || n = 12378 || (n > 12379 && n < 12383))
	; 22..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 12346 || n = 12353 || n = 12373 || n = 12379)
	; 23..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12349)
	; 21 bytes
	ldy #252
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12350)
	; 21 bytes
	ldy #233
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12358)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12359 || n = 12360)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12361 || n = 12362)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12363)
	; 26 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 12364)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 12365)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12366)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 12367)
	; 25 bytes
	ldy #229
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12368)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 12370)
	; 23 bytes
	ldy #217
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12371)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 12375)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12376)
	; 22 bytes
	ldy #225
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12377)
	; 24 bytes
	ldy #238
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.endif
.else
; 12383 to 12421
.if(n = 12383)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 12384)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12385)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12386 || n = 12393 || n = 12395 || n = 12396 || n = 12399 || n = 12400 || n = 12402 || n = 12405 || (n > 12406 && n < 12410))
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12387)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12388)
	; 26 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 12389)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12390)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 12391)
	; 23 bytes
	ldy #210
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12392)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12394)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12397 || n = 12404 || n = 12406)
	; 23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12398)
	; 21 bytes
	ldy #253
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12401)
	; 21 bytes
	ldy #234 ;hides 'nop'
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12403)
	; 21 bytes
	ldy #234
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12410 || n = 12415)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12411 || n = 12413)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12412)
	; 25 bytes
	ldy #197
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12414)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12416)
	; 25 bytes
	ldy #191
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12417)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 12418)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 12419)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 12420)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 12421)
	; 25 bytes
	ldy #230
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.endif
.endif
.else
.if((n >= 12422) && (n < 12461))
; 12422 to 12460
.if(n = 12422)
	; 26 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 12423 || n = 12433 || (n > 12434 && n < 12438) || n = 12449 || n = 12452 || n = 12453 || n = 12455 || n = 12458 || n = 12460)
	; 22..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 12424)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12425)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 12426)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 12427)
	; 23 bytes
	ldy #218
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12428)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12429)
	; 24 bytes
	ldy #239
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12430 || n = 12432 || n = 12434 || n = 12448 || n = 12450 || n = 12454 || n = 12459)
	; 23..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12431)
	; 22 bytes
	ldy #226
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12438 || n = 12446)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 12439)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 12440)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12441)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12442)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12443)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12444)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12445)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12447)
	; 21 bytes
	ldy #254
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12451)
	; 23 bytes
	ldy #249
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12456)
	; 21 bytes
	ldy #235
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12457)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
.if((n >= 12461) && (n < 12481))
; 12461 to 12480
.if((n > 12460 && n < 12464) || n = 12477)
	; 24..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 12464)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12465)
	; 26 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 12466)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12467)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12468)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12469)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 12470)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 12471)
	; 26 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 12472)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12473)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12474)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 12475)
	; 25 bytes
	ldy #231
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12476)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 12478)
	; 26 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12479)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12480)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.endif
.else
; 12481 to 12500
.if(n = 12481)
	; 24 bytes
	ldy #240
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12482)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12483 || n = 12488 || (n > 12489 && n < 12493) || n = 12498 || n = 12500)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12484)
	; 23 bytes
	ldy #219
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12485)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12486)
	; 22 bytes
	ldy #227
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12487 || n = 12489 || n = 12499)
	; 23..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12493)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12494)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12495)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 12496)
	; 21 bytes
	ldy #255
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12497)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 12501) && (n < 13751))
.if((n >= 12501) && (n < 13126))
.if((n >= 12501) && (n < 12813))
.if((n >= 12501) && (n < 12657))
.if((n >= 12501) && (n < 12579))
.if((n >= 12501) && (n < 12540))
.if((n >= 12501) && (n < 12520))
; 12501 to 12519
.if(n = 12501)
	; 23 bytes
	ldy #250
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12502 || n = 12503 || n = 12505 || n = 12511 || (n > 12512 && n < 12517))
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12504 || n = 12512)
	; 23..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12506)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12507)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12508)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12509)
	; 21 bytes
	ldy #236
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12510)
	; 26 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 12517)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12518)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 12519)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 12520 to 12539
.if(n = 12520)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12521)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12522)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12523)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 12524)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 12525)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 12526 || n = 12528)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12527)
	; 24 bytes
	ldy #24 ;hides 'clc'
	nop
	.repeat 9
	 rol $26
	.endrepeat
	iny
	Jne *-21
.elseif(n = 12529)
	; 25 bytes
	ldy #232
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12530)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 12531 || n = 12535)
	; 25..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 12532)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12533)
	; 24 bytes
	ldy #241
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12534)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12536)
	; 24 bytes
	ldy #200 ;hides 'iny'
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	dey
	Jne *-21
.elseif(n = 12537)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 12538)
	; 25 bytes
	ldy #199
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12539)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.endif
.endif
.else
; 12540 to 12578
.if(n = 12540)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12541)
	; 22 bytes
	ldy #228
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12542)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12543 || n = 12549 || n = 12552 || n = 12553 || n = 12564 || n = 12566 || n = 12567 || n = 12569 || n = 12570)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12544 || n = 12548 || n = 12550 || n = 12554 || n = 12565 || n = 12571)
	; 23..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12545)
	; 21 bytes
	ldy #0
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12546)
	; 25 bytes
	ldy #193
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12547)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12551)
	; 23 bytes
	ldy #251
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12555 || n = 12572)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12556 || n = 12558 || n = 12573)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12557 || n = 12559 || n = 12574)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12560)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 12561)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12562)
	; 21 bytes
	ldy #237
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12563)
	; 26 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 12568)
	; 23 bytes
	ldy #213
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12575)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 12576)
	; 26 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12577)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12578)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.endif
.endif
.else
.if((n >= 12579) && (n < 12618))
.if((n >= 12579) && (n < 12598))
; 12579 to 12597
.if(n = 12579)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 12580 || n = 12584)
	; 26 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 12581)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12582 || n = 12595)
	; 26 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 12583)
	; 25 bytes
	ldy #233
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12585)
	; 24 bytes
	ldy #242
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12586)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 12587)
	; 25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12588)
	; 26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12589)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12590)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 12591)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12592)
	; 26 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12593)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 12594)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12596)
	; 22 bytes
	ldy #229
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12597)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.endif
.else
; 12598 to 12617
.if(n = 12598)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12599)
	; 24 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12600 || (n > 12601 && n < 12606) || n = 12617)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12601)
	; 23 bytes
	ldy #252
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12606)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12607)
	; 26 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 12608)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12609)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12610)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12611)
	; 25 bytes
	ldy #194
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12612)
	; 26 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 12613)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 12614)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12615)
	; 21 bytes
	ldy #238
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12616)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.endif
.endif
.else
; 12618 to 12656
.if(n = 12618 || n = 12630 || n = 12640 || n = 12650 || n = 12652)
	; 23..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 12618 && n < 12623) || n = 12629 || n = 12631 || n = 12639 || n = 12651 || n = 12653 || n = 12654 || n = 12656)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12623)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12624)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12625)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12626)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12627)
	; 23 bytes
	ldy #214
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12628)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12632 || n = 12645)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12633)
	; 26 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 12634)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12635)
	; 24 bytes
	ldy #234 ;hides 'nop'
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12636)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12637)
	; 24 bytes
	ldy #243
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12638)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12641)
	; 26 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12642)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 12643)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 12644)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12646 || n = 12648)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 12647)
	; 21 bytes
	ldy #248 ;hides 'sed'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12649)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12655)
	; 23 bytes
	ldy #222
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.endif
.endif
.endif
.else
.if((n >= 12657) && (n < 12735))
.if((n >= 12657) && (n < 12696))
.if((n >= 12657) && (n < 12676))
; 12657 to 12675
.if(n = 12657 || n = 12670 || (n > 12671 && n < 12676))
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12658 || n = 12671)
	; 23..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12659)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12660)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12661)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12662)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 12663)
	; 26 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 12664)
	; 25 bytes
	ldy #201
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12665)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12666)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 12667)
	; 26 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 12668)
	; 21 bytes
	ldy #239
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12669)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.endif
.else
; 12676 to 12695
.if(n = 12676)
	; 25 bytes
	ldy #195
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12677)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 12678)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12679)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12680)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12681 || n = 12683)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12682)
	; 26 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 12684)
	; 26 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 12685)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12686)
	; 23 bytes
	ldy #215
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12687)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12688 || n = 12690)
	; 24..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12689)
	; 24 bytes
	ldy #244
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12691)
	; 25 bytes
	ldy #235
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12692)
	; 26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12693)
	; 26 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 12694)
	; 26 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12695)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.else
; 12696 to 12734
.if(n = 12696 || n = 12698 || n = 12717)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12697)
	; 27 bytes
	ldy #184
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 12699)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 12700)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12701)
	; 23 bytes
	ldy #254
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12702 || n = 12704 || n = 12705 || n = 12707 || n = 12708 || n = 12710 || n = 12711 || n = 12714 || n = 12716 || n = 12723 || (n > 12724 && n < 12729))
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12703 || n = 12709 || n = 12715 || n = 12724)
	; 23..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12706)
	; 22 bytes
	ldy #231
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12712)
	; 23 bytes
	ldy #223
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12713)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 12718)
	; 26 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 12719)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 12720)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 12721)
	; 21 bytes
	ldy #240
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12722)
	; 27 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12729)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12730)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12731)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12732)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12733)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12734)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.endif
.endif
.else
.if((n >= 12735) && (n < 12774))
; 12735 to 12773
.if(n = 12735)
	; 26 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 12736 || n = 12738)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12737)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12739)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12740)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12741)
	; 24 bytes
	ldy #245
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12742)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 12743)
	; 23 bytes
	ldy #216 ;hides 'cld'
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12744 || n = 12746 || n = 12748 || n = 12754 || n = 12762 || n = 12764 || n = 12772)
	; 23..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12745)
	; 23 bytes
	ldy #216
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12747 || n = 12752 || n = 12753 || (n > 12754 && n < 12759) || n = 12763 || (n > 12764 && n < 12769) || n = 12771 || n = 12773)
	; 22..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 12749 || n = 12770)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12750)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12751)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12759)
	; 21 bytes
	ldy #24 ;hides 'clc'
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-18
.elseif(n = 12760)
	; 26 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 12761)
	; 22 bytes
	ldy #232
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12769)
	; 23 bytes
	ldy #224
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.endif
.else
; 12774 to 12812
.if(n = 12774)
	; 21 bytes
	ldy #241
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12775)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 12776 || (n > 12777 && n < 12782) || n = 12783 || n = 12788 || n = 12795 || n = 12797 || n = 12803 || n = 12804 || (n > 12805 && n < 12810))
	; 22..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 12777 || n = 12796 || n = 12805)
	; 23..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12782 || n = 12810)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12784 || n = 12812)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12785)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12786)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12787)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12789)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12790)
	; 25 bytes
	ldy #203
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12791)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12792)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 12793)
	; 24 bytes
	ldy #246
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12794)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 12798)
	; 26 bytes
	ldy #191
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 12799)
	; 25 bytes
	ldy #237
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12800)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12801)
	; 23 bytes
	ldy #0
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12802)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12811)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.endif
.endif
.endif
.endif
.else
.if((n >= 12813) && (n < 12969))
.if((n >= 12813) && (n < 12891))
.if((n >= 12813) && (n < 12852))
; 12813 to 12851
.if(n = 12813 || n = 12818 || (n > 12819 && n < 12823) || n = 12828 || n = 12829 || (n > 12830 && n < 12835) || n = 12847 || n = 12849)
	; 22..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 12814 || n = 12839)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12815)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 12816)
	; 22 bytes
	ldy #233
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12817)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 12819 || n = 12830 || n = 12848)
	; 23..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12823 || n = 12851)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12824)
	; 26 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 12825)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 12826)
	; 23 bytes
	ldy #225
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12827)
	; 21 bytes
	ldy #242
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12835)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12836)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12837)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12838)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12840)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12841)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12842)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12843)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 12844)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 12845)
	; 24 bytes
	ldy #247
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12846)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 12850)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.endif
.else
; 12852 to 12890
.if(n = 12852)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 12853)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12854)
	; 26 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 12855 || (n > 12856 && n < 12861) || n = 12865 || n = 12867 || n = 12873 || (n > 12874 && n < 12878) || n = 12882 || (n > 12883 && n < 12888))
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12856 || n = 12866 || n = 12872 || n = 12874 || n = 12883)
	; 23..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12861 || n = 12888)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12862)
	; 26 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 12863)
	; 23 bytes
	ldy #218
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12864)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12868)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 12869)
	; 21 bytes
	ldy #234 ;hides 'nop'
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12870)
	; 26 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12871)
	; 22 bytes
	ldy #234
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12878)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 12879 || n = 12890)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12880)
	; 21 bytes
	ldy #243
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12881)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12889)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.endif
.endif
.else
.if((n >= 12891) && (n < 12930))
; 12891 to 12929
.if(n = 12891 || n = 12915)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12892)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12893)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12894 || n = 12921)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 12895)
	; 23 bytes
	ldy #248 ;hides 'sed'
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12896)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 12897)
	; 24 bytes
	ldy #248
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12898 || n = 12900 || n = 12907 || n = 12925 || n = 12929)
	; 23..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12899 || n = 12901 || n = 12906 || (n > 12907 && n < 12912) || n = 12924 || n = 12928)
	; 22..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 12902 || n = 12918)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12903)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 12904)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12905)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 12912)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12913)
	; 26 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 12914)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12916)
	; 25 bytes
	ldy #205
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12917)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 12919)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 12920)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12922)
	; 23 bytes
	ldy #219
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12923)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 12926)
	; 22 bytes
	ldy #235
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12927)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.endif
.else
; 12930 to 12968
.if((n > 12929 && n < 12933) || n = 12935 || (n > 12936 && n < 12940) || n = 12942 || n = 12944 || n = 12951 || n = 12957 || (n > 12958 && n < 12963))
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12933)
	; 21 bytes
	ldy #244
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12934)
	; 25 bytes
	ldy #200 ;hides 'iny'
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	dey
	Jne *-22
.elseif(n = 12936 || n = 12952 || n = 12958)
	; 23..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12940)
	; 23 bytes
	ldy #227
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12941 || n = 12963)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12943 || n = 12965)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12945 || n = 12967)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12946)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 12947)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 12948)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 12949)
	; 24 bytes
	ldy #249
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12950)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12953)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12954)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 12955)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12956)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 12964)
	; 26 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 12966)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12968)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.endif
.endif
.endif
.else
.if((n >= 12969) && (n < 13047))
.if((n >= 12969) && (n < 13008))
; 12969 to 13007
.if(n = 12969)
	; 26 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 12970)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 12971 || n = 13002)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12972 || n = 12983 || n = 12985 || n = 12987 || n = 12988 || (n > 12989 && n < 12994) || n = 12999 || n = 13003 || n = 13005)
	; 22..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 12973)
	; 26 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 12974)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 12975)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 12976 || n = 13007)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 12977)
	; 27 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 12978)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 12979)
	; 25 bytes
	ldy #206
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 12980)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 12981)
	; 22 bytes
	ldy #236
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 12982 || n = 13004)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 12984 || n = 12989 || n = 13000)
	; 23..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 12986)
	; 21 bytes
	ldy #245
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 12994)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 12995)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12996)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12997)
	; 23 bytes
	ldy #228
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 12998)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13001)
	; 24 bytes
	ldy #250
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13006)
	; 21 bytes
	ldy #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.endif
.else
.if((n >= 13008) && (n < 13027))
; 13008 to 13026
.if(n = 13008 || (n > 13009 && n < 13014))
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 13009)
	; 23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13014)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13015)
	; 25 bytes
	ldy #241
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13016)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13017)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13018)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13019)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 13020)
	; 26 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 13021)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 13022)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13023)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 13024)
	; 26 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 13025)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 13026)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.endif
.else
; 13027 to 13046
.if(n = 13027)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 13028)
	; 27 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 13029)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 13030)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 13031)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 13032)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13033)
	; 26 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 13034)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 13035)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13036)
	; 22 bytes
	ldy #237
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13037)
	; 26 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13038 || n = 13041 || (n > 13042 && n < 13047))
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 13039)
	; 21 bytes
	ldy #246
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 13040)
	; 23 bytes
	ldy #221
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13042)
	; 23 bytes
	delay_n (n-3)
	ldy $A4
.endif
.endif
.endif
.else
.if((n >= 13047) && (n < 13086))
.if((n >= 13047) && (n < 13066))
; 13047 to 13065
.if(n = 13047 || n = 13065)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13048 || n = 13051)
	; 25..26 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13049)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13050)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13052)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13053)
	; 24 bytes
	ldy #251
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13054)
	; 23 bytes
	ldy #229
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13055)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13056 || n = 13058 || n = 13059 || (n > 13060 && n < 13065))
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 13057)
	; 21 bytes
	ldy #0
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 13060)
	; 23 bytes
	delay_n (n-3)
	ldy $A4
.endif
.else
; 13066 to 13085
.if(n = 13066)
	; 25 bytes
	ldy #201
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13067)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13068)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13069)
	; 25 bytes
	ldy #242
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13070)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 13071)
	; 26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13072)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 13073)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13074)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13075)
	; 26 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 13076)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 13077)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 13078)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 13079)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 13080)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 13081)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 13082)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 13083)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13084)
	; 26 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13085)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.endif
.endif
.else
; 13086 to 13125
.if(n = 13086)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 13087 || n = 13120)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13088 || n = 13106)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13089)
	; 26 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13090 || n = 13119)
	; 26 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 13091)
	; 22 bytes
	ldy #238
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13092)
	; 21 bytes
	ldy #247
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 13093 || n = 13094 || (n > 13095 && n < 13099) || n = 13101 || n = 13107 || n = 13109 || n = 13113 || n = 13115 || n = 13118 || n = 13125)
	; 22..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13095 || n = 13108 || n = 13114)
	; 23..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13099)
	; 23 bytes
	ldy #222
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13100)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13102)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13103)
	; 25 bytes
	ldy #234 ;hides 'nop'
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13104)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13105)
	; 24 bytes
	ldy #252
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13110)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 13111)
	; 23 bytes
	ldy #230
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13112)
	; 26 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13116)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13117)
	; 26 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 13121)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 13122)
	; 26 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 13123)
	; 25 bytes
	ldy #243
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13124)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 13126) && (n < 13438))
.if((n >= 13126) && (n < 13282))
.if((n >= 13126) && (n < 13204))
.if((n >= 13126) && (n < 13165))
.if((n >= 13126) && (n < 13145))
; 13126 to 13144
.if(n = 13126)
	; 26 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 13127)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 13128)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 13129)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 13130)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13131)
	; 25 bytes
	ldy #202
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13132)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 13133)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 13134)
	; 27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13135)
	; 26 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13136)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 13137)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 13138)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13139)
	; 26 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 13140)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 13141)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13142)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 13143)
	; 21 bytes
	ldy #248 ;hides 'sed'
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13144)
	; 27 bytes
	delay_n (n-2)
	nop
.endif
.else
; 13145 to 13164
.if(n = 13145)
	; 21 bytes
	ldy #248
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 13146)
	; 22 bytes
	ldy #239
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13147 || (n > 13148 && n < 13153) || n = 13159 || n = 13160 || n = 13162 || n = 13164)
	; 22..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13148 || n = 13161)
	; 23..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13153)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13154 || n = 13156)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13155)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13157)
	; 24 bytes
	ldy #253
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13158)
	; 23 bytes
	ldy #223
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13163)
	; 26 bytes
	delay_n 106
	delay_n (n-106)
.endif
.endif
.else
.if((n >= 13165) && (n < 13184))
; 13165 to 13183
.if(n = 13165 || n = 13183)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 13166)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 13167)
	; 26 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 13168)
	; 23 bytes
	ldy #231
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13169)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13170 || n = 13172 || n = 13174 || n = 13179)
	; 24..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13171 || n = 13178)
	; 25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13173)
	; 26 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 13175)
	; 23 bytes
	ldy #216 ;hides 'cld'
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13176)
	; 26 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13177)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13180)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13181)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13182)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.endif
.else
; 13184 to 13203
.if(n = 13184 || n = 13186)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 13185)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 13187)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 13188)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 13189 || n = 13191)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13190)
	; 26 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13192 || n = 13194)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13193)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 13195)
	; 26 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13196)
	; 25 bytes
	ldy #203
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13197)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13198)
	; 21 bytes
	ldy #249
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 13199)
	; 26 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 13200 || n = 13202 || n = 13203)
	; 22..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 13201)
	; 22 bytes
	ldy #240
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.endif
.endif
.endif
.else
.if((n >= 13204) && (n < 13243))
; 13204 to 13242
.if(n = 13204 || n = 13212 || n = 13220 || n = 13226 || n = 13228)
	; 24..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 13204 && n < 13208) || n = 13211 || n = 13213 || n = 13219 || n = 13221 || n = 13222 || n = 13227 || n = 13229 || n = 13233 || n = 13240 || n = 13242)
	; 24..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13208)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13209)
	; 24 bytes
	ldy #254
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13210)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13214)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13215)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 13216)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 13217)
	; 23 bytes
	ldy #224
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13218)
	; 26 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 13223)
	; 22 bytes
	ldy #24 ;hides 'clc'
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-19
.elseif(n = 13224)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13225)
	; 23 bytes
	ldy #232
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13230)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13231)
	; 25 bytes
	ldy #245
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13232)
	; 26 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13234 || n = 13237)
	; 26..27 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13235)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 13236)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13238)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13239)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 13241)
	; 26 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 13243 to 13281
.if(n = 13243)
	; 26 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 13244)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13245)
	; 26 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 13246)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 13247 || n = 13250)
	; 25..26 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13248)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13249 || n = 13252)
	; 26..27 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 13251)
	; 21 bytes
	ldy #250
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 13253 || n = 13255 || n = 13257 || n = 13258 || n = 13260 || n = 13262 || n = 13263 || n = 13265 || n = 13270 || n = 13275 || n = 13278 || n = 13280)
	; 22..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13254 || n = 13259 || n = 13264 || n = 13279)
	; 23..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13256)
	; 22 bytes
	ldy #241
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13261)
	; 24 bytes
	ldy #255
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13266)
	; 26 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13267)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13268)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 13269)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 13271)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 13272)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 13273)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13274)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 13276)
	; 23 bytes
	ldy #225
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13277)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13281)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.endif
.endif
.endif
.else
.if((n >= 13282) && (n < 13360))
.if((n >= 13282) && (n < 13321))
; 13282 to 13320
.if(n = 13282)
	; 23 bytes
	ldy #233
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13283)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13284 || n = 13286 || n = 13287 || n = 13290 || n = 13293 || n = 13296 || n = 13301 || n = 13303 || n = 13306 || (n > 13307 && n < 13311) || n = 13313 || n = 13315 || n = 13317 || n = 13318)
	; 22..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13285)
	; 25 bytes
	ldy #246
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13288)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 13289)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13291)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 13292)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 13294)
	; 25 bytes
	ldy #211
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13295)
	; 27 bytes
	delay_n 44
	delay_n (n-44)
.elseif(n = 13297)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13298)
	; 26 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 13299)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13300 || n = 13305)
	; 25..26 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13302)
	; 26 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 13304)
	; 21 bytes
	ldy #251
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 13307 || n = 13314)
	; 23..24 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13311)
	; 22 bytes
	ldy #242
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13312)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13316)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13319)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 13320)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.endif
.else
.if((n >= 13321) && (n < 13340))
; 13321 to 13339
.if(n = 13321)
	; 26 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13322)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 13323)
	; 26 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13324)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 13325)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 13326)
	; 25 bytes
	ldy #205
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13327)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 13328)
	; 26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13329)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13330)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13331)
	; 26 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 13332)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13333)
	; 27 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 13334)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 13335)
	; 23 bytes
	ldy #226
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13336)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13337)
	; 22 bytes
	ldy #234 ;hides 'nop'
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13338)
	; 25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13339)
	; 23 bytes
	ldy #234
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.endif
.else
; 13340 to 13359
.if(n = 13340 || n = 13342)
	; 24..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13341 || n = 13343 || n = 13346 || n = 13356 || n = 13359)
	; 22..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13344)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 13345)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 13347)
	; 26 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13348)
	; 26 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13349)
	; 26 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13350)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13351)
	; 26 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 13352)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 13353)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13354)
	; 26 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13355)
	; 26 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 13357)
	; 21 bytes
	ldy #252
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 13358)
	; 27 bytes
	delay_n 21
	delay_n (n-21)
.endif
.endif
.endif
.else
.if((n >= 13360) && (n < 13399))
.if((n >= 13360) && (n < 13379))
; 13360 to 13378
.if(n = 13360 || n = 13369)
	; 23..24 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 13360 && n < 13365) || n = 13368 || (n > 13369 && n < 13373))
	; 23..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 13365)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13366)
	; 22 bytes
	ldy #243
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13367 || n = 13376)
	; 25..26 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13373)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13374)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 13375)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 13377)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 13378)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.endif
.else
; 13379 to 13398
.if(n = 13379)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 13380)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 13381)
	; 27 bytes
	ldy #223
	delay_n 10
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 13382)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 13383)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13384)
	; 26 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 13385)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13386)
	; 26 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13387)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 13388)
	; 26 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13389)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13390)
	; 26 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 13391)
	; 24 bytes
	ldy #248 ;hides 'sed'
	nop
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13392)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13393)
	; 25 bytes
	ldy #248
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13394)
	; 23 bytes
	ldy #227
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13395 || n = 13398)
	; 24..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13396)
	; 23 bytes
	ldy #235
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13397)
	; 25 bytes
	delay_n (n-3)
	ldy $A4
.endif
.endif
.else
; 13399 to 13437
.if(n = 13399 || n = 13413 || n = 13424)
	; 23..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13400 || n = 13409 || n = 13412 || (n > 13413 && n < 13418) || n = 13419 || n = 13423 || (n > 13424 && n < 13428) || n = 13434)
	; 22..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13401)
	; 26 bytes
	ldy #200
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13402)
	; 26 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13403)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13404)
	; 26 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 13405)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 13406)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13407)
	; 26 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13408)
	; 26 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 13410)
	; 21 bytes
	ldy #253
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 13411)
	; 27 bytes
	delay_n 107
	delay_n (n-107)
.elseif(n = 13418 || n = 13429)
	; 25..26 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13420)
	; 25 bytes
	ldy #213
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13421)
	; 22 bytes
	ldy #244
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13422)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13428)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 13430)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 13431)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 13432)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 13433)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 13435)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 13436)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13437)
	; 26 bytes
	delay_n 186
	delay_n (n-186)
.endif
.endif
.endif
.endif
.else
.if((n >= 13438) && (n < 13594))
.if((n >= 13438) && (n < 13516))
.if((n >= 13438) && (n < 13477))
.if((n >= 13438) && (n < 13457))
; 13438 to 13456
.if(n = 13438)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13439)
	; 27 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 13440)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 13441)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13442)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13443)
	; 26 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 13444)
	; 26 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13445)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13446)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13447)
	; 25 bytes
	ldy #249
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13448)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13449 || n = 13455)
	; 24..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13450)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 13451)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 13452)
	; 27 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13453)
	; 23 bytes
	ldy #228
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13454)
	; 27 bytes
	delay_n 44
	delay_n (n-44)
.elseif(n = 13456)
	; 25 bytes
	delay_n (n-3)
	ldy $A4
.endif
.else
; 13457 to 13476
.if(n = 13457 || n = 13465 || (n > 13466 && n < 13471) || n = 13472)
	; 22..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13458)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 13459)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13460)
	; 26 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13461)
	; 26 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 13462)
	; 27 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 13463)
	; 21 bytes
	ldy #254
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 13464 || n = 13474)
	; 25..27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13466)
	; 23 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13471)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13473)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13475)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13476)
	; 22 bytes
	ldy #245
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.endif
.endif
.else
.if((n >= 13477) && (n < 13496))
; 13477 to 13495
.if(n = 13477 || n = 13478 || n = 13480 || n = 13481 || n = 13484 || n = 13485)
	; 23..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13479)
	; 24 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13482)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13483)
	; 25 bytes
	ldy #214
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13486)
	; 26 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13487)
	; 26 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13488)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 13489)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13490)
	; 26 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 13491)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13492)
	; 27 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13493)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 13494)
	; 26 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13495)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.endif
.else
; 13496 to 13515
.if(n = 13496)
	; 26 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 13497)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13498)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13499)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13500)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 13501)
	; 25 bytes
	ldy #250
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13502)
	; 27 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13503 || n = 13505 || n = 13514)
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13504)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 13506)
	; 26 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13507)
	; 27 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13508)
	; 26 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13509)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13510)
	; 23 bytes
	ldy #237
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13511)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 13512)
	; 23 bytes
	ldy #229
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13513 || n = 13515)
	; 25 bytes
	delay_n (n-3)
	ldy $A4
.endif
.endif
.endif
.else
.if((n >= 13516) && (n < 13555))
; 13516 to 13554
.if(n = 13516)
	; 21 bytes
	ldy #255
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 13517 || n = 13518 || (n > 13519 && n < 13524) || n = 13525 || n = 13530 || n = 13533 || (n > 13534 && n < 13538) || n = 13540 || n = 13545 || n = 13548)
	; 22..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13519 || n = 13534)
	; 23..24 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13524)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13526 || n = 13541)
	; 25..26 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13527)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13528)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13529)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 13531)
	; 22 bytes
	ldy #246
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13532)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13538)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 13539)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 13542)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13543)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13544)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13546)
	; 25 bytes
	ldy #215
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13547)
	; 26 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13549)
	; 26 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 13550)
	; 26 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13551)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13552)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13553)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13554)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 13555 to 13593
.if(n = 13555)
	; 25 bytes
	ldy #251
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13556)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 13557 || n = 13568 || n = 13571 || (n > 13572 && n < 13577) || n = 13578 || n = 13583 || n = 13588 || (n > 13589 && n < 13594))
	; 22..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13558 || n = 13570 || n = 13572 || n = 13589)
	; 23..27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13559)
	; 26 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13560)
	; 27 bytes
	delay_n 44
	delay_n (n-44)
.elseif(n = 13561)
	; 26 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13562)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13563)
	; 26 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 13564)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 13565)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13566)
	; 26 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13567)
	; 23 bytes
	ldy #238
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13569)
	; 21 bytes
	ldy #0
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-17
.elseif(n = 13577)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13579)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13580)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13581)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13582)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 13584)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 13585)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13586)
	; 22 bytes
	ldy #247
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13587)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.endif
.endif
.endif
.else
.if((n >= 13594) && (n < 13672))
.if((n >= 13594) && (n < 13633))
.if((n >= 13594) && (n < 13613))
; 13594 to 13612
.if(n = 13594)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 13595)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13596)
	; 26 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13597)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13598)
	; 26 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13599)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 13600)
	; 26 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13601)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13602)
	; 26 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 13603)
	; 26 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13604)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13605)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13606 || n = 13608 || n = 13611)
	; 25..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13607)
	; 24 bytes
	ldy #216 ;hides 'cld'
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13609)
	; 25 bytes
	ldy #252
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13610)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 13612)
	; 26 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 13613 to 13632
.if(n = 13613 || n = 13626 || n = 13628 || n = 13632)
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13614)
	; 26 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13615)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13616)
	; 26 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 13617)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 13618)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13619)
	; 26 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13620)
	; 26 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 13621)
	; 27 bytes
	ldy #227
	delay_n 10
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 13622)
	; 26 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13623)
	; 27 bytes
	delay_n 107
	delay_n (n-107)
.elseif(n = 13624)
	; 23 bytes
	ldy #239
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13625)
	; 26 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 13627)
	; 25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13629)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 13630)
	; 23 bytes
	ldy #231
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13631)
	; 27 bytes
	ldy #235
	delay_n 8
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.endif
.endif
.else
; 13633 to 13671
.if(n = 13633 || n = 13642 || n = 13644 || n = 13666 || n = 13668)
	; 23..27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13634 || n = 13636 || n = 13643 || (n > 13644 && n < 13649) || n = 13653 || n = 13656 || n = 13664 || n = 13667)
	; 23..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13635)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 13637)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 13638)
	; 26 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 13639)
	; 21 bytes
	ldy #248 ;hides 'sed'
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13640)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 13641)
	; 22 bytes
	ldy #248
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13649)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13650)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13651)
	; 25 bytes
	ldy #210
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13652)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 13654)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13655)
	; 26 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 13657)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13658)
	; 27 bytes
	delay_n 89
	delay_n (n-89)
.elseif(n = 13659)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 13660)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13661)
	; 27 bytes
	delay_n 92
	delay_n (n-92)
.elseif(n = 13662)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 13663)
	; 25 bytes
	ldy #253
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13665)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13669)
	; 26 bytes
	ldy #204
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13670)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 13671)
	; 27 bytes
	delay_n 155
	delay_n (n-155)
.endif
.endif
.else
.if((n >= 13672) && (n < 13711))
; 13672 to 13710
.if(n = 13672)
	; 25 bytes
	ldy #217
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13673)
	; 26 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13674 || n = 13676 || n = 13679 || n = 13683 || n = 13685 || n = 13686 || n = 13691 || n = 13693 || n = 13694 || n = 13698 || (n > 13699 && n < 13704) || n = 13709)
	; 23..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13675)
	; 26 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13677)
	; 26 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 13678)
	; 27 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 13680)
	; 26 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 13681)
	; 23 bytes
	ldy #240
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13682)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 13684 || n = 13690 || n = 13692 || n = 13699)
	; 24..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13687)
	; 23 bytes
	ldy #24 ;hides 'clc'
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-20
.elseif(n = 13688)
	; 25 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13689)
	; 23 bytes
	ldy #232
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13695)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 13696)
	; 22 bytes
	ldy #249
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13697)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 13704)
	; 26 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13705)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 13706)
	; 26 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13707)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13708)
	; 26 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13710)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.endif
.else
; 13711 to 13750
.if(n = 13711)
	; 28 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13712)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 13713)
	; 27 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 13714 || n = 13718 || n = 13719 || n = 13723 || n = 13727 || n = 13728 || n = 13734 || n = 13737 || n = 13739 || n = 13740 || (n > 13741 && n < 13745) || n = 13750)
	; 24..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 13715)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 13716)
	; 25 bytes
	ldy #211
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13717)
	; 25 bytes
	ldy #254
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13720)
	; 26 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13721)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13722)
	; 26 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 13724)
	; 27 bytes
	delay_n 155
	delay_n (n-155)
.elseif(n = 13725)
	; 26 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13726)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13729 || n = 13733 || n = 13741)
	; 25..28 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13730)
	; 26 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 13731)
	; 27 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 13732)
	; 27 bytes
	ldy #199
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 13735)
	; 25 bytes
	ldy #218
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13736)
	; 26 bytes
	ldy #205
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13738)
	; 23 bytes
	ldy #241
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13745)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 13746)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13747)
	; 27 bytes
	ldy #237
	delay_n 8
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 13748)
	; 23 bytes
	ldy #233
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13749)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 13751) && (n < 14376))
.if((n >= 13751) && (n < 14063))
.if((n >= 13751) && (n < 13907))
.if((n >= 13751) && (n < 13829))
.if((n >= 13751) && (n < 13790))
; 13751 to 13789
.if(n = 13751)
	; 22 bytes
	ldy #250
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13752 || n = 13753 || (n > 13754 && n < 13759) || n = 13764 || n = 13767 || n = 13773 || n = 13776 || n = 13779 || n = 13783 || n = 13789)
	; 23..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 13754 || n = 13768 || n = 13774 || n = 13784)
	; 24..28 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13759)
	; 26 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13760)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13761)
	; 26 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13762)
	; 26 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13763)
	; 26 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13765)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 13766)
	; 27 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 13769 || n = 13772)
	; 27..28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 13770)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 13771)
	; 25 bytes
	ldy #255
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13775)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 13777)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13778)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13780)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13781)
	; 25 bytes
	ldy #212
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13782)
	; 27 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13785)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 13786)
	; 28 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13787)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13788)
	; 28 bytes
	delay_n 92
	delay_n (n-92)
.endif
.else
; 13790 to 13828
.if(n = 13790 || n = 13809 || n = 13828)
	; 24..27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13791 || n = 13793 || n = 13797 || (n > 13798 && n < 13802) || n = 13808 || (n > 13809 && n < 13814) || n = 13827)
	; 23..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13792 || n = 13794)
	; 26..27 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 13795)
	; 23 bytes
	ldy #242
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13796)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 13798)
	; 25 bytes
	ldy #219
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13802)
	; 27 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 13803)
	; 26 bytes
	ldy #206
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13804)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 13805)
	; 23 bytes
	ldy #234 ;hides 'nop'
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13806)
	; 22 bytes
	ldy #251
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13807)
	; 23 bytes
	ldy #234
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13814)
	; 26 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13815)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 13816)
	; 26 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13817)
	; 26 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13818 || n = 13819)
	; 26..27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13820)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 13821)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13822)
	; 27 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13823)
	; 27 bytes
	ldy #216 ;hides 'cld'
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 13824)
	; 28 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 13825)
	; 25 bytes
	ldy #0
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13826)
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.endif
.endif
.else
.if((n >= 13829) && (n < 13868))
; 13829 to 13867
.if(n = 13829 || n = 13831 || n = 13850 || n = 13853 || n = 13854 || (n > 13855 && n < 13860) || n = 13863 || n = 13865 || n = 13867)
	; 23..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 13830)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13832)
	; 27 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13833)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13834)
	; 28 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 13835)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 13836)
	; 27 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 13837 || n = 13838)
	; 27..28 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13839)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13840)
	; 27 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13841)
	; 28 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13842 || n = 13843)
	; 27..28 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13844)
	; 27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13845)
	; 28 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13846)
	; 25 bytes
	ldy #213
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13847)
	; 27 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 13848)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13849 || n = 13851 || n = 13855 || n = 13864)
	; 24..27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13852)
	; 23 bytes
	ldy #243
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13860)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13861)
	; 22 bytes
	ldy #252
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13862)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13866)
	; 23 bytes
	ldy #235
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.endif
.else
.if((n >= 13868) && (n < 13887))
; 13868 to 13886
.if(n = 13868 || n = 13870 || n = 13872 || n = 13875)
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13869)
	; 25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13871 || n = 13876)
	; 26..27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13873 || n = 13878)
	; 26..27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13874)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13877)
	; 27 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13879)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 13880)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13881)
	; 28 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 13882)
	; 27 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 13883)
	; 28 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 13884)
	; 28 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 13885)
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 13886)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 13887 to 13906
.if(n = 13887)
	; 25 bytes
	ldy #248 ;hides 'sed'
	delay_n 4
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13888)
	; 28 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13889)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13890)
	; 27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13891 || n = 13893)
	; 27..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 13892)
	; 27 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13894)
	; 28 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 13895 || n = 13900)
	; 27..28 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13896)
	; 28 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 13897)
	; 27 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13898)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13899)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13901)
	; 27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13902)
	; 27 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 13903)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 13904)
	; 27 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 13905)
	; 28 bytes
	delay_n 99
	delay_n (n-99)
.elseif(n = 13906)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.endif
.endif
.endif
.endif
.else
.if((n >= 13907) && (n < 13985))
.if((n >= 13907) && (n < 13946))
; 13907 to 13945
.if(n = 13907)
	; 27 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13908)
	; 28 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 13909)
	; 23 bytes
	ldy #244
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13910)
	; 26 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13911 || (n > 13912 && n < 13916) || n = 13918 || (n > 13919 && n < 13924) || n = 13926 || n = 13927 || (n > 13928 && n < 13932))
	; 23..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13912 || n = 13919 || n = 13928 || n = 13940)
	; 24..28 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13916)
	; 22 bytes
	ldy #253
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13917)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 13924)
	; 25 bytes
	ldy #221
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13925)
	; 23 bytes
	ldy #236
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13932 || n = 13941)
	; 27..28 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13933)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13934)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 13935)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13936)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13937)
	; 26 bytes
	ldy #208
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 13938)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 13939)
	; 27 bytes
	ldy #202
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 13942)
	; 27 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13943)
	; 28 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 13944)
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 13945)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.endif
.else
; 13946 to 13984
.if(n = 13946)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 13947 || n = 13949 || n = 13954 || n = 13964 || n = 13968 || n = 13972 || n = 13973 || n = 13975 || n = 13977 || n = 13978 || n = 13980)
	; 23..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 13948 || n = 13969 || n = 13974)
	; 24..28 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 13950)
	; 27 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 13951)
	; 27 bytes
	ldy #225
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 13952 || n = 13961)
	; 27..28 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13953)
	; 28 bytes
	ldy #218
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 13955 || n = 13962)
	; 27..28 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13956)
	; 28 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13957)
	; 27 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 13958 || n = 13965)
	; 26..27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 13959)
	; 27 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 13960 || n = 13967)
	; 27..28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 13963)
	; 28 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13966)
	; 23 bytes
	ldy #245
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 13970)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 13971)
	; 22 bytes
	ldy #254
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 13976)
	; 25 bytes
	ldy #215
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13979)
	; 26 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13981)
	; 26 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13982)
	; 26 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13983)
	; 26 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13984)
	; 23 bytes
	ldy #237
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.endif
.endif
.else
.if((n >= 13985) && (n < 14024))
.if((n >= 13985) && (n < 14004))
; 13985 to 14003
.if(n = 13985 || n = 13986 || (n > 13987 && n < 13992) || n = 13999 || n = 14003)
	; 24..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 13987)
	; 25 bytes
	ldy #222
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 13992)
	; 27 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 13993)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 13994)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13995)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13996)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13997)
	; 27 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13998)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 14000)
	; 28 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14001)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14002)
	; 27 bytes
	delay_n 31
	delay_n (n-31)
.endif
.else
; 14004 to 14023
.if(n = 14004)
	; 26 bytes
	ldy #209
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14005)
	; 27 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14006 || n = 14009 || n = 14019)
	; 27..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14007)
	; 27 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14008)
	; 27 bytes
	ldy #203
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14010)
	; 28 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14011)
	; 28 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 14012)
	; 27 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 14013)
	; 27 bytes
	ldy #226
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14014)
	; 27 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 14015 || n = 14020)
	; 26..27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 14016)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14017)
	; 27 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14018)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 14021)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14022)
	; 27 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 14023)
	; 23 bytes
	ldy #246
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.endif
.endif
.else
; 14024 to 14062
.if(n = 14024 || n = 14025 || n = 14027 || n = 14028 || n = 14030 || n = 14032 || n = 14033 || n = 14035 || n = 14040 || n = 14045 || (n > 14046 && n < 14050) || n = 14052 || n = 14054 || n = 14059 || n = 14061)
	; 23..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14026)
	; 22 bytes
	ldy #255
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.elseif(n = 14029 || n = 14034 || n = 14044 || n = 14046 || n = 14053)
	; 24..27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14031)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 14036)
	; 26 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14037)
	; 26 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14038 || n = 14055)
	; 26..27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14039)
	; 25 bytes
	ldy #216 ;hides 'cld'
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14041)
	; 25 bytes
	ldy #216
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14042)
	; 27 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14043)
	; 23 bytes
	ldy #238
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14050)
	; 25 bytes
	ldy #223
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14051)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14056)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 14057)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14058)
	; 28 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 14060)
	; 27 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14062)
	; 27 bytes
	delay_n 36
	delay_n (n-36)
.endif
.endif
.endif
.endif
.else
.if((n >= 14063) && (n < 14219))
.if((n >= 14063) && (n < 14141))
.if((n >= 14063) && (n < 14102))
.if((n >= 14063) && (n < 14082))
; 14063 to 14081
.if(n = 14063)
	; 28 bytes
	delay_n 92
	delay_n (n-92)
.elseif(n = 14064)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 14065)
	; 28 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14066)
	; 28 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14067)
	; 27 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 14068 || n = 14073)
	; 27..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14069)
	; 27 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 14070)
	; 28 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 14071)
	; 26 bytes
	ldy #210
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14072)
	; 27 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14074)
	; 28 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14075)
	; 26 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 14076)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14077)
	; 27 bytes
	ldy #204
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14078)
	; 28 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 14079)
	; 28 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14080)
	; 23 bytes
	ldy #247
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14081)
	; 22 bytes
	ldy #0
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-18
.endif
.else
; 14082 to 14101
.if(n = 14082 || n = 14083 || (n > 14084 && n < 14089) || n = 14094 || n = 14096 || n = 14098)
	; 23..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14084 || n = 14095)
	; 24..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14089)
	; 26 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14090 || n = 14091)
	; 26..27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14092)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 14093)
	; 26 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14097)
	; 27 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14099)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 14100)
	; 28 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 14101)
	; 27 bytes
	ldy #235
	delay_n 10
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.endif
.endif
.else
.if((n >= 14102) && (n < 14121))
; 14102 to 14120
.if(n = 14102)
	; 23 bytes
	ldy #239
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14103 || n = 14104 || n = 14107 || n = 14108 || n = 14110 || n = 14115 || n = 14118)
	; 24..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14105 || n = 14109 || n = 14116)
	; 25..27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14106)
	; 25 bytes
	ldy #217
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14111)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 14112)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14113)
	; 25 bytes
	ldy #224
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14114)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14117)
	; 27 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14119)
	; 28 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 14120)
	; 28 bytes
	delay_n 136
	delay_n (n-136)
.endif
.else
; 14121 to 14140
.if(n = 14121)
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 14122)
	; 27 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 14123)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 14124)
	; 27 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 14125)
	; 28 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14126 || n = 14127)
	; 27..28 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14128)
	; 27 bytes
	ldy #200 ;hides 'iny'
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	dey
	Jne *-24
.elseif(n = 14129 || n = 14130)
	; 26..27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 14131)
	; 28 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 14132)
	; 27 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 14133)
	; 28 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14134)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14135)
	; 22 bytes
	ldy #248 ;hides 'sed'
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14136)
	; 27 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14137)
	; 23 bytes
	ldy #248
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14138 || n = 14140)
	; 24..25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14139)
	; 24 bytes
	delay_n (n-2)
	nop
.endif
.endif
.endif
.else
.if((n >= 14141) && (n < 14180))
; 14141 to 14179
.if((n > 14140 && n < 14145) || n = 14155 || (n > 14156 && n < 14160) || n = 14163 || (n > 14164 && n < 14169) || n = 14173 || n = 14175 || n = 14178)
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14145)
	; 26 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14146 || n = 14148)
	; 26..27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14147 || n = 14149)
	; 26..27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14150)
	; 27 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 14151)
	; 23 bytes
	ldy #24 ;hides 'clc'
	.repeat 9
	 rol $36,X
	.endrepeat
	iny
	Jne *-20
.elseif(n = 14152)
	; 27 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 14153)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 14154 || n = 14156 || n = 14164 || n = 14174 || n = 14179)
	; 25..27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14160)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 14161)
	; 23 bytes
	ldy #240
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14162)
	; 27 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14169)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14170)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 14171)
	; 25 bytes
	ldy #218
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14172)
	; 27 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14176)
	; 25 bytes
	ldy #225
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14177)
	; 27 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
.if((n >= 14180) && (n < 14199))
; 14180 to 14198
.if(n = 14180 || n = 14196 || n = 14198)
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14181)
	; 27 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14182)
	; 27 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 14183)
	; 28 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14184 || n = 14186)
	; 26..27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 14185)
	; 28 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14187)
	; 27 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14188)
	; 28 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 14189)
	; 28 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 14190)
	; 27 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14191)
	; 27 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 14192)
	; 27 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 14193)
	; 27 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 14194)
	; 23 bytes
	ldy #249
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14195)
	; 28 bytes
	delay_n 169
	delay_n (n-169)
.elseif(n = 14197)
	; 25 bytes
	delay_n (n-3)
	ldy $A4
.endif
.else
; 14199 to 14218
.if(n = 14199 || n = 14200 || n = 14213 || n = 14216 || n = 14218)
	; 25..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14201)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14202)
	; 27 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 14203)
	; 28 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 14204)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14205)
	; 26 bytes
	ldy #212
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14206)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14207)
	; 27 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 14208)
	; 28 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14209)
	; 28 bytes
	ldy #222
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 14210)
	; 27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 14211)
	; 27 bytes
	ldy #245
	delay_n 8
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14212)
	; 27 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 14214)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 14215)
	; 27 bytes
	ldy #206
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14217)
	; 26 bytes
	delay_n (n-3)
	ldy $A4
.endif
.endif
.endif
.endif
.else
.if((n >= 14219) && (n < 14297))
.if((n >= 14219) && (n < 14258))
; 14219 to 14257
.if(n = 14219 || n = 14222 || n = 14224 || n = 14226 || n = 14227 || n = 14229 || n = 14238 || n = 14240 || n = 14241 || n = 14243 || n = 14244 || n = 14249 || n = 14253 || (n > 14254 && n < 14258))
	; 24..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14220)
	; 23 bytes
	ldy #241
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14221)
	; 27 bytes
	ldy #237
	delay_n 10
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14223 || n = 14242 || n = 14254)
	; 25..27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14225)
	; 26 bytes
	ldy #(n-1)/56
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14228)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14230)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14231)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14232)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14233)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 14234)
	; 27 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 14235)
	; 28 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 14236)
	; 25 bytes
	ldy #219
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14237)
	; 27 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 14239)
	; 25 bytes
	ldy #226
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14245)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 14246)
	; 27 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 14247)
	; 27 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 14248)
	; 27 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 14250)
	; 28 bytes
	delay_n 169
	delay_n (n-169)
.elseif(n = 14251)
	; 23 bytes
	ldy #250
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14252)
	; 27 bytes
	delay_n 171
	delay_n (n-171)
.endif
.else
; 14258 to 14296
.if(n = 14258 || n = 14264 || n = 14274 || n = 14277 || n = 14280 || n = 14281 || (n > 14282 && n < 14287))
	; 24..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14259 || n = 14287)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14260)
	; 28 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 14261)
	; 27 bytes
	ldy #230
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14262)
	; 27 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 14263 || n = 14291)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14265)
	; 28 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 14266 || n = 14292)
	; 27..28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 14267)
	; 27 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 14268)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 14269)
	; 27 bytes
	ldy #246
	delay_n 8
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14270)
	; 28 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 14271)
	; 27 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 14272)
	; 26 bytes
	ldy #213
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14273)
	; 23 bytes
	ldy #234 ;hides 'nop'
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 14275)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 14276 || n = 14278 || n = 14282)
	; 25..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14279)
	; 23 bytes
	ldy #242
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14288)
	; 27 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 14289)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14290)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14293)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 14294)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 14295)
	; 28 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14296)
	; 27 bytes
	delay_n 161
	delay_n (n-161)
.endif
.endif
.else
.if((n >= 14297) && (n < 14336))
; 14297 to 14335
.if(n = 14297)
	; 28 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 14298 || n = 14331 || n = 14333)
	; 27..28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 14299 || n = 14334)
	; 28 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14300 || n = 14322 || n = 14328)
	; 27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 14301)
	; 25 bytes
	ldy #220
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14302)
	; 25 bytes
	ldy #227
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14303 || n = 14304 || n = 14306 || n = 14307 || n = 14310 || (n > 14311 && n < 14316) || n = 14325 || n = 14329)
	; 24..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14305 || n = 14311)
	; 25..27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14308)
	; 23 bytes
	ldy #251
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14309)
	; 27 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 14316)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14317)
	; 28 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 14318)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14319)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14320)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14321)
	; 27 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 14323)
	; 27 bytes
	ldy #231
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14324)
	; 28 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14326)
	; 27 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 14327)
	; 27 bytes
	ldy #247
	delay_n 8
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14330)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 14332)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 14335)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.endif
.else
; 14336 to 14375
.if(n = 14336)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 14337)
	; 26 bytes
	ldy #0
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14338)
	; 23 bytes
	ldy #243
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14339 || n = 14341 || n = 14368)
	; 25..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14340 || (n > 14341 && n < 14346) || n = 14355 || n = 14358 || n = 14367 || (n > 14368 && n < 14373))
	; 24..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14346 || n = 14373)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14347)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 14348 || n = 14375)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14349)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14350)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14351)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 14352)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 14353)
	; 27 bytes
	ldy #208
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14354)
	; 28 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14356)
	; 28 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 14357)
	; 27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 14359)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 14360)
	; 28 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14361)
	; 28 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14362)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 14363)
	; 28 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14364)
	; 28 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14365)
	; 23 bytes
	ldy #252
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14366)
	; 25 bytes
	ldy #221
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14374)
	; 28 bytes
	delay_n 36
	delay_n (n-36)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 14376) && (n < 14688))
.if((n >= 14376) && (n < 14532))
.if((n >= 14376) && (n < 14454))
.if((n >= 14376) && (n < 14415))
.if((n >= 14376) && (n < 14395))
; 14376 to 14394
.if(n = 14376)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14377)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14378)
	; 28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14379)
	; 28 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 14380)
	; 28 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 14381)
	; 28 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 14382)
	; 28 bytes
	ldy #197
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 14383)
	; 26 bytes
	ldy #248 ;hides 'sed'
	delay_n 6
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14384)
	; 28 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14385)
	; 27 bytes
	ldy #232
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14386)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 14387)
	; 27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 14388)
	; 29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14389)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 14390)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 14391)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 14392)
	; 28 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14393)
	; 28 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14394)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.endif
.else
; 14395 to 14414
.if(n = 14395)
	; 28 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 14396)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 14397)
	; 23 bytes
	ldy #244
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14398 || n = 14399 || (n > 14400 && n < 14405) || n = 14408)
	; 24..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14400)
	; 25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14405)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14406)
	; 26 bytes
	ldy #215
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14407)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14409)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14410)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 14411)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 14412)
	; 28 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 14413)
	; 28 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14414)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.endif
.endif
.else
.if((n >= 14415) && (n < 14434))
; 14415 to 14433
.if(n = 14415)
	; 28 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 14416)
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 14417)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 14418)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 14419)
	; 28 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14420)
	; 28 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14421)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 14422)
	; 23 bytes
	ldy #253
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14423)
	; 28 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14424 || n = 14426 || n = 14427 || n = 14429 || n = 14430 || n = 14432 || n = 14433)
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14425)
	; 25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14428)
	; 25 bytes
	ldy #229
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14431)
	; 25 bytes
	ldy #222
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.endif
.else
; 14434 to 14453
.if(n = 14434)
	; 27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14435 || n = 14436 || n = 14445 || n = 14449)
	; 27..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14437)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 14438)
	; 28 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 14439)
	; 28 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 14440)
	; 28 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 14441)
	; 28 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 14442)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 14443)
	; 27 bytes
	ldy #249
	delay_n 8
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14444)
	; 28 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14446)
	; 27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 14447)
	; 27 bytes
	ldy #233
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14448)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 14450)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 14451)
	; 28 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 14452)
	; 28 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14453)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.endif
.endif
.endif
.else
.if((n >= 14454) && (n < 14493))
; 14454 to 14492
.if(n = 14454)
	; 28 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 14455)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 14456)
	; 23 bytes
	ldy #245
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14457 || (n > 14460 && n < 14464) || n = 14475 || n = 14480 || n = 14481 || (n > 14482 && n < 14487))
	; 24..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14458)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 14459 || n = 14474 || n = 14476 || n = 14482)
	; 25..28 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14460)
	; 25 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 14464 || n = 14487)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14465)
	; 28 bytes
	ldy #226
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 14466 || n = 14489)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14467 || n = 14490)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14468)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14469)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 14470)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 14471)
	; 25 bytes
	ldy #216 ;hides 'cld'
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14472)
	; 28 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14473)
	; 26 bytes
	ldy #216
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14477)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 14478)
	; 28 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14479)
	; 23 bytes
	ldy #254
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14488)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14491)
	; 25 bytes
	ldy #230
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14492)
	; 28 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 14493 to 14531
.if(n = 14493 || n = 14495 || n = 14498 || n = 14500 || n = 14503 || n = 14511 || n = 14517 || n = 14520 || n = 14521 || n = 14523 || n = 14524)
	; 24..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14494 || n = 14499 || n = 14518 || n = 14522)
	; 25..27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14496)
	; 25 bytes
	ldy #223
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14497)
	; 28 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 14501)
	; 27 bytes
	ldy #250
	delay_n 8
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14502)
	; 28 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14504)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 14505 || n = 14528)
	; 27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 14506)
	; 28 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14507)
	; 27 bytes
	ldy #234 ;hides 'nop'
	delay_n 10
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 14508)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 14509)
	; 27 bytes
	ldy #234
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14510)
	; 28 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14512)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 14513)
	; 28 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 14514)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 14515)
	; 23 bytes
	ldy #246
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14516)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14519)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 14525)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14526)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14527)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14529)
	; 28 bytes
	ldy #227
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 14530)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 14531)
	; 28 bytes
	delay_n 16
	delay_n (n-16)
.endif
.endif
.endif
.else
.if((n >= 14532) && (n < 14610))
.if((n >= 14532) && (n < 14571))
.if((n >= 14532) && (n < 14551))
; 14532 to 14550
.if(n = 14532)
	; 28 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 14533)
	; 28 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 14534)
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 14535)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 14536)
	; 23 bytes
	ldy #255
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14537)
	; 28 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14538 || (n > 14539 && n < 14544))
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14539)
	; 25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14544)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14545)
	; 28 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 14546)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14547)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14548)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14549)
	; 28 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14550)
	; 28 bytes
	delay_n 71
	delay_n (n-71)
.endif
.else
; 14551 to 14570
.if(n = 14551)
	; 28 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14552)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14553)
	; 28 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 14554)
	; 25 bytes
	ldy #231
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14555)
	; 28 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 14556 || n = 14558 || n = 14562 || n = 14563 || n = 14565 || n = 14569)
	; 26..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14557 || n = 14564)
	; 27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14559)
	; 27 bytes
	ldy #251
	delay_n 8
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14560)
	; 27 bytes
	ldy #211
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14561)
	; 25 bytes
	ldy #224
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14566)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 14567)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 14568)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 14570)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
; 14571 to 14609
.if(n = 14571)
	; 27 bytes
	ldy #235
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14572)
	; 28 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 14573 || n = 14576 || n = 14578 || n = 14579 || n = 14582 || n = 14584 || n = 14585 || n = 14594 || n = 14595 || (n > 14596 && n < 14601) || n = 14609)
	; 24..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14574)
	; 23 bytes
	ldy #247
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14575)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14577 || n = 14583 || n = 14596)
	; 25..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14580)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 14581)
	; 27 bytes
	ldy #243
	delay_n 10
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14586 || n = 14605)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14587)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 14588)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 14589)
	; 28 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 14590)
	; 28 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14591)
	; 28 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 14592)
	; 28 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 14593)
	; 23 bytes
	ldy #0
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14601)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14602)
	; 28 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 14603)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14604)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14606)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14607)
	; 26 bytes
	ldy #218
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14608)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.endif
.endif
.else
.if((n >= 14610) && (n < 14649))
; 14610 to 14648
.if(n = 14610)
	; 28 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14611)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14612)
	; 28 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 14613)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 14614)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 14615)
	; 24 bytes
	ldy #24 ;hides 'clc'
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	iny
	Jne *-21
.elseif(n = 14616)
	; 28 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 14617)
	; 25 bytes
	ldy #232
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14618 || n = 14620 || n = 14634 || n = 14636 || n = 14644)
	; 25..27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14619 || n = 14621 || n = 14622 || n = 14628 || n = 14630 || n = 14635 || (n > 14636 && n < 14641) || n = 14643 || (n > 14644 && n < 14649))
	; 24..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14623 || n = 14642)
	; 27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 14624)
	; 28 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14625)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 14626)
	; 25 bytes
	ldy #225
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14627)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14629)
	; 27 bytes
	ldy #212
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14631)
	; 23 bytes
	ldy #248 ;hides 'sed'
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 14632)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 14633)
	; 23 bytes
	ldy #248
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14641)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.endif
.else
.if((n >= 14649) && (n < 14668))
; 14649 to 14667
.if(n = 14649)
	; 28 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14650)
	; 28 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 14651)
	; 28 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14652)
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 14653)
	; 28 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14654)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 14655)
	; 28 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14656)
	; 28 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14657)
	; 28 bytes
	ldy #229
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 14658 || n = 14663)
	; 29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14659)
	; 28 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14660)
	; 28 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 14661)
	; 28 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 14662)
	; 28 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 14664)
	; 28 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14665)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14666)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14667)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.endif
.else
; 14668 to 14687
.if(n = 14668)
	; 28 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 14669)
	; 28 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14670)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14671)
	; 28 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 14672)
	; 28 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 14673 || (n > 14675 && n < 14679) || n = 14682 || n = 14684 || n = 14685)
	; 26..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14674)
	; 26 bytes
	ldy #219
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14675)
	; 27 bytes
	ldy #253
	delay_n 8
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14679)
	; 28 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14680)
	; 25 bytes
	ldy #233
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14681)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 14683)
	; 27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14686)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 14687)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 14688) && (n < 14844))
.if((n >= 14688) && (n < 14766))
.if((n >= 14688) && (n < 14727))
.if((n >= 14688) && (n < 14707))
; 14688 to 14706
.if(n = 14688)
	; 28 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14689)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 14690)
	; 28 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 14691)
	; 25 bytes
	ldy #226
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14692)
	; 23 bytes
	ldy #249
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14693 || n = 14694 || (n > 14695 && n < 14700) || n = 14704 || n = 14706)
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14695 || n = 14705)
	; 25..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14700)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14701)
	; 27 bytes
	ldy #245
	delay_n 10
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14702)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 14703)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.endif
.else
; 14707 to 14726
.if(n = 14707 || n = 14708 || n = 14716)
	; 27..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14709)
	; 28 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 14710)
	; 28 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 14711)
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 14712)
	; 28 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14713)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 14714)
	; 28 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14715)
	; 28 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14717)
	; 28 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 14718)
	; 28 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14719)
	; 28 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 14720)
	; 28 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 14721)
	; 28 bytes
	ldy #230
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 14722)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 14723)
	; 28 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14724)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14725)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14726)
	; 28 bytes
	delay_n 34
	delay_n (n-34)
.endif
.endif
.else
; 14727 to 14765
.if(n = 14727)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 14728)
	; 28 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14729)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14730)
	; 28 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 14731 || n = 14735 || n = 14745 || n = 14747 || n = 14748 || n = 14753 || n = 14755 || n = 14757 || n = 14758 || n = 14760 || n = 14765)
	; 24..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14732)
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 14733)
	; 27 bytes
	ldy #254
	delay_n 8
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14734)
	; 28 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 14736 || n = 14744 || n = 14746 || n = 14754 || n = 14759)
	; 25..29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14737 || n = 14739)
	; 28 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14738)
	; 28 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14740)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 14741)
	; 24 bytes
	ldy #234 ;hides 'nop'
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14742)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 14743)
	; 25 bytes
	ldy #234
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14749)
	; 28 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 14750)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 14751)
	; 23 bytes
	ldy #250
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14752 || n = 14762)
	; 27..28 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14756)
	; 25 bytes
	ldy #227
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14761)
	; 27 bytes
	ldy #246
	delay_n 10
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14763)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 14764)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.endif
.endif
.else
.if((n >= 14766) && (n < 14805))
.if((n >= 14766) && (n < 14785))
; 14766 to 14784
.if(n = 14766)
	; 26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14767 || n = 14768 || n = 14771 || n = 14781)
	; 26..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14769)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14770)
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 14772)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 14773)
	; 28 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14774)
	; 28 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14775)
	; 28 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14776)
	; 29 bytes
	ldy #197
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 14777)
	; 28 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14778)
	; 28 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 14779)
	; 28 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 14780)
	; 29 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 14782)
	; 28 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14783)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14784)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 14785 to 14804
.if(n = 14785)
	; 28 bytes
	ldy #231
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 14786)
	; 28 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 14787)
	; 28 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14788)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14789)
	; 28 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 14790 || n = 14800)
	; 27..28 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 14791)
	; 27 bytes
	ldy #255
	delay_n 8
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14792)
	; 28 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 14793 || n = 14795 || n = 14796)
	; 28..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14794)
	; 28 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 14797)
	; 28 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14798)
	; 28 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14799)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 14801)
	; 29 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 14802)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 14803)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 14804)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.endif
.endif
.else
; 14805 to 14843
.if(n = 14805)
	; 28 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 14806)
	; 25 bytes
	ldy #235
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14807)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 14808 || n = 14811 || n = 14812 || (n > 14813 && n < 14818) || n = 14823 || n = 14825 || n = 14826 || (n > 14827 && n < 14831) || n = 14838)
	; 24..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14809 || n = 14813 || n = 14827 || n = 14839)
	; 25..29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14810)
	; 23 bytes
	ldy #251
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14818)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14819)
	; 27 bytes
	ldy #239
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14820 || n = 14834)
	; 27..28 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14821)
	; 25 bytes
	ldy #228
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14822)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14824)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 14831)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 14832)
	; 28 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14833)
	; 28 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14835)
	; 28 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14836)
	; 27 bytes
	ldy #215
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14837)
	; 28 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 14840)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14841)
	; 28 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14842)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14843)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.endif
.endif
.endif
.else
.if((n >= 14844) && (n < 14922))
.if((n >= 14844) && (n < 14883))
.if((n >= 14844) && (n < 14863))
; 14844 to 14862
.if(n = 14844)
	; 28 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14845)
	; 28 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 14846 || n = 14860)
	; 28..29 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14847)
	; 27 bytes
	ldy #24 ;hides 'clc'
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	iny
	Jne *-24
.elseif(n = 14848)
	; 28 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 14849)
	; 27 bytes
	ldy #0
	delay_n 8
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14850)
	; 29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14851 || n = 14854)
	; 28..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14852)
	; 28 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 14853)
	; 28 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 14855)
	; 29 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14856)
	; 28 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14857)
	; 28 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14858)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 14859)
	; 27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 14861)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 14862)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.endif
.else
; 14863 to 14882
.if(n = 14863)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 14864 || n = 14871 || n = 14873 || n = 14874 || n = 14876 || n = 14877)
	; 24..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14865)
	; 28 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14866)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 14867)
	; 28 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 14868)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 14869)
	; 23 bytes
	ldy #252
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14870)
	; 29 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14872)
	; 25 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14875)
	; 26 bytes
	ldy #222
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14878)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 14879)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14880)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14881)
	; 27 bytes
	ldy #248
	delay_n 10
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14882)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.endif
.endif
.else
.if((n >= 14883) && (n < 14902))
; 14883 to 14901
.if(n = 14883 || n = 14887 || (n > 14888 && n < 14892) || n = 14894 || n = 14899)
	; 25..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14884 || n = 14888)
	; 26..29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14885)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 14886)
	; 25 bytes
	ldy #229
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14892)
	; 28 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14893)
	; 28 bytes
	ldy #204
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 14895)
	; 28 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14896)
	; 28 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 14897)
	; 28 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 14898)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14900)
	; 28 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14901)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
; 14902 to 14921
.if(n = 14902)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14903)
	; 26 bytes
	ldy #216 ;hides 'cld'
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14904)
	; 28 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 14905)
	; 27 bytes
	ldy #216
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14906)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14907 || n = 14909)
	; 28..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14908 || n = 14914)
	; 29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14910)
	; 28 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 14911)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14912)
	; 28 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 14913)
	; 28 bytes
	ldy #233
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 14915)
	; 28 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14916)
	; 28 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14917)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 14918)
	; 27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 14919)
	; 29 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 14920)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 14921)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.endif
.endif
.endif
.else
.if((n >= 14922) && (n < 14961))
; 14922 to 14960
.if(n = 14922)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 14923 || n = 14929 || n = 14930 || n = 14933 || n = 14934 || n = 14936 || n = 14937 || n = 14944 || n = 14945 || n = 14948 || n = 14950 || n = 14952 || n = 14953 || n = 14955 || n = 14957)
	; 24..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14924)
	; 28 bytes
	ldy #200 ;hides 'iny'
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	dey
	Jne *-25
.elseif(n = 14925)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 14926)
	; 28 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 14927)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 14928)
	; 23 bytes
	ldy #253
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14931 || n = 14935 || n = 14949 || n = 14954)
	; 25..27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14932)
	; 25 bytes
	ldy #237
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14938 || n = 14956)
	; 27..28 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14939)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14940 || n = 14958)
	; 27..28 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14941)
	; 27 bytes
	ldy #249
	delay_n 10
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14942)
	; 26 bytes
	ldy #223
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 14943)
	; 27 bytes
	ldy #241
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14946)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 14947)
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 14951)
	; 25 bytes
	ldy #230
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14959)
	; 28 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 14960)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.endif
.else
.if((n >= 14961) && (n < 14981))
; 14961 to 14980
.if(n = 14961)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14962)
	; 28 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 14963)
	; 28 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 14964)
	; 28 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14965)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14966)
	; 28 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 14967)
	; 29 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 14968 || n = 14973 || n = 14976)
	; 28..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14969)
	; 28 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 14970)
	; 28 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 14971)
	; 28 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 14972)
	; 29 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14974)
	; 27 bytes
	ldy #217
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14975)
	; 27 bytes
	ldy #234 ;hides 'nop'
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 14977)
	; 27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 14978)
	; 29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14979)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 14980)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.endif
.else
; 14981 to 15000
.if(n = 14981)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 14982)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 14983)
	; 28 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 14984)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 14985)
	; 28 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 14986)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 14987)
	; 23 bytes
	ldy #254
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 14988 || n = 14989 || (n > 14990 && n < 14995) || n = 14997 || n = 15000)
	; 24..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14990 || n = 14998)
	; 25..27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 14995)
	; 25 bytes
	ldy #238
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 14996)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 14999)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 15001) && (n < 17501))
.if((n >= 15001) && (n < 16251))
.if((n >= 15001) && (n < 15626))
.if((n >= 15001) && (n < 15313))
.if((n >= 15001) && (n < 15157))
.if((n >= 15001) && (n < 15079))
.if((n >= 15001) && (n < 15040))
; 15001 to 15039
.if(n = 15001)
	; 27 bytes
	ldy #250
	delay_n 10
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15002 || n = 15010 || n = 15019)
	; 26..29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15003 || n = 15009 || (n > 15010 && n < 15014) || n = 15018 || (n > 15019 && n < 15023) || n = 15027 || n = 15031 || n = 15032)
	; 25..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15004)
	; 28 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 15005)
	; 27 bytes
	ldy #242
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15006)
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 15007)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 15008)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15014)
	; 28 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 15015)
	; 28 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 15016)
	; 25 bytes
	ldy #231
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15017 || n = 15026)
	; 28..29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15023)
	; 28 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15024)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15025)
	; 28 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15028)
	; 28 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 15029)
	; 28 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15030)
	; 28 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 15033)
	; 28 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15034)
	; 28 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15035)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 15036)
	; 27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15037)
	; 29 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 15038)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 15039)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.endif
.else
; 15040 to 15078
.if(n = 15040)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 15041)
	; 28 bytes
	ldy #235
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15042)
	; 28 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15043)
	; 27 bytes
	ldy #218
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15044)
	; 28 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 15045 || n = 15047 || n = 15048 || (n > 15049 && n < 15053) || n = 15060 || n = 15062 || n = 15063 || n = 15066 || n = 15069 || n = 15070 || (n > 15071 && n < 15076) || n = 15078)
	; 24..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15046)
	; 23 bytes
	ldy #255
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 15049 || n = 15071)
	; 25..26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15053)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15054)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15055)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 15056)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15057)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15058)
	; 25 bytes
	ldy #239
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15059)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 15061)
	; 27 bytes
	ldy #251
	delay_n 10
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15064)
	; 28 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 15065)
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 15067)
	; 27 bytes
	ldy #243
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15068)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 15076)
	; 26 bytes
	ldy #225
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 15077)
	; 28 bytes
	delay_n 31
	delay_n (n-31)
.endif
.endif
.else
.if((n >= 15079) && (n < 15118))
; 15079 to 15117
.if(n = 15079)
	; 25 bytes
	ldy #24 ;hides 'clc'
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-22
.elseif(n = 15080 || n = 15117)
	; 27..28 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15081)
	; 25 bytes
	ldy #232
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15082 || n = 15084 || n = 15108)
	; 25..27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15083 || n = 15085 || n = 15086 || n = 15090 || n = 15100 || n = 15106 || n = 15107 || (n > 15108 && n < 15112) || n = 15114)
	; 24..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15087)
	; 28 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 15088)
	; 28 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15089)
	; 28 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 15091 || n = 15115)
	; 27..29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15092)
	; 28 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15093)
	; 28 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15094)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 15095)
	; 27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15096)
	; 29 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 15097)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 15098)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 15099)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 15101)
	; 28 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15102)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15103)
	; 28 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 15104)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 15105)
	; 23 bytes
	ldy #0
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-19
.elseif(n = 15112)
	; 27 bytes
	ldy #219
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15113)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15116)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.endif
.else
.if((n >= 15118) && (n < 15137))
; 15118 to 15136
.if(n = 15118)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 15119)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 15120 || n = 15124 || n = 15130 || n = 15132)
	; 25..29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15121)
	; 25 bytes
	ldy #240
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15122)
	; 28 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 15123 || n = 15125 || n = 15131 || (n > 15132 && n < 15136))
	; 25..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 15126)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15127)
	; 23 bytes
	ldy #248 ;hides 'sed'
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 15128)
	; 28 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15129)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 15136)
	; 28 bytes
	delay_n 31
	delay_n (n-31)
.endif
.else
; 15137 to 15156
.if(n = 15137)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15138)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15139)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15140)
	; 28 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 15141)
	; 28 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15142)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15143)
	; 26 bytes
	ldy #226
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 15144 || n = 15145 || n = 15148 || n = 15150)
	; 26..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15146)
	; 25 bytes
	ldy #233
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15147)
	; 28 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15149)
	; 27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15151)
	; 28 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15152)
	; 28 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15153)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 15154)
	; 27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15155)
	; 29 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 15156)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.endif
.endif
.endif
.endif
.else
.if((n >= 15157) && (n < 15235))
.if((n >= 15157) && (n < 15196))
.if((n >= 15157) && (n < 15176))
; 15157 to 15175
.if(n = 15157)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 15158)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 15159 || n = 15164)
	; 29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15160)
	; 28 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 15161)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15162)
	; 28 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 15163)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 15165)
	; 29 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15166)
	; 28 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 15167)
	; 28 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 15168)
	; 28 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 15169)
	; 28 bytes
	ldy #237
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15170)
	; 29 bytes
	ldy #197
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 15171)
	; 28 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 15172)
	; 28 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 15173)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 15174)
	; 28 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 15175)
	; 29 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 15176 to 15195
.if(n = 15176)
	; 27 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15177)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 15178)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 15179 || n = 15183 || n = 15186 || n = 15188 || n = 15189 || n = 15192 || n = 15194)
	; 25..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15180)
	; 29 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 15181)
	; 27 bytes
	ldy #253
	delay_n 10
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15182)
	; 28 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 15184)
	; 25 bytes
	ldy #241
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15185)
	; 28 bytes
	ldy #208
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15187 || n = 15193)
	; 26..27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15190)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 15191)
	; 27 bytes
	ldy #245
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15195)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.endif
.endif
.else
.if((n >= 15196) && (n < 15215))
; 15196 to 15214
.if(n = 15196 || n = 15212 || n = 15213)
	; 26..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 15197)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15198)
	; 28 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15199)
	; 28 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 15200)
	; 28 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15201)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15202)
	; 28 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15203 || n = 15205)
	; 28..29 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 15204)
	; 29 bytes
	delay_n 99
	delay_n (n-99)
.elseif(n = 15206)
	; 28 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15207)
	; 28 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 15208)
	; 28 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15209)
	; 25 bytes
	ldy #234 ;hides 'nop'
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 15210)
	; 26 bytes
	ldy #227
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 15211)
	; 25 bytes
	ldy #234
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15214)
	; 27 bytes
	delay_n (n-3)
	ldy $A4
.endif
.else
; 15215 to 15234
.if(n = 15215 || n = 15219 || n = 15224 || n = 15229 || n = 15234)
	; 27..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15216)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 15217)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 15218)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15220)
	; 28 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 15221)
	; 28 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 15222)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 15223)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15225)
	; 29 bytes
	delay_n 120
	delay_n (n-120)
.elseif(n = 15226)
	; 28 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 15227)
	; 28 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 15228 || n = 15230)
	; 28..29 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15231)
	; 28 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 15232)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 15233)
	; 28 bytes
	ldy #238
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.endif
.endif
.endif
.else
.if((n >= 15235) && (n < 15274))
; 15235 to 15273
.if(n = 15235 || n = 15243 || n = 15245 || n = 15249 || n = 15252 || n = 15253 || (n > 15254 && n < 15258) || n = 15268)
	; 25..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15236)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 15237)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 15238 || n = 15240)
	; 28..29 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 15239)
	; 28 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15241)
	; 27 bytes
	ldy #254
	delay_n 10
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15242 || n = 15264)
	; 28..29 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 15244 || n = 15250 || n = 15254 || n = 15269)
	; 26..29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15246 || n = 15270)
	; 28..29 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15247)
	; 25 bytes
	ldy #242
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15248)
	; 28 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 15251)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 15258)
	; 28 bytes
	ldy #209
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15259)
	; 28 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15260)
	; 29 bytes
	delay_n 155
	delay_n (n-155)
.elseif(n = 15261)
	; 28 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15262)
	; 28 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15263)
	; 28 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15265)
	; 29 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 15266)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15267)
	; 29 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 15271)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 15272)
	; 28 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 15273)
	; 28 bytes
	delay_n 146
	delay_n (n-146)
.endif
.else
.if((n >= 15274) && (n < 15293))
; 15274 to 15292
.if(n = 15274)
	; 29 bytes
	delay_n 169
	delay_n (n-169)
.elseif(n = 15275)
	; 29 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 15276)
	; 25 bytes
	ldy #235
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15277)
	; 26 bytes
	ldy #228
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 15278 || n = 15280 || n = 15282)
	; 26..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 15279)
	; 27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15281)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 15283 || n = 15285)
	; 28..29 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15284)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15286)
	; 28 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 15287)
	; 29 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15288 || n = 15290)
	; 28..29 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 15289)
	; 29 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 15291)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 15292)
	; 29 bytes
	delay_n 41
	delay_n (n-41)
.endif
.else
; 15293 to 15312
.if(n = 15293)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 15294)
	; 28 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 15295)
	; 29 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 15296)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 15297)
	; 28 bytes
	ldy #239
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15298)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 15299 || n = 15303 || n = 15305 || n = 15309)
	; 28..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15300)
	; 28 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15301)
	; 27 bytes
	ldy #255
	delay_n 10
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15302)
	; 29 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 15304)
	; 29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15306)
	; 29 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 15307)
	; 29 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15308)
	; 28 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 15310)
	; 25 bytes
	ldy #243
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15311)
	; 28 bytes
	ldy #24 ;hides 'clc'
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	iny
	Jne *-25
.elseif(n = 15312)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 15313) && (n < 15469))
.if((n >= 15313) && (n < 15391))
.if((n >= 15313) && (n < 15352))
; 15313 to 15351
.if(n = 15313 || n = 15315 || n = 15334 || n = 15340 || n = 15347)
	; 26..30 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15314 || (n > 15315 && n < 15319) || n = 15321 || n = 15329 || n = 15339 || n = 15343 || n = 15345 || n = 15346 || (n > 15347 && n < 15351))
	; 25..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15319)
	; 27 bytes
	ldy #222
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15320)
	; 28 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15322 || n = 15351)
	; 28..29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15323)
	; 28 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15324)
	; 28 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15325)
	; 29 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 15326)
	; 29 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 15327)
	; 29 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 15328)
	; 29 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15330 || n = 15336)
	; 29..30 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 15331)
	; 28 bytes
	ldy #210
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15332)
	; 29 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15333)
	; 29 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15335)
	; 27 bytes
	ldy #216 ;hides 'cld'
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15337)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15338)
	; 29 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15341)
	; 25 bytes
	ldy #236
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15342)
	; 29 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15344)
	; 26 bytes
	ldy #229
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.endif
.else
.if((n >= 15352) && (n < 15371))
; 15352 to 15370
.if(n = 15352)
	; 29 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15353)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15354)
	; 30 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15355)
	; 30 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 15356)
	; 29 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 15357)
	; 29 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15358)
	; 29 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15359)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15360 || n = 15366)
	; 29..30 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 15361)
	; 27 bytes
	ldy #0
	delay_n 10
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15362)
	; 29 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 15363 || n = 15365)
	; 28..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15364)
	; 29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15367)
	; 29 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 15368)
	; 29 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15369)
	; 30 bytes
	ldy #226
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 15370)
	; 30 bytes
	delay_n 186
	delay_n (n-186)
.endif
.else
; 15371 to 15390
.if(n = 15371)
	; 29 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 15372)
	; 29 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 15373)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 15374 || n = 15375 || (n > 15376 && n < 15381) || n = 15390)
	; 25..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15376)
	; 26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15381)
	; 28 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15382)
	; 29 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 15383)
	; 28 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15384)
	; 28 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15385)
	; 28 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15386)
	; 29 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 15387)
	; 29 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 15388)
	; 27 bytes
	ldy #223
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15389)
	; 29 bytes
	delay_n 16
	delay_n (n-16)
.endif
.endif
.endif
.else
.if((n >= 15391) && (n < 15430))
; 15391 to 15429
.if(n = 15391 || n = 15409 || n = 15414)
	; 27..29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15392 || n = 15400 || n = 15405 || n = 15408 || n = 15410 || n = 15412 || n = 15413 || n = 15415 || n = 15420 || n = 15427)
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15393)
	; 29 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15394)
	; 29 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15395)
	; 30 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15396 || n = 15398)
	; 29..30 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 15397)
	; 29 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 15399)
	; 29 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15401)
	; 29 bytes
	ldy #200
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 15402)
	; 29 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15403)
	; 29 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15404)
	; 28 bytes
	ldy #211
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15406)
	; 25 bytes
	ldy #237
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15407)
	; 29 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15411)
	; 26 bytes
	ldy #230
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 15416)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15417)
	; 29 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 15418)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15419)
	; 29 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15421 || n = 15423)
	; 29..30 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 15422)
	; 28 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15424)
	; 29 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 15425)
	; 28 bytes
	ldy #241
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15426 || n = 15428)
	; 29..30 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 15429)
	; 29 bytes
	delay_n 56
	delay_n (n-56)
.endif
.else
.if((n >= 15430) && (n < 15449))
; 15430 to 15448
.if(n = 15430)
	; 30 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 15431 || n = 15433)
	; 29..30 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 15432)
	; 29 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 15434)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 15435 || n = 15438 || (n > 15439 && n < 15443))
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15436)
	; 25 bytes
	ldy #245
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15437)
	; 26 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15439)
	; 27 bytes
	ldy #249
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15443)
	; 28 bytes
	ldy #234 ;hides 'nop'
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 15444)
	; 28 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15445)
	; 28 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15446 || n = 15448)
	; 28..29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15447)
	; 29 bytes
	delay_n 196
	delay_n (n-196)
.endif
.else
; 15449 to 15468
.if(n = 15449)
	; 29 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 15450 || n = 15452)
	; 29..30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15451)
	; 29 bytes
	ldy #206
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 15453)
	; 29 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15454)
	; 29 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15455)
	; 29 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15456 || n = 15458)
	; 29..30 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 15457)
	; 27 bytes
	ldy #224
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15459 || n = 15461)
	; 28..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15460)
	; 29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15462)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15463)
	; 29 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15464)
	; 29 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15465 || n = 15467)
	; 29..30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15466 || n = 15468)
	; 29..30 bytes
	delay_n 156
	delay_n (n-156)
.endif
.endif
.endif
.endif
.else
.if((n >= 15469) && (n < 15547))
.if((n >= 15469) && (n < 15508))
; 15469 to 15507
.if(n = 15469)
	; 29 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15470 || n = 15472)
	; 29..30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15471)
	; 25 bytes
	ldy #238
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15473 || n = 15475 || n = 15476 || n = 15480 || n = 15482 || n = 15491 || n = 15497 || n = 15500 || n = 15501 || n = 15503 || n = 15504)
	; 25..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15474 || n = 15481 || n = 15492 || n = 15498 || n = 15502)
	; 26..30 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15477)
	; 28 bytes
	ldy #212
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15478)
	; 26 bytes
	ldy #231
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 15479)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15483)
	; 28 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15484)
	; 29 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 15485 || n = 15487)
	; 29..30 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 15486)
	; 29 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 15488)
	; 29 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 15489)
	; 28 bytes
	ldy #242
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15490)
	; 29 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15493)
	; 29 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 15494)
	; 29 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 15495)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 15496)
	; 30 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 15499)
	; 25 bytes
	ldy #246
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15505)
	; 28 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15506)
	; 28 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15507)
	; 28 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 15508 to 15546
.if(n = 15508)
	; 29 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 15509)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15510 || n = 15512)
	; 29..30 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 15511)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 15513 || n = 15518 || n = 15523 || n = 15528 || n = 15530 || n = 15533 || n = 15538 || (n > 15539 && n < 15543))
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15514)
	; 29 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15515 || n = 15517)
	; 29..30 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15516)
	; 29 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15519)
	; 29 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 15520 || n = 15522)
	; 29..30 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 15521)
	; 29 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15524)
	; 29 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15525 || n = 15527)
	; 29..30 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15526)
	; 27 bytes
	ldy #225
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15529 || n = 15539 || n = 15546)
	; 27..29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15531)
	; 29 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15532)
	; 30 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15534)
	; 29 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 15535 || n = 15537)
	; 29..30 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15536)
	; 25 bytes
	ldy #239
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15543)
	; 25 bytes
	ldy #24 ;hides 'clc'
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-22
.elseif(n = 15544)
	; 28 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15545)
	; 26 bytes
	ldy #232
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.endif
.endif
.else
.if((n >= 15547) && (n < 15586))
; 15547 to 15585
.if(n = 15547 || n = 15549 || n = 15552 || n = 15555 || n = 15557 || n = 15558 || n = 15560 || n = 15561 || n = 15564 || n = 15566 || n = 15567 || n = 15569 || n = 15579 || n = 15584)
	; 25..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15548 || n = 15559 || n = 15565)
	; 26..28 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15550)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15551)
	; 29 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15553)
	; 28 bytes
	ldy #243
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15554)
	; 29 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 15556)
	; 24 bytes
	ldy #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 15562)
	; 25 bytes
	ldy #247
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15563)
	; 27 bytes
	ldy #251
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15568 || n = 15574)
	; 28..29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15570)
	; 29 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 15571)
	; 29 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 15572)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15573)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15575)
	; 29 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15576)
	; 29 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15577)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 15578)
	; 30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15580)
	; 29 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 15581)
	; 29 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 15582)
	; 29 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15583)
	; 30 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15585)
	; 29 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 15586 to 15625
.if(n = 15586)
	; 29 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15587 || n = 15593)
	; 29..30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15588)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15589 || n = 15594 || n = 15597 || n = 15599 || n = 15603 || (n > 15604 && n < 15609) || n = 15614 || n = 15616 || n = 15618 || n = 15619 || n = 15621 || n = 15622 || n = 15624)
	; 25..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15590)
	; 29 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15591)
	; 29 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15592)
	; 29 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15595)
	; 27 bytes
	ldy #226
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15596)
	; 29 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15598 || n = 15604 || n = 15615 || n = 15620)
	; 26..29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15600)
	; 29 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 15601)
	; 25 bytes
	ldy #240
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15602)
	; 29 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15609)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15610)
	; 29 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 15611)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15612)
	; 26 bytes
	ldy #233
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 15613)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15617)
	; 24 bytes
	ldy #0
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-20
.elseif(n = 15623)
	; 24 bytes
	ldy #248 ;hides 'sed'
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15625)
	; 25 bytes
	ldy #248
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 15626) && (n < 15938))
.if((n >= 15626) && (n < 15782))
.if((n >= 15626) && (n < 15704))
.if((n >= 15626) && (n < 15665))
.if((n >= 15626) && (n < 15645))
; 15626 to 15644
.if(n = 15626 || n = 15628)
	; 26..27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15627 || (n > 15628 && n < 15632))
	; 26..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 15632)
	; 29 bytes
	ldy #203
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 15633)
	; 28 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15634)
	; 28 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15635 || n = 15637)
	; 28..29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15636 || n = 15640)
	; 29..30 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15638 || n = 15644)
	; 29 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15639)
	; 29 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15641)
	; 29 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 15642)
	; 29 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 15643)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.endif
.else
; 15645 to 15664
.if(n = 15645)
	; 30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15646)
	; 29 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15647)
	; 29 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15648 || n = 15654)
	; 29 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15649)
	; 29 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15650)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15651 || n = 15655)
	; 29..30 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15652)
	; 29 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15653 || n = 15659)
	; 29 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15656 || n = 15660)
	; 29..30 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 15657)
	; 29 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15658)
	; 29 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 15661)
	; 29 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 15662)
	; 29 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15663)
	; 29 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15664)
	; 27 bytes
	ldy #227
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.endif
.endif
.else
; 15665 to 15703
.if(n = 15665)
	; 30 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 15666)
	; 25 bytes
	ldy #241
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15667 || n = 15669 || n = 15680 || n = 15682 || n = 15691 || n = 15695)
	; 27..29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15668 || (n > 15669 && n < 15673) || n = 15681 || (n > 15682 && n < 15686) || n = 15689 || n = 15690 || n = 15693 || n = 15694)
	; 26..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15673)
	; 29 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15674)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15675)
	; 30 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 15676)
	; 29 bytes
	ldy #209
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 15677)
	; 25 bytes
	ldy #234 ;hides 'nop'
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 15678 || n = 15700)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15679)
	; 26 bytes
	ldy #234
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 15686)
	; 29 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 15687)
	; 27 bytes
	ldy #253
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15688)
	; 25 bytes
	ldy #249
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15692)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15696)
	; 28 bytes
	ldy #215
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15697)
	; 29 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15698)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15699)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15701)
	; 30 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 15702)
	; 29 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 15703)
	; 29 bytes
	delay_n 86
	delay_n (n-86)
.endif
.endif
.else
.if((n >= 15704) && (n < 15743))
; 15704 to 15742
.if(n = 15704 || n = 15706)
	; 29..30 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15705 || n = 15710 || n = 15711 || n = 15720 || n = 15725 || n = 15733 || (n > 15734 && n < 15738) || n = 15740)
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15707)
	; 29 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15708 || n = 15714 || n = 15716)
	; 29..30 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15709)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 15712)
	; 29 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15713 || n = 15719 || n = 15721)
	; 29..30 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15715 || n = 15726)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15717)
	; 29 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 15718 || n = 15724)
	; 29 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15722)
	; 29 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 15723 || n = 15729)
	; 29 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15727)
	; 29 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 15728)
	; 29 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 15730)
	; 29 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 15731)
	; 25 bytes
	ldy #242
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15732)
	; 29 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 15734)
	; 27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15738)
	; 29 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 15739)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15741)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15742)
	; 29 bytes
	delay_n 186
	delay_n (n-186)
.endif
.else
; 15743 to 15781
.if(n = 15743)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15744)
	; 29 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 15745)
	; 28 bytes
	ldy #246
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15746)
	; 26 bytes
	ldy #235
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 15747 || n = 15748 || n = 15750 || n = 15753 || (n > 15754 && n < 15759) || n = 15760 || n = 15765 || n = 15771)
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15749)
	; 27 bytes
	ldy #254
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15751)
	; 25 bytes
	ldy #250
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15752 || n = 15754 || n = 15766)
	; 27..29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15759)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15761)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15762)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15763)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15764)
	; 29 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15767)
	; 27 bytes
	ldy #216 ;hides 'cld'
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15768 || n = 15774)
	; 29 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15769)
	; 28 bytes
	ldy #216
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15770 || n = 15776)
	; 29 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 15772)
	; 30 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15773 || n = 15779 || n = 15781)
	; 29..30 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15775)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 15777)
	; 30 bytes
	ldy #232
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 15778)
	; 29 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 15780)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.endif
.endif
.endif
.else
.if((n >= 15782) && (n < 15860))
.if((n >= 15782) && (n < 15821))
; 15782 to 15820
.if(n = 15782)
	; 30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15783 || n = 15789)
	; 29 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 15784 || n = 15790)
	; 29 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 15785 || n = 15795 || n = 15798 || n = 15800 || n = 15801 || n = 15804 || n = 15810 || n = 15815 || n = 15816 || (n > 15817 && n < 15821))
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15786)
	; 29 bytes
	ldy #205
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 15787)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15788 || n = 15794)
	; 29 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 15791 || n = 15797)
	; 29 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 15792)
	; 30 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 15793)
	; 29 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 15796)
	; 25 bytes
	ldy #243
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15799 || n = 15805 || n = 15812 || n = 15817)
	; 27..30 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15802)
	; 27 bytes
	ldy #229
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15803)
	; 29 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 15806)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15807)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15808)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15809)
	; 28 bytes
	ldy #247
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15811)
	; 27 bytes
	ldy #255
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15813)
	; 26 bytes
	ldy #236
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 15814)
	; 25 bytes
	ldy #251
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.endif
.else
.if((n >= 15821) && (n < 15840))
; 15821 to 15839
.if(n = 15821 || n = 15828 || n = 15831 || n = 15836 || n = 15838)
	; 28..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 15822)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15823 || n = 15824)
	; 29..30 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15825)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15826)
	; 29 bytes
	ldy #211
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 15827)
	; 30 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 15829)
	; 30 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15830)
	; 30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15832)
	; 30 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15833)
	; 30 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15834)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15835)
	; 30 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15837)
	; 29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15839)
	; 30 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 15840 to 15859
.if(n = 15840)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15841)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 15842)
	; 28 bytes
	ldy #217
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15843 || n = 15844 || n = 15846 || n = 15856)
	; 29..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 15845)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15847)
	; 30 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15848)
	; 30 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15849)
	; 30 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 15850)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15851)
	; 30 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 15852)
	; 30 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15853)
	; 30 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 15854)
	; 30 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 15855)
	; 30 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 15857)
	; 30 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15858)
	; 30 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 15859)
	; 30 bytes
	delay_n 171
	delay_n (n-171)
.endif
.endif
.endif
.else
.if((n >= 15860) && (n < 15899))
; 15860 to 15898
.if(n = 15860)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15861)
	; 25 bytes
	ldy #244
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15862)
	; 30 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 15863 || (n > 15864 && n < 15869) || n = 15875 || n = 15878 || n = 15879 || n = 15881 || n = 15882 || (n > 15883 && n < 15887) || n = 15891 || n = 15894)
	; 26..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 15864 || n = 15874 || n = 15876 || n = 15883)
	; 27..29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15869)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15870)
	; 30 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15871)
	; 27 bytes
	ldy #230
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15872 || n = 15888)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15873)
	; 27 bytes
	ldy #0
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15877)
	; 25 bytes
	ldy #252
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15880)
	; 26 bytes
	ldy #237
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 15887)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15889)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15890)
	; 30 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 15892)
	; 30 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15893 || n = 15896)
	; 30..31 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15895)
	; 30 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15897)
	; 30 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 15898)
	; 30 bytes
	delay_n 21
	delay_n (n-21)
.endif
.else
.if((n >= 15899) && (n < 15918))
; 15899 to 15917
.if(n = 15899)
	; 31 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 15900)
	; 30 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 15901)
	; 29 bytes
	ldy #212
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 15902)
	; 30 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15903)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15904)
	; 30 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 15905)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 15906)
	; 31 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 15907 || n = 15909 || n = 15914 || n = 15917)
	; 28..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 15908)
	; 29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15910)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15911)
	; 29 bytes
	ldy #234 ;hides 'nop'
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 15912)
	; 30 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 15913)
	; 30 bytes
	ldy #234
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 15915)
	; 28 bytes
	ldy #218
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15916)
	; 30 bytes
	delay_n 55
	delay_n (n-55)
.endif
.else
; 15918 to 15937
.if(n = 15918 || n = 15921)
	; 30..31 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 15919 || n = 15924 || n = 15928 || (n > 15929 && n < 15934))
	; 26..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 15920)
	; 30 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15922)
	; 30 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 15923)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15925)
	; 30 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 15926)
	; 25 bytes
	ldy #245
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15927)
	; 30 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 15929)
	; 27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15934)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15935)
	; 30 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 15936)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15937)
	; 28 bytes
	ldy #249
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 15938) && (n < 16094))
.if((n >= 15938) && (n < 16016))
.if((n >= 15938) && (n < 15977))
.if((n >= 15938) && (n < 15957))
; 15938 to 15956
.if(n = 15938 || n = 15952)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15939 || n = 15941 || n = 15942 || (n > 15943 && n < 15947) || n = 15949 || n = 15951 || n = 15953 || n = 15954)
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15940)
	; 25 bytes
	ldy #253
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15943 || n = 15950)
	; 27..28 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15947)
	; 26 bytes
	ldy #238
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 15948)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15955)
	; 30 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15956)
	; 30 bytes
	delay_n 16
	delay_n (n-16)
.endif
.else
; 15957 to 15976
.if(n = 15957 || n = 15971)
	; 30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15958)
	; 30 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15959)
	; 30 bytes
	ldy #202
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 15960)
	; 30 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 15961)
	; 30 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15962)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15963)
	; 30 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 15964)
	; 31 bytes
	delay_n (n-2)
	nop
.elseif(n = 15965)
	; 30 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15966)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15967)
	; 30 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 15968)
	; 30 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15969)
	; 31 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 15970)
	; 30 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15972)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15973)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 15974)
	; 30 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 15975)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15976)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.endif
.endif
.else
; 15977 to 16015
.if(n = 15977)
	; 30 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 15978 || n = 15980 || n = 15984 || n = 15990 || n = 15993 || (n > 15994 && n < 15999) || n = 16005 || n = 16008 || n = 16011)
	; 26..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 15979 || n = 15994 || n = 16004 || n = 16006 || n = 16010 || n = 16012)
	; 27..30 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 15981)
	; 30 bytes
	ldy #235
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 15982)
	; 30 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15983)
	; 30 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15985)
	; 30 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 15986)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15987)
	; 30 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 15988)
	; 28 bytes
	ldy #219
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 15989)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 15991)
	; 25 bytes
	ldy #246
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 15992)
	; 30 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 15999)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16000)
	; 30 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 16001)
	; 28 bytes
	ldy #250
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 16002)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16003)
	; 25 bytes
	ldy #254
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 16007)
	; 26 bytes
	ldy #24 ;hides 'clc'
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-23
.elseif(n = 16009)
	; 27 bytes
	ldy #232
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16013)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16014)
	; 26 bytes
	ldy #239
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 16015)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.else
.if((n >= 16016) && (n < 16055))
.if((n >= 16016) && (n < 16035))
; 16016 to 16034
.if(n = 16016 || (n > 16017 && n < 16021))
	; 27..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 16017)
	; 28 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16021)
	; 30 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16022 || n = 16034)
	; 30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16023)
	; 30 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 16024)
	; 30 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 16025)
	; 30 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16026)
	; 30 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 16027)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16028)
	; 30 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16029)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16030)
	; 30 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 16031)
	; 30 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16032)
	; 30 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 16033)
	; 30 bytes
	delay_n 156
	delay_n (n-156)
.endif
.else
; 16035 to 16054
.if(n = 16035)
	; 30 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 16036)
	; 30 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16037)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16038)
	; 30 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 16039)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16040 || n = 16052)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16041 || n = 16045 || n = 16049)
	; 28..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16042 || n = 16054)
	; 30 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 16043)
	; 30 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 16044)
	; 30 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 16046)
	; 30 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16047)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16048)
	; 30 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 16050)
	; 29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16051)
	; 29 bytes
	ldy #214
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16053)
	; 30 bytes
	delay_n 176
	delay_n (n-176)
.endif
.endif
.else
; 16055 to 16093
.if(n = 16055 || n = 16058 || (n > 16059 && n < 16064) || n = 16067 || n = 16068 || (n > 16069 && n < 16074) || n = 16075 || n = 16080 || n = 16082 || n = 16083 || (n > 16084 && n < 16089))
	; 26..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16056)
	; 25 bytes
	ldy #247
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 16057)
	; 30 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 16059 || n = 16069 || n = 16084)
	; 27..28 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16064 || n = 16074)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16065)
	; 28 bytes
	ldy #251
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 16066)
	; 25 bytes
	ldy #255
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 16076)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16077)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16078)
	; 27 bytes
	ldy #233
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16079)
	; 30 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 16081)
	; 26 bytes
	ldy #240
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 16089)
	; 30 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 16090)
	; 30 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16091)
	; 30 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16092)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16093)
	; 30 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.endif
.else
.if((n >= 16094) && (n < 16172))
.if((n >= 16094) && (n < 16133))
; 16094 to 16132
.if(n = 16094)
	; 29 bytes
	ldy #209
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16095)
	; 31 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 16096)
	; 30 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 16097)
	; 30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16098 || n = 16103 || n = 16107 || n = 16113 || n = 16120 || n = 16123 || (n > 16124 && n < 16129) || n = 16131)
	; 26..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16099 || n = 16110)
	; 30..31 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16100)
	; 30 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16101)
	; 30 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 16102)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16104)
	; 30 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 16105)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16106)
	; 30 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 16108 || n = 16122 || n = 16124 || n = 16132)
	; 27..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16109)
	; 30 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16111)
	; 30 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 16112)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16114)
	; 30 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 16115)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16116)
	; 30 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 16117)
	; 30 bytes
	ldy #237
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 16118)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16119)
	; 25 bytes
	ldy #248 ;hides 'sed'
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 16121)
	; 25 bytes
	ldy #248
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 16129)
	; 25 bytes
	ldy #0
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 16130)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.endif
.else
; 16133 to 16171
.if((n > 16132 && n < 16137) || n = 16138 || n = 16143 || n = 16146 || n = 16149 || n = 16150 || (n > 16151 && n < 16155) || n = 16161 || n = 16166)
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16137)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16139 || n = 16158)
	; 29..30 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16140)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16141)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16142)
	; 30 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 16144)
	; 30 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16145)
	; 26 bytes
	ldy #234 ;hides 'nop'
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16147)
	; 27 bytes
	ldy #234
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16148)
	; 26 bytes
	ldy #241
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 16151)
	; 28 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16155)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16156)
	; 30 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 16157)
	; 30 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16159)
	; 30 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 16160)
	; 30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16162)
	; 30 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16163)
	; 30 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16164)
	; 30 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 16165)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16167)
	; 30 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 16168 || n = 16170)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16169)
	; 30 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 16171)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.endif
.endif
.else
.if((n >= 16172) && (n < 16211))
.if((n >= 16172) && (n < 16191))
; 16172 to 16190
.if(n = 16172)
	; 30 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16173 || n = 16181 || n = 16188 || n = 16190)
	; 26..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16174)
	; 30 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 16175)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16176)
	; 30 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16177)
	; 30 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 16178)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16179)
	; 30 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 16180)
	; 30 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 16182)
	; 30 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 16183)
	; 31 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16184)
	; 30 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 16185)
	; 30 bytes
	ldy #238
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 16186)
	; 25 bytes
	ldy #249
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 16187)
	; 30 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 16189)
	; 27 bytes
	delay_n (n-3)
	ldy $A4
.endif
.else
; 16191 to 16210
.if(n = 16191 || n = 16192 || n = 16195 || n = 16203 || n = 16209)
	; 28..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 16193)
	; 28 bytes
	ldy #253
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 16194)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16196)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16197)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16198)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16199)
	; 28 bytes
	ldy #216 ;hides 'cld'
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16200)
	; 30 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 16201)
	; 29 bytes
	ldy #216
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16202)
	; 30 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 16204)
	; 31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16205)
	; 30 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 16206)
	; 31 bytes
	delay_n 85
	delay_n (n-85)
.elseif(n = 16207)
	; 28 bytes
	ldy #222
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 16208)
	; 31 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16210)
	; 30 bytes
	delay_n 81
	delay_n (n-81)
.endif
.endif
.else
.if((n >= 16211) && (n < 16231))
; 16211 to 16230
.if(n = 16211 || (n > 16216 && n < 16222) || n = 16224 || n = 16229)
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16212)
	; 30 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 16213)
	; 31 bytes
	delay_n 92
	delay_n (n-92)
.elseif(n = 16214)
	; 31 bytes
	delay_n 85
	delay_n (n-85)
.elseif(n = 16215)
	; 26 bytes
	ldy #242
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 16216)
	; 27 bytes
	ldy #235
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16222)
	; 30 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 16223)
	; 30 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 16225)
	; 30 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16226)
	; 30 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16227)
	; 30 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 16228)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16230)
	; 30 bytes
	delay_n 101
	delay_n (n-101)
.endif
.else
; 16231 to 16250
.if(n = 16231)
	; 31 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16232)
	; 30 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 16233)
	; 30 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 16234 || n = 16239 || n = 16244)
	; 30..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16235)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16236)
	; 31 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 16237)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16238)
	; 31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16240)
	; 30 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 16241)
	; 30 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16242)
	; 30 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 16243)
	; 31 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 16245)
	; 30 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 16246)
	; 31 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16247)
	; 30 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 16248)
	; 29 bytes
	ldy #211
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16249)
	; 31 bytes
	delay_n 120
	delay_n (n-120)
.elseif(n = 16250)
	; 30 bytes
	delay_n 121
	delay_n (n-121)
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 16251) && (n < 16876))
.if((n >= 16251) && (n < 16563))
.if((n >= 16251) && (n < 16407))
.if((n >= 16251) && (n < 16329))
.if((n >= 16251) && (n < 16290))
; 16251 to 16289
.if(n = 16251)
	; 25 bytes
	ldy #250
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 16252)
	; 30 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 16253 || n = 16255 || n = 16256 || n = 16258 || n = 16259 || n = 16262 || n = 16264 || n = 16266 || n = 16278 || n = 16284 || n = 16286 || n = 16287 || n = 16289)
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 16254 || n = 16263 || n = 16279 || n = 16283 || n = 16288)
	; 27..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16257)
	; 28 bytes
	ldy #254
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 16260)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16261)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16265)
	; 30 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 16267)
	; 30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16268)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16269)
	; 31 bytes
	ldy #196
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16270)
	; 30 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16271)
	; 31 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 16272)
	; 30 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 16273)
	; 31 bytes
	ldy #226
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16274)
	; 30 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 16275)
	; 30 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 16276)
	; 29 bytes
	ldy #217
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16277)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16280)
	; 28 bytes
	ldy #223
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 16281)
	; 31 bytes
	ldy #220
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16282)
	; 26 bytes
	ldy #243
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 16285)
	; 27 bytes
	ldy #236
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.endif
.else
.if((n >= 16290) && (n < 16309))
; 16290 to 16308
.if(n = 16290)
	; 30 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 16291 || n = 16299 || n = 16304 || n = 16305)
	; 30..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16292)
	; 30 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 16293)
	; 30 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16294)
	; 30 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16295)
	; 30 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 16296)
	; 30 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 16297)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16298)
	; 31 bytes
	delay_n 169
	delay_n (n-169)
.elseif(n = 16300)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16301)
	; 31 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 16302)
	; 30 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 16303)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16306)
	; 30 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16307)
	; 30 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 16308)
	; 31 bytes
	delay_n 26
	delay_n (n-26)
.endif
.else
; 16309 to 16328
.if(n = 16309 || n = 16314 || n = 16318 || n = 16320 || n = 16322 || n = 16323)
	; 26..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16310)
	; 30 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 16311)
	; 31 bytes
	ldy #233
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16312)
	; 30 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 16313)
	; 31 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16315)
	; 30 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 16316)
	; 25 bytes
	ldy #251
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 16317)
	; 30 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 16319)
	; 27 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16321)
	; 28 bytes
	ldy #255
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-24
.elseif(n = 16324)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16325)
	; 29 bytes
	ldy #212
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16326)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16327)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16328)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.endif
.else
.if((n >= 16329) && (n < 16368))
; 16329 to 16367
.if(n = 16329 || n = 16330 || n = 16333 || n = 16335 || n = 16336 || n = 16344 || n = 16351 || n = 16355 || n = 16356 || (n > 16357 && n < 16361))
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16331)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16332)
	; 30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16334 || n = 16352 || n = 16357)
	; 28..29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16337)
	; 30 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 16338)
	; 31 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16339)
	; 30 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 16340)
	; 31 bytes
	delay_n 89
	delay_n (n-89)
.elseif(n = 16341 || n = 16364)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16342)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16343 || n = 16361 || n = 16366)
	; 30..31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16345)
	; 31 bytes
	ldy #227
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16346)
	; 31 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 16347)
	; 30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16348)
	; 31 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 16349)
	; 26 bytes
	ldy #244
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 16350)
	; 30 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16353)
	; 28 bytes
	ldy #224
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 16354)
	; 27 bytes
	ldy #237
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16362)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16363)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16365)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16367)
	; 28 bytes
	ldy #248 ;hides 'sed'
	nop
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.endif
.else
; 16368 to 16406
.if(n = 16368)
	; 31 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 16369)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16370 || n = 16384 || n = 16396 || n = 16405)
	; 27..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16371 || n = 16373 || n = 16374 || n = 16383 || (n > 16384 && n < 16389) || n = 16390 || n = 16394 || n = 16395 || n = 16399 || n = 16404 || n = 16406)
	; 26..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16372)
	; 30 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 16375)
	; 31 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16376)
	; 31 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 16377)
	; 30 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 16378)
	; 31 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16379)
	; 30 bytes
	ldy #234 ;hides 'nop'
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16380)
	; 31 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16381)
	; 25 bytes
	ldy #252
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 16382)
	; 30 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 16389)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16391)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16392)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16393)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16397)
	; 30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16398)
	; 30 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16400)
	; 31 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 16401)
	; 31 bytes
	delay_n 85
	delay_n (n-85)
.elseif(n = 16402)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16403)
	; 31 bytes
	delay_n 22
	delay_n (n-22)
.endif
.endif
.endif
.else
.if((n >= 16407) && (n < 16485))
.if((n >= 16407) && (n < 16446))
.if((n >= 16407) && (n < 16426))
; 16407 to 16425
.if(n = 16407)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16408 || n = 16409 || n = 16418 || (n > 16419 && n < 16423) || n = 16425)
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16410)
	; 31 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 16411)
	; 31 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 16412)
	; 30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16413)
	; 31 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 16414)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16415)
	; 30 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16416)
	; 26 bytes
	ldy #245
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 16417)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16419)
	; 28 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16423)
	; 27 bytes
	ldy #238
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16424)
	; 30 bytes
	delay_n (n-8)
	delay_n 8
.endif
.else
; 16426 to 16445
.if(n = 16426)
	; 28 bytes
	ldy #225
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 16427 || n = 16428 || n = 16434 || n = 16437 || n = 16439)
	; 29..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16429 || n = 16438)
	; 30..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16430)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16431)
	; 31 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 16432)
	; 30 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 16433)
	; 30 bytes
	ldy #208
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 16435)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16436)
	; 30 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 16440)
	; 31 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16441)
	; 31 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 16442)
	; 30 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 16443)
	; 31 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16444)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16445)
	; 31 bytes
	delay_n 64
	delay_n (n-64)
.endif
.endif
.else
; 16446 to 16484
.if(n = 16446)
	; 25 bytes
	ldy #253
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 16447)
	; 30 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 16448 || (n > 16449 && n < 16454) || n = 16455 || n = 16459 || n = 16460 || n = 16464 || n = 16475 || n = 16477 || n = 16478 || n = 16481 || n = 16484)
	; 26..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16449 || n = 16461 || n = 16474 || n = 16476)
	; 27..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16454)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16456)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16457)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16458)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16462)
	; 30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16463)
	; 31 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 16465)
	; 30 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16466)
	; 31 bytes
	delay_n 85
	delay_n (n-85)
.elseif(n = 16467)
	; 30 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 16468)
	; 31 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16469)
	; 30 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 16470)
	; 31 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 16471)
	; 27 bytes
	ldy #24 ;hides 'clc'
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	iny
	Jne *-24
.elseif(n = 16472)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16473)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16479)
	; 29 bytes
	ldy #214
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16480)
	; 30 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16482)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16483)
	; 26 bytes
	ldy #246
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.endif
.endif
.else
.if((n >= 16485) && (n < 16524))
; 16485 to 16523
.if(n = 16485 || (n > 16486 && n < 16491) || n = 16494 || (n > 16495 && n < 16499) || n = 16503 || n = 16505 || n = 16513 || (n > 16514 && n < 16519) || n = 16520)
	; 26..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16486 || n = 16495 || n = 16504 || n = 16514)
	; 27..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16491 || n = 16519)
	; 29..30 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16492)
	; 27 bytes
	ldy #239
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16493 || n = 16521)
	; 29..30 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16499)
	; 28 bytes
	ldy #226
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 16500)
	; 31 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16501)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16502)
	; 30 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 16506)
	; 31 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 16507)
	; 30 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 16508)
	; 31 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16509)
	; 31 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16510)
	; 31 bytes
	delay_n 64
	delay_n (n-64)
.elseif(n = 16511)
	; 25 bytes
	ldy #254
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 16512)
	; 30 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 16522)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16523)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 16524 to 16562
.if(n = 16524 || n = 16529 || n = 16539 || n = 16546 || n = 16548 || n = 16549 || n = 16551 || n = 16552 || n = 16554 || n = 16555 || n = 16558)
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16525)
	; 30 bytes
	ldy #243
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 16526 || n = 16528 || n = 16547 || n = 16553 || n = 16559)
	; 28..32 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16527)
	; 30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16530)
	; 31 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 16531)
	; 31 bytes
	delay_n 85
	delay_n (n-85)
.elseif(n = 16532)
	; 30 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 16533)
	; 31 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16534)
	; 30 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 16535)
	; 31 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 16536)
	; 31 bytes
	delay_n 155
	delay_n (n-155)
.elseif(n = 16537)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16538)
	; 31 bytes
	delay_n 92
	delay_n (n-92)
.elseif(n = 16540)
	; 31 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 16541 || n = 16560)
	; 29..31 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16542)
	; 30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16543)
	; 31 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 16544)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16545)
	; 30 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16550)
	; 26 bytes
	ldy #247
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 16556)
	; 29 bytes
	ldy #215
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16557)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16561)
	; 27 bytes
	ldy #240
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16562)
	; 30 bytes
	delay_n 51
	delay_n (n-51)
.endif
.endif
.endif
.endif
.else
.if((n >= 16563) && (n < 16719))
.if((n >= 16563) && (n < 16641))
.if((n >= 16563) && (n < 16602))
; 16563 to 16601
.if(n = 16563 || n = 16565 || n = 16566 || n = 16568 || n = 16569 || n = 16574 || n = 16578 || (n > 16579 && n < 16584) || n = 16585 || n = 16589 || n = 16590 || n = 16594 || n = 16595)
	; 26..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16564 || n = 16570 || n = 16575 || n = 16579)
	; 27..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16567)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16571 || n = 16586)
	; 29..31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16572)
	; 28 bytes
	ldy #227
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 16573 || n = 16588)
	; 29..31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16576)
	; 25 bytes
	ldy #255
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 16577)
	; 30 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 16584)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16587)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16591)
	; 30 bytes
	ldy #210
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 16592)
	; 30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16593)
	; 30 bytes
	ldy #244
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 16596)
	; 31 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16597)
	; 30 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 16598)
	; 31 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16599)
	; 30 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16600)
	; 31 bytes
	delay_n 89
	delay_n (n-89)
.elseif(n = 16601)
	; 31 bytes
	ldy #200
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.endif
.else
; 16602 to 16640
.if(n = 16602)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16603)
	; 31 bytes
	delay_n 92
	delay_n (n-92)
.elseif(n = 16604 || n = 16609 || n = 16614 || n = 16619 || (n > 16620 && n < 16625) || n = 16632 || (n > 16633 && n < 16637) || n = 16639)
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16605)
	; 31 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 16606)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16607)
	; 30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16608)
	; 31 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 16610)
	; 30 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16611 || n = 16637)
	; 30..31 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 16612)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16613)
	; 27 bytes
	ldy #234 ;hides 'nop'
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 16615)
	; 25 bytes
	ldy #248 ;hides 'sed'
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 16616 || n = 16618 || n = 16620)
	; 27..29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16617)
	; 26 bytes
	ldy #248
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 16625)
	; 29 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16626 || n = 16628)
	; 29..30 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16627 || n = 16629)
	; 29..30 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16630)
	; 27 bytes
	ldy #241
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16631)
	; 29 bytes
	ldy #216 ;hides 'cld'
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 16633)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16638)
	; 31 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 16640)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 16641) && (n < 16680))
; 16641 to 16679
.if(n = 16641)
	; 25 bytes
	ldy #0
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-21
.elseif(n = 16642)
	; 30 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 16643 || (n > 16644 && n < 16649) || n = 16650 || n = 16654 || n = 16655 || n = 16659 || n = 16663 || n = 16669 || n = 16674 || n = 16679)
	; 26..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16644 || n = 16660)
	; 27..32 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16649)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16651)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16652)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16653)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16656 || n = 16658)
	; 30..31 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 16657)
	; 30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16661)
	; 30 bytes
	ldy #245
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 16662)
	; 30 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 16664 || n = 16666)
	; 29..30 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16665)
	; 31 bytes
	delay_n 89
	delay_n (n-89)
.elseif(n = 16667)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16668)
	; 31 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 16670)
	; 30 bytes
	ldy #211
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 16671 || n = 16673)
	; 30..31 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 16672)
	; 30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16675)
	; 30 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 16676)
	; 30 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 16677)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16678)
	; 30 bytes
	delay_n 167
	delay_n (n-167)
.endif
.else
; 16680 to 16718
.if(n = 16680)
	; 31 bytes
	delay_n 169
	delay_n (n-169)
.elseif(n = 16681 || n = 16683 || n = 16707)
	; 30..31 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 16682)
	; 30 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 16684)
	; 26 bytes
	ldy #249
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 16685)
	; 30 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 16686)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16687 || n = 16689 || n = 16702 || n = 16713)
	; 28..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16688 || n = 16690 || n = 16691 || n = 16701 || (n > 16702 && n < 16706) || n = 16714)
	; 28..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16692)
	; 30 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 16693 || n = 16717)
	; 30..31 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 16694 || n = 16709)
	; 30..31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16695)
	; 30 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16696 || n = 16698)
	; 30..31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16697)
	; 30 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 16699)
	; 27 bytes
	ldy #242
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16700)
	; 31 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16706 || n = 16708)
	; 30..31 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16710)
	; 29 bytes
	ldy #217
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16711)
	; 30 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16712)
	; 30 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 16715)
	; 31 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16716)
	; 30 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 16718)
	; 28 bytes
	ldy #229
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.endif
.endif
.endif
.else
.if((n >= 16719) && (n < 16797))
.if((n >= 16719) && (n < 16758))
.if((n >= 16719) && (n < 16738))
; 16719 to 16737
.if(n = 16719 || n = 16720 || n = 16724 || n = 16728 || n = 16734)
	; 29..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16721)
	; 30 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16722)
	; 30 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16723)
	; 31 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16725)
	; 31 bytes
	ldy #226
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16726)
	; 29 bytes
	ldy #223
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16727)
	; 30 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 16729)
	; 30 bytes
	ldy #246
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 16730)
	; 31 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16731)
	; 30 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 16732)
	; 30 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16733)
	; 30 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16735)
	; 31 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 16736)
	; 30 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 16737)
	; 30 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 16738 to 16757
.if(n = 16738)
	; 31 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 16739 || n = 16744 || n = 16753 || n = 16755 || n = 16756)
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16740)
	; 31 bytes
	delay_n 99
	delay_n (n-99)
.elseif(n = 16741)
	; 30 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 16742)
	; 30 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 16743)
	; 30 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 16745)
	; 31 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 16746 || n = 16748)
	; 30..31 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 16747)
	; 30 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16749)
	; 30 bytes
	ldy #212
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 16750)
	; 30 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 16751)
	; 26 bytes
	ldy #250
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 16752)
	; 30 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 16754)
	; 28 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16757)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.endif
.endif
.else
; 16758 to 16796
.if(n = 16758 || n = 16759 || n = 16761 || n = 16764 || n = 16767 || n = 16769 || n = 16770 || (n > 16771 && n < 16776) || n = 16779 || n = 16784 || n = 16789 || n = 16793 || n = 16795)
	; 28..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16760 || n = 16771 || n = 16790 || n = 16794)
	; 29..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16762)
	; 30 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 16763 || n = 16780)
	; 30..31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16765)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16766 || n = 16792)
	; 30 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16768)
	; 27 bytes
	ldy #243
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16776)
	; 30 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 16777)
	; 30 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 16778)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16781 || n = 16783)
	; 30..31 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 16782)
	; 30 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16785)
	; 31 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 16786 || n = 16788)
	; 30..31 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 16787)
	; 29 bytes
	ldy #218
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16791)
	; 28 bytes
	ldy #230
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 16796)
	; 30 bytes
	delay_n 181
	delay_n (n-181)
.endif
.endif
.else
.if((n >= 16797) && (n < 16836))
; 16797 to 16835
.if(n = 16797)
	; 30 bytes
	ldy #247
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 16798)
	; 31 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 16799)
	; 31 bytes
	ldy #227
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16800)
	; 30 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16801)
	; 29 bytes
	ldy #224
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16802)
	; 30 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 16803 || n = 16805 || n = 16809 || n = 16814 || n = 16819 || n = 16820 || (n > 16821 && n < 16826) || n = 16830 || (n > 16831 && n < 16836))
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16804 || n = 16821)
	; 28..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16806)
	; 30 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 16807)
	; 30 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 16808)
	; 30 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 16810)
	; 31 bytes
	delay_n 169
	delay_n (n-169)
.elseif(n = 16811 || n = 16813)
	; 30..31 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 16812)
	; 30 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 16815)
	; 30 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 16816 || n = 16827)
	; 30..32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 16817)
	; 30 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 16818)
	; 26 bytes
	ldy #251
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 16826)
	; 30 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16828)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16829)
	; 30 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16831)
	; 29 bytes
	ldy #(n-1)/66
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.endif
.else
.if((n >= 16836) && (n < 16856))
; 16836 to 16855
.if(n = 16836)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16837)
	; 27 bytes
	ldy #244
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16838)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16839 || (n > 16840 && n < 16845) || n = 16846)
	; 28..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 16840)
	; 29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16845)
	; 31 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 16847)
	; 31 bytes
	ldy #234 ;hides 'nop'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 16848)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16849)
	; 31 bytes
	ldy #234
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16850)
	; 31 bytes
	ldy #203
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16851)
	; 31 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 16852)
	; 31 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 16853)
	; 32 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16854)
	; 31 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16855)
	; 31 bytes
	delay_n 171
	delay_n (n-171)
.endif
.else
; 16856 to 16875
.if(n = 16856 || n = 16861 || n = 16866 || n = 16868)
	; 29..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 16857)
	; 31 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16858)
	; 31 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 16859)
	; 31 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 16860)
	; 31 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 16862)
	; 31 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 16863)
	; 29 bytes
	ldy #248 ;hides 'sed'
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 16864)
	; 28 bytes
	ldy #231
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 16865)
	; 30 bytes
	ldy #248
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 16867)
	; 30 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16869)
	; 31 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 16870)
	; 31 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 16871)
	; 31 bytes
	ldy #241
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16872)
	; 31 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 16873)
	; 31 bytes
	ldy #228
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16874)
	; 31 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 16875)
	; 31 bytes
	delay_n 191
	delay_n (n-191)
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 16876) && (n < 17188))
.if((n >= 16876) && (n < 17032))
.if((n >= 16876) && (n < 16954))
.if((n >= 16876) && (n < 16915))
; 16876 to 16914
.if(n = 16876)
	; 29 bytes
	ldy #225
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16877)
	; 31 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 16878 || n = 16880 || n = 16881 || n = 16887 || (n > 16888 && n < 16893) || n = 16898 || n = 16901 || (n > 16902 && n < 16906) || n = 16908 || (n > 16909 && n < 16914))
	; 27..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 16879 || n = 16888 || n = 16900 || n = 16902 || n = 16909)
	; 28..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16882)
	; 31 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 16883)
	; 32 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16884)
	; 31 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 16885)
	; 26 bytes
	ldy #252
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 16886)
	; 31 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16893)
	; 30 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16894)
	; 31 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 16895)
	; 30 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16896)
	; 30 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16897)
	; 29 bytes
	ldy #0
	delay_n 4
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16899)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16906)
	; 27 bytes
	ldy #245
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16907)
	; 30 bytes
	ldy #214
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 16914)
	; 31 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
.if((n >= 16915) && (n < 16934))
; 16915 to 16933
.if(n = 16915)
	; 32 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16916)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16917)
	; 31 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 16918)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16919)
	; 31 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 16920)
	; 32 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 16921)
	; 31 bytes
	ldy #235
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16922)
	; 31 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 16923)
	; 32 bytes
	delay_n (n-2)
	nop
.elseif(n = 16924)
	; 31 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16925)
	; 31 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 16926)
	; 31 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 16927)
	; 31 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 16928)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16929)
	; 31 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 16930)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16931)
	; 31 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16932)
	; 31 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 16933)
	; 30 bytes
	ldy #249
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.endif
.else
; 16934 to 16953
.if(n = 16934 || n = 16948)
	; 30..31 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 16935)
	; 27 bytes
	ldy #24 ;hides 'clc'
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	iny
	Jne *-24
.elseif(n = 16936 || n = 16950)
	; 31..32 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 16937)
	; 28 bytes
	ldy #232
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 16938 || n = 16940 || n = 16944)
	; 29..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16939 || n = 16943 || n = 16945 || n = 16953)
	; 29..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16941)
	; 29 bytes
	ldy #220
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16942)
	; 31 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 16946)
	; 31 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 16947)
	; 31 bytes
	ldy #229
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16949)
	; 31 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 16951)
	; 29 bytes
	ldy #226
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 16952)
	; 26 bytes
	ldy #253
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.endif
.endif
.endif
.else
.if((n >= 16954) && (n < 16993))
; 16954 to 16992
.if(n = 16954 || (n > 16955 && n < 16960) || n = 16965 || n = 16972 || n = 16974 || n = 16976 || n = 16977 || (n > 16978 && n < 16982))
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16955 || n = 16973 || n = 16978)
	; 28..29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 16960)
	; 30 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16961)
	; 31 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 16962 || n = 16985)
	; 30..31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16963)
	; 30 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16964 || n = 16982 || n = 16987)
	; 30..31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16966)
	; 31 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16967)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 16968)
	; 31 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16969)
	; 31 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16970)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16971)
	; 31 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 16975)
	; 27 bytes
	ldy #246
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 16983)
	; 31 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16984)
	; 31 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 16986)
	; 30 bytes
	ldy #215
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 16988)
	; 31 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16989)
	; 31 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 16990)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16991)
	; 31 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16992)
	; 31 bytes
	delay_n 174
	delay_n (n-174)
.endif
.else
.if((n >= 16993) && (n < 17012))
; 16993 to 17011
.if(n = 16993)
	; 31 bytes
	ldy #236
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 16994)
	; 31 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 16995 || n = 17003 || n = 17005)
	; 31..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 16996)
	; 31 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 16997)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16998)
	; 31 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16999)
	; 31 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17000)
	; 32 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 17001)
	; 30 bytes
	ldy #250
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 17002)
	; 32 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17004)
	; 31 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17006)
	; 31 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 17007)
	; 31 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17008)
	; 31 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 17009)
	; 31 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17010)
	; 28 bytes
	ldy #233
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 17011)
	; 31 bytes
	ldy #243
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.endif
.else
; 17012 to 17031
.if(n = 17012 || n = 17014 || n = 17015 || n = 17020 || n = 17021 || (n > 17022 && n < 17026) || n = 17028)
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17013 || n = 17022)
	; 28..30 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17016)
	; 31 bytes
	ldy #205
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17017)
	; 32 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 17018)
	; 29 bytes
	ldy #221
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17019)
	; 26 bytes
	ldy #254
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 17026)
	; 29 bytes
	ldy #227
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17027)
	; 30 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17029)
	; 30 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17030)
	; 30 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 17031)
	; 30 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.endif
.endif
.else
.if((n >= 17032) && (n < 17110))
.if((n >= 17032) && (n < 17071))
.if((n >= 17032) && (n < 17051))
; 17032 to 17050
.if(n = 17032 || n = 17043 || n = 17045 || n = 17046 || (n > 17047 && n < 17051))
	; 28..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17033)
	; 31 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17034 || n = 17047)
	; 29..32 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17035)
	; 31 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17036)
	; 31 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17037)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17038)
	; 31 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 17039)
	; 32 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 17040)
	; 31 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 17041)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17042)
	; 32 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 17044)
	; 27 bytes
	ldy #247
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.endif
.else
; 17051 to 17070
.if(n = 17051 || n = 17054)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17052)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17053 || n = 17056)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17055)
	; 31 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17057)
	; 32 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17058)
	; 31 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17059)
	; 31 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 17060)
	; 31 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 17061)
	; 31 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 17062)
	; 32 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 17063)
	; 29 bytes
	ldy #216 ;hides 'cld'
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 17064)
	; 32 bytes
	delay_n 179
	delay_n (n-179)
.elseif(n = 17065)
	; 30 bytes
	ldy #216
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 17066)
	; 31 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17067)
	; 31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17068)
	; 30 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 17069)
	; 30 bytes
	ldy #251
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 17070)
	; 31 bytes
	delay_n 51
	delay_n (n-51)
.endif
.endif
.else
; 17071 to 17109
.if(n = 17071 || n = 17085 || n = 17087 || n = 17088 || (n > 17089 && n < 17094) || n = 17097 || n = 17103 || n = 17105 || n = 17109)
	; 27..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17072 || n = 17084 || n = 17089 || n = 17104)
	; 28..32 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17073)
	; 31 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 17074)
	; 31 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17075)
	; 31 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 17076)
	; 31 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17077)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17078)
	; 31 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 17079)
	; 32 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 17080)
	; 31 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 17081)
	; 27 bytes
	ldy #234 ;hides 'nop'
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 17082)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 17083)
	; 28 bytes
	ldy #234
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 17086)
	; 26 bytes
	ldy #255
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 17094)
	; 30 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17095)
	; 29 bytes
	ldy #222
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17096)
	; 30 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17098)
	; 30 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17099)
	; 31 bytes
	ldy #206
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17100)
	; 31 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17101)
	; 29 bytes
	ldy #228
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17102)
	; 31 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17106)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17107)
	; 31 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 17108)
	; 31 bytes
	delay_n 156
	delay_n (n-156)
.endif
.endif
.else
.if((n >= 17110) && (n < 17149))
.if((n >= 17110) && (n < 17129))
; 17110 to 17128
.if(n = 17110)
	; 31 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17111)
	; 26 bytes
	ldy #248 ;hides 'sed'
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17112)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17113)
	; 27 bytes
	ldy #248
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17114 || n = 17115 || (n > 17116 && n < 17121))
	; 28..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17116)
	; 29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17121)
	; 30 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17122)
	; 30 bytes
	delay_n 11
	delay_n (n-11)
.elseif((n > 17122 && n < 17126))
	; 30..31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17126)
	; 31 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 17127)
	; 31 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 17128)
	; 31 bytes
	delay_n 176
	delay_n (n-176)
.endif
.else
; 17129 to 17148
.if(n = 17129)
	; 32 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17130)
	; 31 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 17131)
	; 32 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 17132)
	; 31 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17133)
	; 31 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17134)
	; 32 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 17135)
	; 30 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 17136)
	; 32 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 17137)
	; 30 bytes
	ldy #252
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 17138)
	; 31 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17139 || n = 17146)
	; 31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17140)
	; 31 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 17141)
	; 31 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17142)
	; 31 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 17143)
	; 31 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17144)
	; 30 bytes
	ldy #217
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 17145)
	; 31 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 17147)
	; 31 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 17148)
	; 31 bytes
	delay_n 196
	delay_n (n-196)
.endif
.endif
.else
; 17149 to 17187
.if(n = 17149)
	; 32 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17150)
	; 31 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17151)
	; 31 bytes
	ldy #245
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17152)
	; 31 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 17153)
	; 26 bytes
	ldy #0
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-22
.elseif(n = 17154)
	; 32 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 17155 || (n > 17156 && n < 17161) || n = 17166 || n = 17171 || n = 17174 || n = 17178 || n = 17180 || n = 17181 || n = 17184 || n = 17185 || n = 17187)
	; 27..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17156 || n = 17168 || n = 17175 || n = 17179 || n = 17186)
	; 28..32 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17161)
	; 30 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17162)
	; 31 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 17163)
	; 30 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17164)
	; 30 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 17165)
	; 30 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17167)
	; 31 bytes
	ldy #24 ;hides 'clc'
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	iny
	Jne *-28
.elseif(n = 17169)
	; 31 bytes
	ldy #232
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17170)
	; 31 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17172)
	; 29 bytes
	ldy #223
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17173)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17176)
	; 29 bytes
	ldy #229
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17177)
	; 31 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17182)
	; 27 bytes
	ldy #249
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17183)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.endif
.endif
.endif
.endif
.else
.if((n >= 17188) && (n < 17344))
.if((n >= 17188) && (n < 17266))
.if((n >= 17188) && (n < 17227))
.if((n >= 17188) && (n < 17207))
; 17188 to 17206
.if(n = 17188 || n = 17189 || n = 17196 || n = 17201 || n = 17206)
	; 30..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17190)
	; 31 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17191)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17192 || n = 17193)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17194 || n = 17195)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17197)
	; 31 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 17198)
	; 32 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17199)
	; 31 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17200)
	; 31 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17202)
	; 30 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 17203)
	; 32 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 17204)
	; 31 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 17205)
	; 30 bytes
	ldy #253
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.endif
.else
; 17207 to 17226
.if(n = 17207 || n = 17211 || n = 17216 || n = 17225)
	; 31..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17208)
	; 31 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17209)
	; 31 bytes
	ldy #239
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17210)
	; 31 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17212)
	; 31 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 17213)
	; 32 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17214)
	; 31 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 17215)
	; 31 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 17217)
	; 31 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17218)
	; 32 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17219)
	; 31 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 17220)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 17221)
	; 31 bytes
	ldy #246
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17222)
	; 31 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 17223)
	; 30 bytes
	ldy #218
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 17224)
	; 31 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 17226)
	; 32 bytes
	delay_n (n-3)
	ldy $A4
.endif
.endif
.else
; 17227 to 17265
.if(n = 17227)
	; 31 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17228)
	; 32 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17229)
	; 28 bytes
	ldy #236
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 17230)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17231 || n = 17233 || n = 17235 || n = 17236 || n = 17245 || n = 17246 || n = 17250 || n = 17253 || n = 17255 || n = 17256 || (n > 17257 && n < 17261))
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17232 || n = 17252 || n = 17257)
	; 29..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17234)
	; 31 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17237)
	; 31 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17238)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 17239)
	; 31 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 17240)
	; 32 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 17241 || n = 17263)
	; 31..32 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17242)
	; 31 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 17243)
	; 31 bytes
	ldy #233
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17244)
	; 31 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17247)
	; 31 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 17248)
	; 32 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 17249)
	; 29 bytes
	ldy #224
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17251)
	; 27 bytes
	ldy #250
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17254)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17261 || n = 17264)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17262)
	; 31 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 17265)
	; 31 bytes
	ldy #208
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.endif
.endif
.else
.if((n >= 17266) && (n < 17305))
.if((n >= 17266) && (n < 17285))
; 17266 to 17284
.if(n = 17266)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17267)
	; 31 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17268)
	; 32 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 17269)
	; 31 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 17270)
	; 32 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17271 || n = 17275 || n = 17283)
	; 31..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17272)
	; 31 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17273)
	; 30 bytes
	ldy #254
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 17274)
	; 31 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 17276)
	; 32 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17277)
	; 31 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17278)
	; 31 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 17279)
	; 31 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 17280)
	; 32 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17281)
	; 31 bytes
	ldy #240
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17282)
	; 31 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 17284)
	; 31 bytes
	delay_n 131
	delay_n (n-131)
.endif
.else
; 17285 to 17304
.if(n = 17285)
	; 31 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 17286 || n = 17293 || n = 17296 || n = 17301 || n = 17304)
	; 29..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17287)
	; 31 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 17288)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17289)
	; 31 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 17290)
	; 32 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17291)
	; 31 bytes
	ldy #247
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17292)
	; 31 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17294)
	; 31 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17295)
	; 32 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 17297)
	; 31 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17298)
	; 32 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 17299)
	; 31 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 17300 || n = 17303)
	; 31 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 17302)
	; 28 bytes
	ldy #237
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.endif
.endif
.else
; 17305 to 17343
.if(n = 17305 || n = 17323 || n = 17328)
	; 29..30 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 17305 && n < 17309) || n = 17311 || n = 17316 || n = 17321 || n = 17322 || n = 17324 || n = 17327 || (n > 17328 && n < 17332) || n = 17343)
	; 28..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17309 || n = 17338)
	; 31..32 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 17310)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 17312)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 17313)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17314)
	; 31 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 17315)
	; 31 bytes
	ldy #234 ;hides 'nop'
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 17317)
	; 31 bytes
	ldy #234
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17318)
	; 32 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 17319)
	; 31 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 17320)
	; 27 bytes
	ldy #251
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17325)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17326)
	; 29 bytes
	ldy #231
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17332 || n = 17337)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17333)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17334)
	; 31 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17335)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17336)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 17339)
	; 31 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17340)
	; 32 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 17341)
	; 30 bytes
	ldy #255
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 17342)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.endif
.else
.if((n >= 17344) && (n < 17422))
.if((n >= 17344) && (n < 17383))
.if((n >= 17344) && (n < 17363))
; 17344 to 17362
.if(n = 17344)
	; 31 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17345 || n = 17355)
	; 32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17346)
	; 32 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17347 || n = 17350)
	; 32 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17348)
	; 31 bytes
	ldy #209
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17349)
	; 31 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 17351)
	; 32 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17352)
	; 32 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 17353)
	; 31 bytes
	ldy #241
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17354)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17356)
	; 32 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17357 || n = 17360)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17358)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 17359)
	; 30 bytes
	ldy #248 ;hides 'sed'
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17361)
	; 31 bytes
	ldy #248
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17362)
	; 32 bytes
	delay_n 111
	delay_n (n-111)
.endif
.else
; 17363 to 17382
.if(n = 17363 || n = 17377 || n = 17379 || n = 17380)
	; 29..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17364)
	; 32 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17365)
	; 32 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 17366)
	; 32 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17367 || n = 17370)
	; 32 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 17368)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17369 || n = 17374)
	; 31 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 17371)
	; 32 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 17372)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 17373)
	; 32 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17375)
	; 28 bytes
	ldy #238
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 17376)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 17378)
	; 30 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17381)
	; 30 bytes
	ldy #220
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 17382)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.endif
.endif
.else
; 17383 to 17421
.if(n = 17383 || n = 17388 || n = 17391 || (n > 17392 && n < 17396) || n = 17398 || n = 17400 || n = 17402 || n = 17405 || n = 17407 || n = 17411 || n = 17413 || n = 17414 || n = 17418)
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17384 || n = 17392 || n = 17404 || n = 17412 || n = 17419)
	; 29..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17385 || n = 17406)
	; 31..32 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17386)
	; 32 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 17387 || n = 17390)
	; 32 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 17389)
	; 27 bytes
	ldy #252
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17396)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17397)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17399)
	; 28 bytes
	ldy #24 ;hides 'clc'
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	iny
	Jne *-25
.elseif(n = 17401)
	; 29 bytes
	ldy #232
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17403)
	; 29 bytes
	ldy #226
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17408)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17409)
	; 30 bytes
	ldy #0
	delay_n 6
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-26
.elseif(n = 17410)
	; 32 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 17415)
	; 32 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17416)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17417)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 17420)
	; 32 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17421)
	; 32 bytes
	delay_n 101
	delay_n (n-101)
.endif
.endif
.else
.if((n >= 17422) && (n < 17461))
.if((n >= 17422) && (n < 17441))
; 17422 to 17440
.if(n = 17422)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17423)
	; 32 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 17424 || n = 17438)
	; 31..32 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 17425)
	; 31 bytes
	ldy #242
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17426)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17427 || n = 17429 || n = 17433)
	; 32..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17428)
	; 32 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 17430)
	; 32 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 17431)
	; 31 bytes
	ldy #249
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17432)
	; 32 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17434)
	; 33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17435)
	; 32 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17436)
	; 32 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 17437)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17439)
	; 33 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 17440)
	; 32 bytes
	delay_n 51
	delay_n (n-51)
.endif
.else
; 17441 to 17460
.if(n = 17441)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 17442)
	; 32 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17443 || n = 17449 || n = 17450 || (n > 17451 && n < 17455) || n = 17460)
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17444)
	; 32 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17445)
	; 31 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 17446)
	; 32 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 17447)
	; 32 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 17448)
	; 28 bytes
	ldy #239
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 17451)
	; 30 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17455)
	; 32 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 17456)
	; 32 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 17457)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 17458)
	; 27 bytes
	ldy #253
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17459)
	; 32 bytes
	delay_n 11
	delay_n (n-11)
.endif
.endif
.else
; 17461 to 17500
.if(n = 17461 || n = 17470 || n = 17479 || n = 17483)
	; 29..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 17461 && n < 17465) || n = 17469 || (n > 17470 && n < 17475) || n = 17478 || n = 17482 || n = 17484 || n = 17499)
	; 28..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17465)
	; 31 bytes
	ldy #236
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17466 || n = 17475)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17467)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17468 || n = 17477)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17476)
	; 29 bytes
	ldy #233
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17480)
	; 29 bytes
	ldy #227
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17481)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 17485 || n = 17492)
	; 32 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17486)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 17487)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17488)
	; 32 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 17489 || n = 17498)
	; 32 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17490)
	; 32 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 17491)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17493)
	; 32 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17494)
	; 32 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17495)
	; 30 bytes
	ldy #216 ;hides 'cld'
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17496)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 17497)
	; 31 bytes
	ldy #243
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17500)
	; 32 bytes
	delay_n 111
	delay_n (n-111)
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 17501) && (n < 18751))
.if((n >= 17501) && (n < 18126))
.if((n >= 17501) && (n < 17813))
.if((n >= 17501) && (n < 17657))
.if((n >= 17501) && (n < 17579))
.if((n >= 17501) && (n < 17540))
; 17501 to 17539
.if(n = 17501)
	; 31 bytes
	ldy #250
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17502)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17503 || n = 17523 || n = 17525 || n = 17526 || n = 17529 || (n > 17530 && n < 17535) || n = 17536)
	; 28..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17504 || n = 17513)
	; 32 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17505 || n = 17512)
	; 32 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 17506)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17507 || n = 17516)
	; 31 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 17508)
	; 32 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 17509 || n = 17518)
	; 32 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 17510 || n = 17517)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 17511)
	; 32 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17514)
	; 31 bytes
	ldy #211
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17515 || n = 17522)
	; 32 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 17519 || n = 17528)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 17520)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17521)
	; 28 bytes
	ldy #240
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 17524 || n = 17530)
	; 29..30 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17527)
	; 27 bytes
	ldy #254
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17535)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17537)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17538)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17539)
	; 30 bytes
	ldy #222
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.endif
.else
; 17540 to 17578
.if(n = 17540 || (n > 17541 && n < 17546) || n = 17553 || n = 17555 || n = 17556 || n = 17559 || n = 17561 || n = 17566 || n = 17573)
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17541 || n = 17552 || n = 17554 || n = 17560)
	; 29..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17546)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17547)
	; 32 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17548)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17549)
	; 28 bytes
	ldy #234 ;hides 'nop'
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17550)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17551)
	; 29 bytes
	ldy #234
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17557)
	; 29 bytes
	ldy #228
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17558)
	; 32 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17562)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 17563)
	; 32 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17564)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17565 || n = 17572)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 17567)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17568)
	; 32 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 17569)
	; 31 bytes
	ldy #244
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17570 || n = 17577)
	; 32 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17571)
	; 31 bytes
	ldy #251
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17574)
	; 32 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 17575)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17576)
	; 31 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 17578)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.endif
.endif
.else
.if((n >= 17579) && (n < 17618))
; 17579 to 17617
.if(n = 17579)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 17580)
	; 32 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17581 || n = 17597 || n = 17599 || n = 17610 || n = 17612)
	; 29..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17582)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17583)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 17584)
	; 32 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 17585 || n = 17592)
	; 32 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 17586 || n = 17590 || n = 17595 || n = 17598 || (n > 17599 && n < 17604) || n = 17605 || n = 17611 || (n > 17612 && n < 17617))
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17587)
	; 31 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 17588)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 17589)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17591)
	; 33 bytes
	delay_n 64
	delay_n (n-64)
.elseif(n = 17593)
	; 32 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 17594)
	; 28 bytes
	ldy #241
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 17596)
	; 27 bytes
	ldy #255
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17604 || n = 17617)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17606)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17607)
	; 27 bytes
	ldy #248 ;hides 'sed'
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 17608)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17609)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.endif
.else
.if((n >= 17618) && (n < 17637))
; 17618 to 17636
.if(n = 17618)
	; 30 bytes
	ldy #223
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 17619)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17620 || n = 17628 || n = 17630 || n = 17636)
	; 30..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17621)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17622 || n = 17635)
	; 32 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17623)
	; 32 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17624)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 17625)
	; 32 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17626)
	; 29 bytes
	ldy #235
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17627)
	; 32 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17629)
	; 31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17631)
	; 31 bytes
	ldy #24 ;hides 'clc'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	iny
	Jne *-28
.elseif(n = 17632)
	; 32 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17633)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 17634)
	; 29 bytes
	ldy #229
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.endif
.else
; 17637 to 17656
.if(n = 17637)
	; 31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17638 || n = 17643)
	; 31..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17639)
	; 32 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17640)
	; 32 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17641)
	; 31 bytes
	ldy #252
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17642 || n = 17655)
	; 32 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17644)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17645 || n = 17656)
	; 31 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 17646)
	; 33 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 17647)
	; 32 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 17648)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 17649)
	; 32 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17650)
	; 32 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 17651)
	; 32 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 17652)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 17653)
	; 32 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 17654)
	; 32 bytes
	delay_n 196
	delay_n (n-196)
.endif
.endif
.endif
.endif
.else
.if((n >= 17657) && (n < 17735))
.if((n >= 17657) && (n < 17696))
; 17657 to 17695
.if(n = 17657)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 17658)
	; 31 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 17659)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17660)
	; 32 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 17661 || n = 17667 || (n > 17668 && n < 17673) || n = 17679 || n = 17682 || (n > 17683 && n < 17687) || n = 17689)
	; 28..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17662)
	; 32 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 17663 || n = 17674)
	; 32 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 17664)
	; 32 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 17665)
	; 27 bytes
	ldy #0
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17666)
	; 33 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17668 || n = 17683)
	; 29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17673 || n = 17688)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17675 || n = 17690)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17676 || n = 17691)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 17677 || n = 17692)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17678)
	; 32 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17680)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17681)
	; 32 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17687)
	; 31 bytes
	ldy #239
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17693)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 17694)
	; 32 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 17695)
	; 32 bytes
	delay_n 86
	delay_n (n-86)
.endif
.else
.if((n >= 17696) && (n < 17715))
; 17696 to 17714
.if(n = 17696)
	; 32 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17697)
	; 30 bytes
	ldy #224
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 17698)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17699 || n = 17703 || n = 17705 || n = 17713)
	; 30..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17700)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17701)
	; 29 bytes
	ldy #236
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17702)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17704 || n = 17714)
	; 31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17706)
	; 32 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 17707)
	; 32 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 17708)
	; 32 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17709)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 17710)
	; 32 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 17711)
	; 29 bytes
	ldy #230
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17712)
	; 32 bytes
	delay_n 116
	delay_n (n-116)
.endif
.else
; 17715 to 17734
.if(n = 17715)
	; 31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17716)
	; 32 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 17717 || n = 17728 || n = 17730)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 17718)
	; 32 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17719)
	; 32 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17720)
	; 32 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 17721)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 17722 || n = 17733)
	; 32 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 17723 || n = 17734)
	; 32 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 17724)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17725)
	; 32 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 17726)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 17727)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17729)
	; 31 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 17731)
	; 32 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 17732)
	; 32 bytes
	delay_n 136
	delay_n (n-136)
.endif
.endif
.endif
.else
.if((n >= 17735) && (n < 17774))
; 17735 to 17773
.if(n = 17735)
	; 32 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 17736)
	; 32 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 17737)
	; 32 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17738)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17739 || n = 17742 || (n > 17743 && n < 17747) || n = 17753 || (n > 17754 && n < 17759) || n = 17764 || n = 17769)
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17740)
	; 28 bytes
	ldy #243
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 17741)
	; 32 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 17743 || n = 17754)
	; 29..30 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17747 || n = 17760)
	; 32 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17748 || n = 17759)
	; 31..32 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17749)
	; 32 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 17750)
	; 32 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17751)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17752 || n = 17765)
	; 32 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 17761)
	; 31 bytes
	ldy #240
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17762)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 17763)
	; 31 bytes
	ldy #214
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17766)
	; 32 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 17767)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17768 || n = 17770)
	; 32 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 17771)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17772)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 17773)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.endif
.else
; 17774 to 17812
.if(n = 17774)
	; 32 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 17775)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 17776)
	; 29 bytes
	ldy #237
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17777)
	; 32 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17778 || n = 17780 || n = 17787 || n = 17790 || (n > 17791 && n < 17795) || n = 17804 || n = 17808 || n = 17809)
	; 30..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17779 || n = 17784 || n = 17791 || n = 17810)
	; 31..34 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17781)
	; 31 bytes
	ldy #254
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17782 || n = 17795)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17783)
	; 31 bytes
	ldy #234 ;hides 'nop'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 17785)
	; 31 bytes
	ldy #247
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17786)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 17788)
	; 29 bytes
	ldy #231
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17789 || n = 17800)
	; 31..32 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 17796)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17797)
	; 32 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17798)
	; 32 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17799)
	; 33 bytes
	delay_n 134
	delay_n (n-134)
.elseif(n = 17801)
	; 32 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 17802)
	; 32 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 17803 || n = 17805)
	; 32 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 17806)
	; 32 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17807)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 17811)
	; 32 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 17812)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.endif
.endif
.endif
.endif
.else
.if((n >= 17813) && (n < 17969))
.if((n >= 17813) && (n < 17891))
.if((n >= 17813) && (n < 17852))
; 17813 to 17851
.if(n = 17813)
	; 28 bytes
	ldy #244
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 17814 || n = 17815 || (n > 17816 && n < 17821) || n = 17823 || n = 17824 || (n > 17825 && n < 17830) || n = 17837 || n = 17840 || n = 17845 || n = 17850)
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17816 || n = 17825 || n = 17849)
	; 29..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17821)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17822)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17830)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17831)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 17832)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17833)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 17834)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17835)
	; 31 bytes
	ldy #241
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17836)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17838)
	; 32 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17839)
	; 32 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 17841)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 17842)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17843)
	; 32 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 17844)
	; 33 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17846)
	; 31 bytes
	ldy #215
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17847)
	; 32 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17848)
	; 32 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17851)
	; 29 bytes
	ldy #238
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.endif
.else
; 17852 to 17890
.if(n = 17852)
	; 32 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 17853 || n = 17859 || n = 17867 || n = 17869 || n = 17870 || n = 17874 || n = 17880 || n = 17884 || n = 17885 || n = 17888 || n = 17890)
	; 29..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17854 || n = 17860 || n = 17866 || n = 17868 || n = 17889)
	; 30..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17855)
	; 30 bytes
	ldy #226
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 17856)
	; 32 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17857)
	; 31 bytes
	ldy #248
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17858)
	; 32 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17861)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 17862)
	; 32 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 17863)
	; 29 bytes
	ldy #24 ;hides 'clc'
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	iny
	Jne *-26
.elseif(n = 17864 || n = 17873)
	; 32..33 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 17865)
	; 29 bytes
	ldy #232
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17871)
	; 31 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 17872)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 17875)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17876)
	; 32 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 17877)
	; 32 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 17878)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 17879)
	; 33 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17881)
	; 33 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 17882)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17883)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 17886)
	; 28 bytes
	ldy #245
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 17887)
	; 32 bytes
	delay_n 136
	delay_n (n-136)
.endif
.endif
.else
.if((n >= 17891) && (n < 17930))
; 17891 to 17929
.if(n = 17891 || n = 17892 || n = 17895 || (n > 17896 && n < 17901) || n = 17906 || n = 17910 || n = 17911 || n = 17916 || n = 17920 || n = 17923 || n = 17928)
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17893)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 17894 || n = 17901)
	; 31..32 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17896)
	; 29 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17902)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17903)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17904)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 17905)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17907)
	; 32 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 17908)
	; 32 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 17909)
	; 31 bytes
	ldy #242
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17912)
	; 32 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 17913)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17914)
	; 32 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 17915)
	; 33 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 17917)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 17918)
	; 32 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17919)
	; 32 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17921)
	; 31 bytes
	ldy #0
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17922)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17924)
	; 32 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17925)
	; 32 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 17926)
	; 29 bytes
	ldy #239
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17927)
	; 31 bytes
	ldy #216 ;hides 'cld'
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.elseif(n = 17929)
	; 31 bytes
	ldy #249
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.endif
.else
; 17930 to 17968
.if(n = 17930 || n = 17931 || n = 17936 || n = 17940 || n = 17944 || n = 17946 || n = 17951 || n = 17955 || n = 17956 || n = 17960 || n = 17961 || n = 17963 || n = 17965 || n = 17966 || n = 17968)
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17932)
	; 32 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17933)
	; 32 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 17934)
	; 30 bytes
	ldy #227
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 17935)
	; 32 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 17937)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 17938)
	; 32 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 17939)
	; 32 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17941)
	; 33 bytes
	ldy #230
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 17942)
	; 29 bytes
	ldy #233
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 17943)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 17945 || n = 17962 || n = 17967)
	; 29..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17947)
	; 32 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 17948)
	; 32 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 17949)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 17950)
	; 33 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 17952)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17953)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17954)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 17957)
	; 33 bytes
	delay_n 64
	delay_n (n-64)
.elseif(n = 17958)
	; 32 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 17959)
	; 28 bytes
	ldy #246
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 17964)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.endif
.endif
.endif
.else
.if((n >= 17969) && (n < 18047))
.if((n >= 17969) && (n < 18008))
; 17969 to 18007
.if((n > 17968 && n < 17972) || n = 17977 || n = 17981 || n = 17982 || n = 17985 || n = 17987 || n = 17991 || n = 17992 || n = 17997 || n = 18002 || n = 18003 || (n > 18004 && n < 18008))
	; 30..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17972)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17973)
	; 32 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17974)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17975)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 17976)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17978)
	; 32 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 17979)
	; 32 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 17980)
	; 32 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17983)
	; 31 bytes
	ldy #243
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 17984)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17986 || n = 18004)
	; 31..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 17988)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 17989)
	; 32 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17990)
	; 32 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17993)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17994)
	; 32 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 17995)
	; 32 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17996)
	; 32 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 17998)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 17999)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18000)
	; 32 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18001)
	; 29 bytes
	ldy #240
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.endif
.else
; 18008 to 18046
.if(n = 18008)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 18009)
	; 32 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 18010)
	; 32 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18011 || n = 18045)
	; 31..33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18012)
	; 31 bytes
	ldy #217
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18013)
	; 30 bytes
	ldy #228
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 18014)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 18015 || n = 18021 || n = 18023 || n = 18026 || n = 18027 || n = 18034 || n = 18036 || n = 18037 || (n > 18038 && n < 18043))
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18016 || n = 18020 || n = 18022 || n = 18038)
	; 29..32 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18017)
	; 29 bytes
	ldy #234 ;hides 'nop'
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 18018)
	; 32 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18019)
	; 29 bytes
	ldy #234
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18024)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 18025)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 18028)
	; 33 bytes
	delay_n 64
	delay_n (n-64)
.elseif(n = 18029)
	; 32 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 18030)
	; 32 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 18031)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18032)
	; 28 bytes
	ldy #247
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 18033)
	; 32 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18035)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 18043)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18044)
	; 32 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18046)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.endif
.endif
.else
.if((n >= 18047) && (n < 18086))
; 18047 to 18085
.if(n = 18047)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18048 || n = 18052 || n = 18053 || n = 18058 || n = 18059 || n = 18062 || n = 18063 || n = 18072 || n = 18075 || n = 18077 || n = 18078 || (n > 18079 && n < 18084))
	; 30..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18049)
	; 32 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 18050)
	; 32 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 18051)
	; 32 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18054)
	; 32 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 18055)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18056)
	; 32 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 18057)
	; 31 bytes
	ldy #244
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18060)
	; 32 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18061)
	; 32 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18064)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18065)
	; 32 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18066)
	; 32 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18067)
	; 32 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 18068)
	; 33 bytes
	ldy #203
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 18069)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 18070)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18071)
	; 32 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18073)
	; 31 bytes
	ldy #251
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18074)
	; 32 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 18076)
	; 29 bytes
	ldy #241
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18079)
	; 31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18084)
	; 31 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 18085)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.endif
.else
; 18086 to 18125
.if(n = 18086)
	; 32 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 18087 || n = 18093 || n = 18094 || n = 18097 || n = 18098 || n = 18100 || n = 18102 || n = 18107 || n = 18108 || (n > 18109 && n < 18114) || n = 18119 || n = 18123)
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18088 || n = 18115 || n = 18118)
	; 31..33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18089)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 18090)
	; 32 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 18091)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 18092)
	; 30 bytes
	ldy #229
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 18095)
	; 31 bytes
	ldy #218
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18096)
	; 29 bytes
	ldy #235
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18099 || n = 18109)
	; 29..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18101)
	; 32 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 18103)
	; 27 bytes
	ldy #248 ;hides 'sed'
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 18104)
	; 32 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18105)
	; 28 bytes
	ldy #248
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 18106)
	; 27 bytes
	ldy #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 18114)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18116)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18117)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 18120)
	; 32 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 18121)
	; 32 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 18122)
	; 32 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18124)
	; 32 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 18125)
	; 32 bytes
	delay_n 161
	delay_n (n-161)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 18126) && (n < 18438))
.if((n >= 18126) && (n < 18282))
.if((n >= 18126) && (n < 18204))
.if((n >= 18126) && (n < 18165))
.if((n >= 18126) && (n < 18145))
; 18126 to 18144
.if(n = 18126)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18127)
	; 32 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 18128)
	; 33 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18129 || n = 18132)
	; 32 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18130)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 18131)
	; 31 bytes
	ldy #245
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18133 || n = 18143)
	; 32..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18134 || n = 18137)
	; 32 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18135)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 18136)
	; 32 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18138)
	; 32 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 18139 || n = 18142)
	; 32 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18140)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 18141)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18144)
	; 32 bytes
	delay_n 41
	delay_n (n-41)
.endif
.else
; 18145 to 18164
.if(n = 18145)
	; 31 bytes
	ldy #252
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18146)
	; 32 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 18147 || n = 18153 || n = 18155 || n = 18156)
	; 30..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 18148 || n = 18154)
	; 31..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18149)
	; 32 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18150)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 18151)
	; 29 bytes
	ldy #242
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18152)
	; 31 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 18157)
	; 32 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 18158)
	; 32 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18159 || n = 18162)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 18160)
	; 32 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18161)
	; 32 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 18163)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18164)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.endif
.endif
.else
; 18165 to 18203
.if(n = 18165)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 18166)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 18167)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 18168 || n = 18175 || n = 18179 || (n > 18180 && n < 18185) || n = 18195 || n = 18200)
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18169 || n = 18172)
	; 32 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 18170)
	; 33 bytes
	delay_n 64
	delay_n (n-64)
.elseif(n = 18171)
	; 30 bytes
	ldy #230
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 18173)
	; 29 bytes
	ldy #236
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18174 || n = 18176 || n = 18180)
	; 29..32 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18177)
	; 27 bytes
	ldy #0
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-23
.elseif(n = 18178)
	; 28 bytes
	ldy #249
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 18185)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18186)
	; 32 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18187)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18188)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 18189 || n = 18190)
	; 31..32 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18191)
	; 32 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 18192)
	; 32 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 18193)
	; 32 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18194 || n = 18197)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18196)
	; 32 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 18198)
	; 32 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 18199 || n = 18202)
	; 32 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18201)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 18203)
	; 32 bytes
	delay_n 26
	delay_n (n-26)
.endif
.endif
.else
.if((n >= 18204) && (n < 18243))
.if((n >= 18204) && (n < 18223))
; 18204 to 18222
.if(n = 18204)
	; 32 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18205)
	; 31 bytes
	ldy #246
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18206)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 18207 || n = 18210 || n = 18215 || n = 18219)
	; 32..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18208)
	; 32 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18209)
	; 32 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 18211)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 18212)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18213)
	; 32 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18214)
	; 32 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 18216)
	; 32 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 18217)
	; 31 bytes
	ldy #253
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18218)
	; 32 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 18220)
	; 33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18221)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 18222)
	; 32 bytes
	delay_n 116
	delay_n (n-116)
.endif
.else
; 18223 to 18242
.if(n = 18223)
	; 32 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18224 || n = 18227)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 18225 || n = 18228 || n = 18230 || n = 18231 || n = 18235 || n = 18240)
	; 30..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18226)
	; 29 bytes
	ldy #243
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18229)
	; 31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18232)
	; 32 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 18233)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 18234 || n = 18237)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 18236)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18238)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 18239 || n = 18242)
	; 32 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 18241)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.endif
.endif
.else
.if((n >= 18243) && (n < 18262))
; 18243 to 18261
.if(n = 18243)
	; 32 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 18244 || n = 18247)
	; 32 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18245 || n = 18252 || n = 18253 || (n > 18254 && n < 18258))
	; 29..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18246)
	; 32 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18248)
	; 32 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 18249)
	; 32 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 18250)
	; 29 bytes
	ldy #237
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18251)
	; 28 bytes
	ldy #250
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 18254)
	; 30 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18258)
	; 32 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18259)
	; 32 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 18260)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18261)
	; 31 bytes
	ldy #220
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.endif
.else
; 18262 to 18281
.if(n = 18262)
	; 32 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 18263)
	; 32 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 18264 || n = 18266 || n = 18267)
	; 32..33 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 18265 || n = 18275 || n = 18281)
	; 32..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18268)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18269 || n = 18272)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 18270)
	; 32 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 18271)
	; 33 bytes
	ldy #210
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 18273)
	; 32 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18274 || n = 18277)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 18276)
	; 32 bytes
	ldy #215
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.elseif(n = 18278)
	; 32 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18279)
	; 31 bytes
	ldy #247
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18280)
	; 32 bytes
	delay_n 174
	delay_n (n-174)
.endif
.endif
.endif
.endif
.else
.if((n >= 18282) && (n < 18360))
.if((n >= 18282) && (n < 18321))
; 18282 to 18320
.if(n = 18282)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 18283)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18284 || n = 18286 || n = 18287)
	; 32..33 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 18285 || n = 18290 || n = 18291 || n = 18295 || n = 18303 || n = 18305 || n = 18306 || n = 18309 || n = 18315 || n = 18319 || n = 18320)
	; 30..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18288)
	; 32 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 18289)
	; 31 bytes
	ldy #254
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18292)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 18293)
	; 32 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 18294 || n = 18297)
	; 32 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 18296)
	; 33 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 18298)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 18299 || n = 18302)
	; 32 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18300)
	; 32 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 18301)
	; 29 bytes
	ldy #244
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18304 || n = 18310 || n = 18316)
	; 31..34 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18307)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18308)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 18311)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18312)
	; 33 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 18313 || n = 18314)
	; 32..33 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 18317)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 18318)
	; 32 bytes
	delay_n 141
	delay_n (n-141)
.endif
.else
; 18321 to 18359
.if(n = 18321 || n = 18325 || n = 18326 || n = 18328 || n = 18332 || n = 18340 || n = 18345 || n = 18346 || n = 18350 || n = 18355 || n = 18357)
	; 29..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18322)
	; 33 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 18323)
	; 32 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 18324)
	; 28 bytes
	ldy #251
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 18327)
	; 29 bytes
	ldy #238
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18329)
	; 30 bytes
	ldy #232
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 18330 || n = 18341 || n = 18347 || n = 18356)
	; 31..34 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18331)
	; 31 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 18333)
	; 32 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 18334 || n = 18337)
	; 32..33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18335)
	; 32 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 18336 || n = 18339)
	; 32..33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18338)
	; 32 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 18342)
	; 33 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18343)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 18344)
	; 31 bytes
	ldy #221
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18348 || n = 18349)
	; 32..33 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 18351)
	; 31 bytes
	ldy #248 ;hides 'sed'
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 18352)
	; 33 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18353)
	; 31 bytes
	ldy #248
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18354)
	; 33 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 18358)
	; 32 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 18359)
	; 31 bytes
	ldy #216 ;hides 'cld'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.endif
.endif
.else
.if((n >= 18360) && (n < 18399))
; 18360 to 18398
.if(n = 18360 || n = 18363 || n = 18365 || n = 18366 || n = 18370 || n = 18375 || n = 18378 || (n > 18379 && n < 18384) || n = 18389 || n = 18390 || n = 18392 || n = 18394 || n = 18395)
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18361)
	; 31 bytes
	ldy #255
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18362)
	; 33 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 18364 || n = 18371 || n = 18379 || n = 18391 || n = 18396)
	; 31..34 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18367)
	; 33 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 18368 || n = 18369)
	; 32..33 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 18372)
	; 33 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 18373)
	; 32 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18374)
	; 33 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 18376)
	; 29 bytes
	ldy #245
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18377)
	; 33 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 18384)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18385)
	; 33 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 18386)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18387)
	; 33 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 18388)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18393)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 18397)
	; 28 bytes
	ldy #252
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 18398)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.endif
.else
; 18399 to 18437
.if(n = 18399 || (n > 18400 && n < 18404) || n = 18406 || n = 18409 || n = 18410 || n = 18412 || n = 18429 || n = 18431 || n = 18434 || n = 18435 || n = 18437)
	; 29..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18400 || n = 18407 || n = 18411 || n = 18430 || n = 18436)
	; 30..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18404)
	; 29 bytes
	ldy #239
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18405)
	; 32 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18408)
	; 30 bytes
	ldy #233
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 18413)
	; 33 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18414)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18415)
	; 33 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18416)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18417)
	; 33 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 18418)
	; 33 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 18419)
	; 34 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18420)
	; 33 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18421 || n = 18422)
	; 33..34 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 18423)
	; 33 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18424)
	; 33 bytes
	ldy #207
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 18425)
	; 33 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18426)
	; 34 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 18427)
	; 31 bytes
	ldy #249
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18428)
	; 33 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18432)
	; 33 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 18433)
	; 31 bytes
	ldy #0
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.endif
.endif
.endif
.endif
.else
.if((n >= 18438) && (n < 18594))
.if((n >= 18438) && (n < 18516))
.if((n >= 18438) && (n < 18477))
; 18438 to 18476
.if(n = 18438)
	; 33 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 18439 || n = 18444 || n = 18453 || (n > 18454 && n < 18459) || n = 18464 || n = 18471 || n = 18472 || (n > 18473 && n < 18477))
	; 29..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18440)
	; 33 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 18441 || n = 18442)
	; 33..34 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 18443)
	; 33 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18445)
	; 33 bytes
	ldy #212
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 18446)
	; 32 bytes
	ldy #217
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.elseif(n = 18447)
	; 33 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18448)
	; 33 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 18449 || n = 18454 || n = 18473)
	; 30..34 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18450)
	; 33 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 18451)
	; 29 bytes
	ldy #246
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18452)
	; 33 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18459)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18460)
	; 33 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 18461)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18462)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 18463)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18465)
	; 33 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18466)
	; 33 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18467)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18468)
	; 33 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 18469)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18470)
	; 28 bytes
	ldy #253
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.endif
.else
; 18477 to 18515
.if(n = 18477 || n = 18479 || n = 18483 || n = 18486 || n = 18489 || n = 18491 || n = 18492 || n = 18494 || n = 18503 || n = 18505 || n = 18507 || n = 18512 || n = 18514)
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18478 || n = 18509)
	; 32..35 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18480)
	; 32 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18481)
	; 29 bytes
	ldy #240
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18482 || n = 18493 || n = 18497 || n = 18499)
	; 32..34 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18484 || n = 18488 || n = 18490 || n = 18504 || n = 18513)
	; 31..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18485)
	; 29 bytes
	ldy #234 ;hides 'nop'
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 18487)
	; 30 bytes
	ldy #234
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 18495)
	; 33 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 18496)
	; 33 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18498)
	; 33 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18500)
	; 33 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 18501)
	; 31 bytes
	ldy #250
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18502)
	; 34 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 18506)
	; 33 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18508)
	; 33 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 18510)
	; 31 bytes
	ldy #223
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18511)
	; 33 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 18515)
	; 33 bytes
	delay_n 191
	delay_n (n-191)
.endif
.endif
.else
.if((n >= 18516) && (n < 18555))
; 18516 to 18554
.if(n = 18516)
	; 33 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18517)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 18518)
	; 33 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 18519)
	; 32 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 18520)
	; 33 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18521)
	; 33 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 18522 || n = 18541)
	; 33..34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 18523)
	; 33 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 18524)
	; 34 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18525)
	; 33 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18526)
	; 29 bytes
	ldy #247
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18527)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 18528 || (n > 18529 && n < 18534) || n = 18540 || n = 18544 || n = 18545 || (n > 18546 && n < 18550) || n = 18552)
	; 29..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 18529 || n = 18546)
	; 30..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18534 || n = 18551)
	; 32..33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18535)
	; 34 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 18536 || n = 18553)
	; 32..33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18537 || n = 18554)
	; 32..33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 18538)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18539)
	; 33 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18542)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18543)
	; 28 bytes
	ldy #254
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 18550)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.endif
.else
; 18555 to 18593
.if(n = 18555)
	; 32 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18556)
	; 33 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 18557 || n = 18560 || (n > 18561 && n < 18565) || n = 18567 || n = 18568 || n = 18570 || n = 18572 || n = 18577 || n = 18579 || n = 18580)
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18558)
	; 29 bytes
	ldy #241
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18559)
	; 33 bytes
	ldy #24 ;hides 'clc'
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	iny
	Jne *-30
.elseif(n = 18561 || n = 18569 || n = 18578)
	; 31..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18565)
	; 33 bytes
	ldy #238
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 18566)
	; 30 bytes
	ldy #235
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 18571)
	; 33 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18573)
	; 33 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 18574)
	; 33 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18575)
	; 31 bytes
	ldy #251
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18576)
	; 33 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18581)
	; 33 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 18582)
	; 34 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 18583 || n = 18590)
	; 33..34 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 18584)
	; 33 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 18585)
	; 34 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 18586)
	; 33 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 18587)
	; 34 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 18588)
	; 33 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 18589)
	; 33 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18591)
	; 33 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 18592)
	; 32 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 18593)
	; 31 bytes
	ldy #224
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.endif
.endif
.endif
.else
.if((n >= 18594) && (n < 18672))
.if((n >= 18594) && (n < 18633))
; 18594 to 18632
.if(n = 18594)
	; 33 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 18595 || n = 18597 || n = 18603 || (n > 18604 && n < 18609) || n = 18618 || (n > 18619 && n < 18624) || n = 18625 || n = 18630)
	; 29..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18596 || n = 18602 || n = 18604 || n = 18619)
	; 30..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18598)
	; 33 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18599)
	; 28 bytes
	ldy #248 ;hides 'sed'
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18600)
	; 34 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18601)
	; 29 bytes
	ldy #248
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18609 || n = 18626)
	; 32 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18610 || n = 18612 || n = 18627)
	; 32..33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 18611 || n = 18613 || n = 18628)
	; 32..33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18614)
	; 33 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 18615 || n = 18617 || n = 18632)
	; 33..34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18616)
	; 28 bytes
	ldy #255
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 18624)
	; 32 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18629)
	; 33 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 18631)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.endif
.else
.if((n >= 18633) && (n < 18652))
; 18633 to 18651
.if(n = 18633 || n = 18637 || (n > 18638 && n < 18642) || n = 18647 || n = 18650 || n = 18651)
	; 30..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18634 || n = 18638 || n = 18648)
	; 31..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18635)
	; 29 bytes
	ldy #242
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18636)
	; 33 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 18642)
	; 33 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18643)
	; 33 bytes
	ldy #239
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 18644)
	; 33 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18645)
	; 30 bytes
	ldy #236
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 18646)
	; 33 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 18649)
	; 31 bytes
	ldy #252
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.endif
.else
; 18652 to 18671
.if(n = 18652)
	; 33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18653 || n = 18658 || n = 18663)
	; 33..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18654)
	; 33 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 18655)
	; 33 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 18656)
	; 33 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 18657)
	; 33 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 18659)
	; 33 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 18660)
	; 33 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 18661)
	; 33 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 18662)
	; 33 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18664)
	; 33 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 18665)
	; 32 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 18666)
	; 33 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18667)
	; 33 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 18668)
	; 33 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18669)
	; 33 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 18670)
	; 33 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 18671)
	; 33 bytes
	delay_n 55
	delay_n (n-55)
.endif
.endif
.endif
.else
.if((n >= 18672) && (n < 18711))
; 18672 to 18710
.if(n = 18672)
	; 33 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 18673)
	; 34 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18674)
	; 33 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 18675)
	; 33 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 18676)
	; 29 bytes
	ldy #249
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18677)
	; 33 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 18678 || (n > 18679 && n < 18684) || n = 18691 || (n > 18692 && n < 18697) || n = 18698 || n = 18703 || n = 18708)
	; 29..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18679 || n = 18692)
	; 30..31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18684)
	; 33 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18685)
	; 33 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 18686 || n = 18699)
	; 32..33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18687)
	; 33 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 18688 || n = 18701)
	; 32..33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18689)
	; 28 bytes
	ldy #0
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-24
.elseif(n = 18690 || n = 18707)
	; 33 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18697)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 18700)
	; 32 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 18702)
	; 33 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 18704)
	; 33 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 18705)
	; 33 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18706)
	; 33 bytes
	ldy #215
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 18709)
	; 33 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 18710)
	; 33 bytes
	delay_n 21
	delay_n (n-21)
.endif
.else
.if((n >= 18711) && (n < 18731))
; 18711 to 18730
.if(n = 18711)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 18712)
	; 29 bytes
	ldy #243
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18713)
	; 34 bytes
	delay_n 170
	delay_n (n-170)
.elseif(n = 18714 || (n > 18715 && n < 18719) || n = 18725 || n = 18726 || n = 18728)
	; 30..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 18715 || n = 18727)
	; 31..32 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18719)
	; 33 bytes
	ldy #234 ;hides 'nop'
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-30
.elseif(n = 18720)
	; 33 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18721)
	; 33 bytes
	ldy #240
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 18722)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18723)
	; 31 bytes
	ldy #253
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18724)
	; 30 bytes
	ldy #237
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 18729)
	; 33 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 18730)
	; 33 bytes
	delay_n 41
	delay_n (n-41)
.endif
.else
; 18731 to 18750
.if(n = 18731)
	; 34 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18732)
	; 33 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 18733)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 18734 || n = 18749)
	; 33..34 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 18735)
	; 33 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18736)
	; 34 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18737)
	; 33 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 18738)
	; 32 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 18739)
	; 33 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18740)
	; 33 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 18741)
	; 34 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18742)
	; 33 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 18743)
	; 34 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18744)
	; 33 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18745)
	; 33 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 18746)
	; 34 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 18747)
	; 33 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 18748)
	; 34 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18750)
	; 33 bytes
	delay_n 61
	delay_n (n-61)
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 18751) && (n < 19376))
.if((n >= 18751) && (n < 19063))
.if((n >= 18751) && (n < 18907))
.if((n >= 18751) && (n < 18829))
.if((n >= 18751) && (n < 18790))
; 18751 to 18789
.if(n = 18751)
	; 29 bytes
	ldy #250
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18752 || n = 18771)
	; 33..34 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 18753 || (n > 18754 && n < 18759) || n = 18761 || n = 18764 || n = 18775 || n = 18779 || n = 18784 || n = 18788)
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18754 || n = 18762 || n = 18776)
	; 31..34 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18759)
	; 31 bytes
	ldy #226
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18760)
	; 33 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 18763)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18765)
	; 33 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 18766 || n = 18783)
	; 33 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 18767)
	; 33 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18768)
	; 34 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 18769)
	; 33 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 18770)
	; 33 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18772)
	; 33 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 18773)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 18774)
	; 34 bytes
	delay_n 85
	delay_n (n-85)
.elseif(n = 18777)
	; 33 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 18778)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 18780)
	; 33 bytes
	ldy #211
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 18781)
	; 34 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 18782)
	; 33 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 18785)
	; 33 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18786)
	; 32 bytes
	ldy #221
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.elseif(n = 18787)
	; 33 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 18789)
	; 29 bytes
	ldy #244
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.endif
.else
; 18790 to 18828
.if(n = 18790)
	; 33 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18791 || n = 18794 || n = 18795 || n = 18798 || n = 18799 || n = 18801 || n = 18804 || n = 18805 || (n > 18806 && n < 18810) || n = 18814 || n = 18819 || n = 18824 || n = 18828)
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 18792 || n = 18796 || n = 18800 || n = 18806)
	; 31..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18793)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18797)
	; 31 bytes
	ldy #254
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18802 || n = 18821)
	; 33..34 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 18803)
	; 30 bytes
	ldy #238
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 18810)
	; 33 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 18811)
	; 34 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 18812)
	; 33 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18813)
	; 34 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18815)
	; 33 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 18816)
	; 34 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 18817)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 18818)
	; 34 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18820)
	; 33 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 18822)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 18823)
	; 34 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 18825)
	; 33 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 18826)
	; 29 bytes
	ldy #251
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18827)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.endif
.endif
.else
.if((n >= 18829) && (n < 18868))
; 18829 to 18867
.if(n = 18829 || n = 18841 || n = 18845 || n = 18852)
	; 31..35 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 18829 && n < 18834) || n = 18839 || n = 18844 || n = 18846 || n = 18851 || n = 18853 || n = 18854 || n = 18864)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 18834)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18835 || n = 18858)
	; 33..34 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 18836)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18837)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 18838)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18840)
	; 33 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18842)
	; 31 bytes
	ldy #227
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18843)
	; 34 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 18847)
	; 31 bytes
	ldy #248 ;hides 'sed'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 18848)
	; 34 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 18849)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 18850)
	; 33 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 18855)
	; 33 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 18856)
	; 33 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 18857)
	; 34 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18859)
	; 34 bytes
	delay_n 170
	delay_n (n-170)
.elseif(n = 18860)
	; 33 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 18861)
	; 35 bytes
	ldy #230
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-31
.elseif(n = 18862)
	; 34 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18863)
	; 33 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 18865)
	; 33 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 18866)
	; 29 bytes
	ldy #245
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18867)
	; 34 bytes
	delay_n 41
	delay_n (n-41)
.endif
.else
; 18868 to 18906
.if(n = 18868 || n = 18870 || n = 18872 || n = 18873 || n = 18875 || n = 18876 || n = 18879 || n = 18884 || (n > 18885 && n < 18890) || n = 18896 || n = 18899 || n = 18903 || n = 18905 || n = 18906)
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 18869 || n = 18885 || n = 18904)
	; 31..32 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18871)
	; 31 bytes
	ldy #255
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18874)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18877)
	; 33 bytes
	ldy #242
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 18878 || n = 18894)
	; 33..34 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18880)
	; 33 bytes
	ldy #217
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 18881)
	; 34 bytes
	ldy #236
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-30
.elseif(n = 18882)
	; 30 bytes
	ldy #239
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 18883)
	; 34 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 18890)
	; 34 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18891)
	; 35 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 18892)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 18893)
	; 34 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 18895)
	; 34 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18897)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 18898)
	; 34 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 18900)
	; 34 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 18901)
	; 29 bytes
	ldy #252
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18902)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.endif
.endif
.endif
.else
.if((n >= 18907) && (n < 18985))
.if((n >= 18907) && (n < 18946))
.if((n >= 18907) && (n < 18926))
; 18907 to 18925
.if(n = 18907 || n = 18908 || n = 18914 || n = 18919 || n = 18924)
	; 32..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 18909)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18910)
	; 34 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 18911)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18912)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 18913)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18915)
	; 33 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 18916)
	; 35 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18917)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18918)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 18920)
	; 34 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 18921)
	; 34 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18922)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 18923)
	; 35 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18925)
	; 31 bytes
	ldy #228
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.endif
.else
; 18926 to 18945
.if(n = 18926 || n = 18927 || n = 18929 || n = 18931 || n = 18934 || n = 18939 || n = 18944 || n = 18945)
	; 30..36 bytes
	delay_n (n-2)
	nop
.elseif(n = 18928)
	; 33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18930)
	; 34 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18932)
	; 34 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18933)
	; 35 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 18935)
	; 34 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 18936)
	; 35 bytes
	delay_n 35
	delay_n (n-35)
.elseif(n = 18937)
	; 34 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18938)
	; 35 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 18940)
	; 34 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18941)
	; 35 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 18942)
	; 34 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 18943)
	; 29 bytes
	ldy #246
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.endif
.endif
.else
; 18946 to 18984
.if(n = 18946 || n = 18958 || n = 18964 || n = 18979)
	; 31..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif((n > 18946 && n < 18951) || n = 18957 || n = 18959 || n = 18960 || n = 18963 || n = 18965 || n = 18966 || n = 18968 || n = 18978 || (n > 18979 && n < 18984))
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18951 || n = 18984)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18952)
	; 34 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 18953)
	; 30 bytes
	ldy #234 ;hides 'nop'
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18954)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 18955)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 18956)
	; 32 bytes
	ldy #223
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.elseif(n = 18961)
	; 30 bytes
	ldy #240
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 18962)
	; 34 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 18967)
	; 33 bytes
	ldy #218
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 18969)
	; 34 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18970)
	; 34 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 18971)
	; 34 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18972)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 18973)
	; 34 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18974)
	; 34 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18975)
	; 34 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 18976)
	; 29 bytes
	ldy #253
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 18977)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.endif
.endif
.else
.if((n >= 18985) && (n < 19024))
.if((n >= 18985) && (n < 19004))
; 18985 to 19003
.if(n = 18985)
	; 34 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18986)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18987)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 18988)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18989)
	; 34 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18990 || n = 18996 || n = 19003)
	; 33..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 18991)
	; 35 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 18992)
	; 33 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 18993)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 18994)
	; 34 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 18995)
	; 35 bytes
	delay_n 169
	delay_n (n-169)
.elseif(n = 18997)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 18998)
	; 34 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 18999)
	; 34 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 19000)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 19001)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19002)
	; 34 bytes
	delay_n 26
	delay_n (n-26)
.endif
.else
; 19004 to 19023
.if(n = 19004 || n = 19011 || n = 19023)
	; 31..34 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19005 || n = 19006 || n = 19010 || n = 19012 || n = 19013 || n = 19015 || n = 19022)
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19007)
	; 34 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19008)
	; 31 bytes
	ldy #229
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19009)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19014)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 19016)
	; 35 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 19017)
	; 34 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 19018)
	; 35 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19019)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 19020)
	; 29 bytes
	ldy #247
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 19021)
	; 35 bytes
	delay_n 120
	delay_n (n-120)
.endif
.endif
.else
; 19024 to 19062
.if((n > 19023 && n < 19028) || n = 19035 || n = 19038 || n = 19042 || (n > 19043 && n < 19047) || n = 19053 || (n > 19054 && n < 19059) || n = 19060)
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 19028 || n = 19048 || n = 19059)
	; 33..34 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19029)
	; 34 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 19030 || n = 19050 || n = 19061)
	; 33..34 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19031 || n = 19062)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19032)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19033)
	; 33 bytes
	ldy #244
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19034)
	; 34 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19036)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19037)
	; 34 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 19039 || n = 19043 || n = 19054)
	; 31..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19040)
	; 30 bytes
	ldy #241
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 19041)
	; 32 bytes
	ldy #224
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19047)
	; 33 bytes
	ldy #214
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19049)
	; 34 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19051)
	; 29 bytes
	ldy #254
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 19052)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.endif
.endif
.endif
.endif
.else
.if((n >= 19063) && (n < 19219))
.if((n >= 19063) && (n < 19141))
.if((n >= 19063) && (n < 19102))
.if((n >= 19063) && (n < 19082))
; 19063 to 19081
.if(n = 19063)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19064)
	; 34 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 19065 || n = 19078 || n = 19079 || n = 19081)
	; 33..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19066)
	; 34 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19067)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19068)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 19069)
	; 33 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 19070)
	; 35 bytes
	delay_n 169
	delay_n (n-169)
.elseif(n = 19071)
	; 34 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 19072)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19073)
	; 35 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 19074)
	; 34 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 19075)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 19076)
	; 34 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 19077)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19080)
	; 34 bytes
	delay_n (n-3)
	ldy $A4
.endif
.else
; 19082 to 19101
.if(n = 19082)
	; 34 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19083 || n = 19088 || n = 19093 || n = 19099 || n = 19101)
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19084)
	; 34 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19085)
	; 34 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19086)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19087)
	; 34 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19089)
	; 34 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 19090 || n = 19094 || n = 19098 || n = 19100)
	; 31..36 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19091)
	; 31 bytes
	ldy #230
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19092)
	; 34 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 19095)
	; 29 bytes
	ldy #248 ;hides 'sed'
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 19096)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 19097)
	; 29 bytes
	ldy #248
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.endif
.endif
.else
; 19102 to 19140
.if((n > 19101 && n < 19105) || n = 19113 || n = 19115 || n = 19121 || (n > 19122 && n < 19126) || n = 19128 || (n > 19129 && n < 19134) || n = 19135 || n = 19140)
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19105 || n = 19134)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19106 || n = 19108 || n = 19137)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19107 || n = 19136)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19109 || n = 19138)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19110)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 19111)
	; 33 bytes
	ldy #245
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19112)
	; 34 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 19114)
	; 34 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19116)
	; 34 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19117)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19118)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19119)
	; 30 bytes
	ldy #242
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 19120 || n = 19122 || n = 19129)
	; 31..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19126)
	; 29 bytes
	ldy #255
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 19127)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 19139)
	; 34 bytes
	delay_n 196
	delay_n (n-196)
.endif
.endif
.else
.if((n >= 19141) && (n < 19180))
; 19141 to 19179
.if(n = 19141)
	; 33 bytes
	ldy #220
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19142)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19143 || n = 19165 || n = 19172)
	; 34..35 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19144 || n = 19146 || n = 19175)
	; 33 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 19145)
	; 35 bytes
	delay_n 169
	delay_n (n-169)
.elseif(n = 19147)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19148 || n = 19170)
	; 34..35 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 19149 || n = 19154 || n = 19155 || n = 19157 || n = 19159 || n = 19169 || n = 19176 || n = 19178 || n = 19179)
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19150)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 19151)
	; 34 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 19152)
	; 34 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19153)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19156 || n = 19177)
	; 31..34 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19158)
	; 34 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 19160)
	; 34 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19161)
	; 34 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19162)
	; 34 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19163)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19164)
	; 34 bytes
	delay_n 69
	delay_n (n-69)
.elseif(n = 19166)
	; 34 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 19167)
	; 34 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 19168)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 19171)
	; 34 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19173)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 19174)
	; 29 bytes
	ldy #249
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.endif
.else
; 19180 to 19218
.if(n = 19180 || n = 19181 || n = 19191 || n = 19199 || n = 19200 || n = 19202 || n = 19203 || (n > 19204 && n < 19209) || n = 19215)
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19182 || n = 19209)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19183)
	; 34 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 19184)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19185 || n = 19212)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19186 || n = 19213)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19187)
	; 34 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 19188)
	; 34 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19189)
	; 33 bytes
	ldy #246
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19190 || n = 19217)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19192)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19193)
	; 34 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19194)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 19195)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19196)
	; 34 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 19197)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 19198)
	; 30 bytes
	ldy #243
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 19201)
	; 29 bytes
	ldy #0
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 19204 || n = 19214)
	; 31..34 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19210)
	; 34 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19211)
	; 32 bytes
	ldy #226
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19216)
	; 34 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 19218)
	; 34 bytes
	delay_n 121
	delay_n (n-121)
.endif
.endif
.endif
.else
.if((n >= 19219) && (n < 19297))
.if((n >= 19219) && (n < 19258))
; 19219 to 19257
.if(n = 19219 || n = 19224 || n = 19230 || n = 19231 || n = 19233 || n = 19234 || n = 19239 || n = 19244 || n = 19249 || n = 19253 || n = 19255 || n = 19256)
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19220 || n = 19247)
	; 34 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19221)
	; 34 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 19222)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19223 || n = 19250)
	; 33 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 19225)
	; 33 bytes
	ldy #216
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19226)
	; 34 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 19227)
	; 34 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19228)
	; 33 bytes
	ldy #221
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19229)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19232 || n = 19254)
	; 31..34 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19235)
	; 34 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 19236 || n = 19238)
	; 34 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19237)
	; 34 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19240)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19241 || n = 19243)
	; 34 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 19242)
	; 34 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 19245)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 19246 || n = 19248)
	; 34 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19251)
	; 29 bytes
	ldy #250
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 19252)
	; 34 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 19257)
	; 31 bytes
	ldy #232
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.endif
.else
; 19258 to 19296
.if(n = 19258 || n = 19259 || n = 19269 || n = 19274 || n = 19281 || n = 19283 || n = 19284 || n = 19294)
	; 32..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19260 || n = 19280 || n = 19282)
	; 32..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19261)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19262)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19263 || n = 19289)
	; 33..34 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19264)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 19265 || n = 19292)
	; 34 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19266 || n = 19268)
	; 34 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19267)
	; 33 bytes
	ldy #247
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19270)
	; 34 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19271)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 19272)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 19273)
	; 34 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 19275)
	; 34 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 19276 || n = 19278)
	; 34 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 19277)
	; 30 bytes
	ldy #244
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 19279)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19285)
	; 34 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 19286 || n = 19288)
	; 34 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19287)
	; 34 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 19290)
	; 34 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 19291 || n = 19293)
	; 34 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 19295)
	; 34 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 19296)
	; 32 bytes
	ldy #227
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.endif
.endif
.else
.if((n >= 19297) && (n < 19336))
; 19297 to 19335
.if(n = 19297)
	; 34 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19298 || n = 19304 || n = 19307 || n = 19309 || n = 19311 || n = 19316 || n = 19319 || n = 19324 || n = 19329 || n = 19330 || (n > 19331 && n < 19336))
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19299 || n = 19308 || n = 19318 || n = 19331)
	; 31..35 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19300 || n = 19326)
	; 33..34 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 19301)
	; 35 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 19302)
	; 34 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 19303)
	; 35 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19305)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19306)
	; 34 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19310)
	; 34 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 19312)
	; 34 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 19313)
	; 35 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19314)
	; 33 bytes
	ldy #217
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19315)
	; 33 bytes
	ldy #222
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19317)
	; 34 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 19320)
	; 34 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 19321)
	; 35 bytes
	ldy #230
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-31
.elseif(n = 19322)
	; 34 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 19323)
	; 35 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 19325)
	; 34 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19327)
	; 34 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 19328)
	; 29 bytes
	ldy #251
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.endif
.else
.if((n >= 19336) && (n < 19356))
; 19336 to 19355
.if(n = 19336)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19337)
	; 34 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 19338)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19339)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19340)
	; 31 bytes
	ldy #233
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19341)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 19342 || n = 19344 || n = 19346 || n = 19347)
	; 32..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 19343)
	; 32 bytes
	ldy #248 ;hides 'sed'
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19345)
	; 33 bytes
	ldy #248
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19348)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 19349)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19350)
	; 34 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 19351)
	; 35 bytes
	ldy #(n-1)/86
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-31
.elseif(n = 19352)
	; 34 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19353)
	; 35 bytes
	ldy #236
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-31
.elseif(n = 19354)
	; 34 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19355)
	; 34 bytes
	delay_n 181
	delay_n (n-181)
.endif
.else
; 19356 to 19375
.if(n = 19356)
	; 30 bytes
	ldy #245
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 19357)
	; 34 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 19358 || n = 19361 || n = 19362 || n = 19364 || n = 19365 || n = 19371)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19359 || n = 19363)
	; 32..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19360)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19366)
	; 34 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19367)
	; 34 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 19368)
	; 34 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19369)
	; 34 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 19370)
	; 34 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 19372)
	; 34 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19373)
	; 35 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19374)
	; 34 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19375)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 19376) && (n < 19688))
.if((n >= 19376) && (n < 19532))
.if((n >= 19376) && (n < 19454))
.if((n >= 19376) && (n < 19415))
; 19376 to 19414
.if(n = 19376 || n = 19383 || n = 19385 || n = 19386 || n = 19396 || n = 19401 || n = 19404 || n = 19407 || (n > 19408 && n < 19413))
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19377)
	; 33 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 19378)
	; 35 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 19379)
	; 34 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 19380 || n = 19384 || n = 19395 || n = 19400 || n = 19406 || n = 19408)
	; 31..36 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19381)
	; 32 bytes
	ldy #(n-1)/76
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19382)
	; 34 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 19387)
	; 34 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 19388)
	; 35 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 19389)
	; 34 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 19390)
	; 35 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19391)
	; 35 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 19392)
	; 34 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19393)
	; 35 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 19394)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19397)
	; 34 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 19398)
	; 35 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 19399)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 19402)
	; 33 bytes
	ldy #223
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19403)
	; 33 bytes
	ldy #218
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19405)
	; 29 bytes
	ldy #252
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 19413)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19414)
	; 34 bytes
	delay_n 86
	delay_n (n-86)
.endif
.else
; 19415 to 19453
.if(n = 19415 || n = 19433)
	; 33..35 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19416)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19417 || n = 19453)
	; 33..35 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19418)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 19419)
	; 34 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19420 || n = 19424 || n = 19426 || n = 19438 || n = 19444)
	; 32..35 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19421)
	; 31 bytes
	ldy #234 ;hides 'nop'
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19422 || n = 19450)
	; 34..36 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19423)
	; 31 bytes
	ldy #234
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19425 || n = 19427 || n = 19428 || n = 19430 || n = 19437 || n = 19439 || n = 19440 || n = 19443 || n = 19445 || n = 19446 || n = 19448)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19429)
	; 34 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 19431)
	; 34 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19432)
	; 34 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 19434)
	; 34 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19435)
	; 30 bytes
	ldy #246
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 19436)
	; 34 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19441)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19442)
	; 34 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19447)
	; 34 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 19449)
	; 34 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 19451)
	; 34 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19452)
	; 35 bytes
	delay_n 96
	delay_n (n-96)
.endif
.endif
.else
.if((n >= 19454) && (n < 19493))
; 19454 to 19492
.if(n = 19454)
	; 33 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 19455)
	; 35 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 19456)
	; 34 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 19457)
	; 32 bytes
	ldy #0
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19458 || n = 19459 || (n > 19460 && n < 19464) || n = 19468 || n = 19470 || n = 19473 || n = 19478 || n = 19480 || n = 19483 || n = 19484 || (n > 19485 && n < 19489) || n = 19491)
	; 30..36 bytes
	delay_n (n-2)
	nop
.elseif(n = 19460 || n = 19485)
	; 31..34 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19464)
	; 34 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 19465)
	; 36 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 19466)
	; 32 bytes
	ldy #229
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19467)
	; 35 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 19469)
	; 34 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19471)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19472)
	; 35 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 19474)
	; 34 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 19475)
	; 34 bytes
	ldy #(n-1)/91
	nop
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-30
.elseif(n = 19476)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 19477)
	; 35 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 19479)
	; 34 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19481)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 19482)
	; 29 bytes
	ldy #253
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 19489)
	; 33 bytes
	ldy #224
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19490)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19492)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 19493 to 19531
.if(n = 19493)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19494)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19495)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 19496)
	; 34 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19497 || n = 19509 || n = 19517 || n = 19525)
	; 32..35 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19498)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19499)
	; 34 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19500 || n = 19503 || n = 19505 || n = 19508 || n = 19510 || n = 19512 || n = 19515 || n = 19516 || (n > 19517 && n < 19521) || n = 19524 || n = 19526 || n = 19527 || n = 19529 || n = 19530)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19501)
	; 33 bytes
	ldy #250
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19502)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 19504)
	; 34 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 19506)
	; 31 bytes
	ldy #235
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19507)
	; 35 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19511)
	; 34 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19513)
	; 34 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19514)
	; 30 bytes
	ldy #247
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 19521)
	; 34 bytes
	ldy #244
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-30
.elseif(n = 19522)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19523)
	; 34 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 19528)
	; 34 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19531)
	; 33 bytes
	delay_n 49
	delay_n (n-49)
.endif
.endif
.endif
.else
.if((n >= 19532) && (n < 19610))
.if((n >= 19532) && (n < 19571))
.if((n >= 19532) && (n < 19551))
; 19532 to 19550
.if(n = 19532)
	; 35 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19533)
	; 34 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 19534)
	; 35 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19535)
	; 35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19536)
	; 34 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 19537)
	; 34 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19538)
	; 34 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 19539)
	; 35 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19540)
	; 35 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19541)
	; 34 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 19542)
	; 35 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 19543)
	; 34 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 19544 || n = 19549)
	; 36 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19545)
	; 35 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19546)
	; 34 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19547)
	; 35 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19548)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19550)
	; 35 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 19551 to 19570
.if(n = 19551)
	; 32 bytes
	ldy #230
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19552)
	; 35 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 19553 || n = 19555 || n = 19557 || n = 19561 || (n > 19562 && n < 19567))
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19554 || n = 19562)
	; 31..34 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19556)
	; 34 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19558)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 19559)
	; 29 bytes
	ldy #254
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 19560)
	; 35 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19567)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19568)
	; 34 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 19569)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19570)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.endif
.endif
.else
; 19571 to 19609
.if(n = 19571)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19572)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 19573)
	; 34 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19574 || n = 19582 || n = 19584 || n = 19592 || n = 19594 || n = 19596 || n = 19606)
	; 31..35 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19575)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19576)
	; 33 bytes
	ldy #225
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19577 || n = 19578 || n = 19587 || n = 19595 || (n > 19596 && n < 19600) || n = 19605 || n = 19607 || n = 19609)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19579)
	; 33 bytes
	ldy #251
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19580)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19581)
	; 33 bytes
	ldy #220
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19583)
	; 34 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 19585)
	; 34 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19586)
	; 34 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 19588)
	; 34 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19589)
	; 31 bytes
	ldy #236
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19590)
	; 34 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19591)
	; 29 bytes
	ldy #248 ;hides 'sed'
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 19593)
	; 30 bytes
	ldy #248
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 19600)
	; 34 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 19601)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19602 || n = 19604)
	; 33..34 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19603)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19608)
	; 33 bytes
	delay_n 49
	delay_n (n-49)
.endif
.endif
.else
.if((n >= 19610) && (n < 19649))
.if((n >= 19610) && (n < 19629))
; 19610 to 19628
.if(n = 19610)
	; 34 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 19611)
	; 35 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 19612)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19613)
	; 34 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 19614)
	; 34 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19615)
	; 34 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 19616)
	; 35 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19617 || n = 19619)
	; 34..35 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19618)
	; 34 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 19620)
	; 34 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 19621)
	; 35 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 19622)
	; 34 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19623)
	; 34 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19624)
	; 35 bytes
	ldy #211
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-31
.elseif(n = 19625)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19626)
	; 35 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19627)
	; 34 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19628)
	; 34 bytes
	delay_n 146
	delay_n (n-146)
.endif
.else
; 19629 to 19648
.if(n = 19629)
	; 35 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19630)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 19631)
	; 35 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 19632 || n = 19634)
	; 34..35 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 19633)
	; 34 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19635)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 19636)
	; 29 bytes
	ldy #255
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 19637)
	; 34 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19638 || (n > 19639 && n < 19644))
	; 30..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 19639)
	; 31 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19644)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19645)
	; 34 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 19646)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19647)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19648)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.else
; 19649 to 19687
.if(n = 19649)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 19650)
	; 34 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19651 || n = 19666 || n = 19675 || n = 19687)
	; 32..35 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19652)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19653)
	; 34 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19654 || n = 19659 || n = 19661 || n = 19664 || n = 19665 || n = 19669 || n = 19674 || (n > 19675 && n < 19680) || n = 19686)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19655)
	; 33 bytes
	ldy #216 ;hides 'cld'
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-30
.elseif(n = 19656)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 19657)
	; 33 bytes
	ldy #252
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19658)
	; 34 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 19660)
	; 34 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 19662)
	; 34 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19663)
	; 33 bytes
	ldy #226
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19667)
	; 34 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19668)
	; 34 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 19670)
	; 33 bytes
	ldy #221
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19671)
	; 35 bytes
	delay_n 35
	delay_n (n-35)
.elseif(n = 19672)
	; 30 bytes
	ldy #249
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 19673)
	; 34 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19680)
	; 34 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 19681)
	; 34 bytes
	ldy #246
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-30
.elseif(n = 19682)
	; 34 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19683)
	; 34 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19684)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19685)
	; 33 bytes
	delay_n 49
	delay_n (n-49)
.endif
.endif
.endif
.endif
.else
.if((n >= 19688) && (n < 19844))
.if((n >= 19688) && (n < 19766))
.if((n >= 19688) && (n < 19727))
.if((n >= 19688) && (n < 19707))
; 19688 to 19706
.if(n = 19688 || n = 19689 || n = 19699 || n = 19704)
	; 33..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19690)
	; 34 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 19691)
	; 34 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19692)
	; 34 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 19693)
	; 35 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19694)
	; 35 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19695)
	; 34 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 19696)
	; 35 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19697)
	; 34 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 19698)
	; 35 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19700)
	; 34 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19701)
	; 36 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 19702)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19703)
	; 35 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19705)
	; 34 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 19706)
	; 35 bytes
	delay_n 34
	delay_n (n-34)
.endif
.else
; 19707 to 19726
.if(n = 19707)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 19708)
	; 35 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19709 || n = 19714 || n = 19715 || n = 19717 || n = 19718 || n = 19720)
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19710)
	; 34 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19711)
	; 35 bytes
	delay_n 120
	delay_n (n-120)
.elseif(n = 19712)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 19713)
	; 29 bytes
	ldy #0
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-25
.elseif(n = 19716 || n = 19722)
	; 31..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19719)
	; 31 bytes
	ldy #24 ;hides 'clc'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	iny
	Jne *-28
.elseif(n = 19721)
	; 32 bytes
	ldy #232
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19723)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19724)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19725)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19726)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.endif
.endif
.else
.if((n >= 19727) && (n < 19746))
; 19727 to 19745
.if(n = 19727)
	; 34 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19728 || n = 19738)
	; 35 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19729)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19730)
	; 34 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19731 || n = 19736 || n = 19737)
	; 34..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19732)
	; 34 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19733)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 19734)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19735)
	; 33 bytes
	ldy #253
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19739)
	; 34 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19740)
	; 34 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 19741)
	; 35 bytes
	ldy #235
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-31
.elseif(n = 19742)
	; 34 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19743)
	; 35 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 19744)
	; 34 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19745)
	; 34 bytes
	delay_n 186
	delay_n (n-186)
.endif
.else
; 19746 to 19765
.if(n = 19746 || n = 19752 || n = 19753 || n = 19756 || n = 19757 || n = 19760)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19747)
	; 34 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 19748)
	; 34 bytes
	ldy #(n-1)/91
	nop
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-30
.elseif(n = 19749)
	; 34 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19750)
	; 33 bytes
	ldy #227
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19751)
	; 30 bytes
	ldy #250
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 19754 || n = 19758)
	; 32..33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19755)
	; 31 bytes
	ldy #238
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19759)
	; 33 bytes
	ldy #222
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19761)
	; 34 bytes
	ldy #247
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-30
.elseif(n = 19762)
	; 33 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 19763)
	; 34 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19764)
	; 34 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 19765)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.endif
.endif
.endif
.else
.if((n >= 19766) && (n < 19805))
.if((n >= 19766) && (n < 19785))
; 19766 to 19784
.if(n = 19766 || n = 19767 || (n > 19768 && n < 19772) || n = 19776)
	; 32..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19768)
	; 33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19772)
	; 34 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 19773)
	; 35 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 19774)
	; 34 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 19775)
	; 35 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19777)
	; 34 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19778)
	; 35 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19779)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19780)
	; 36 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19781)
	; 35 bytes
	ldy #(n-1)/86
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-31
.elseif(n = 19782)
	; 34 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 19783)
	; 35 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 19784)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.endif
.else
; 19785 to 19804
.if(n = 19785)
	; 35 bytes
	delay_n 34
	delay_n (n-34)
.elseif((n .mod 5 = 1) && ((n > 19785 && n < 19802)))
	; 35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19787)
	; 34 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19788)
	; 35 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 19789)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 19790)
	; 36 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19792)
	; 34 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 19793)
	; 35 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 19794)
	; 34 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 19795)
	; 36 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 19797)
	; 34 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 19798)
	; 35 bytes
	delay_n 85
	delay_n (n-85)
.elseif(n = 19799)
	; 34 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 19800)
	; 34 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 19802)
	; 34 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 19803)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 19804)
	; 34 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
; 19805 to 19843
.if(n = 19805)
	; 35 bytes
	delay_n 92
	delay_n (n-92)
.elseif(n = 19806)
	; 32 bytes
	ldy #233
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19807)
	; 34 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19808 || n = 19810 || n = 19811 || n = 19815 || n = 19821 || n = 19826 || n = 19831 || n = 19832 || (n > 19833 && n < 19837) || n = 19840 || n = 19842 || n = 19843)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19809)
	; 34 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19812)
	; 34 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 19813)
	; 33 bytes
	ldy #254
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19814)
	; 34 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 19816 || n = 19833 || n = 19841)
	; 32..35 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19817)
	; 34 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 19818)
	; 35 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 19819)
	; 34 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19820)
	; 35 bytes
	delay_n 107
	delay_n (n-107)
.elseif(n = 19822)
	; 34 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 19823)
	; 35 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19824)
	; 34 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 19825)
	; 35 bytes
	ldy #236
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-31
.elseif(n = 19827)
	; 34 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19828)
	; 35 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 19829)
	; 34 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 19830)
	; 30 bytes
	ldy #251
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 19837)
	; 33 bytes
	ldy #228
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19838)
	; 31 bytes
	ldy #239
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19839)
	; 33 bytes
	ldy #248 ;hides 'sed'
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-30
.endif
.endif
.endif
.else
.if((n >= 19844) && (n < 19922))
.if((n >= 19844) && (n < 19883))
.if((n >= 19844) && (n < 19863))
; 19844 to 19862
.if(n = 19844)
	; 34 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 19845)
	; 35 bytes
	ldy #242
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	delay_n 12
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-31
.elseif(n = 19846)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19847)
	; 35 bytes
	delay_n 134
	delay_n (n-134)
.elseif(n = 19848 || (n > 19849 && n < 19854) || n = 19861)
	; 32..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19849)
	; 33 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19854)
	; 34 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19855)
	; 36 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19856)
	; 35 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19857)
	; 35 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19858)
	; 35 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19859)
	; 34 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 19860)
	; 36 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 19862)
	; 35 bytes
	delay_n 111
	delay_n (n-111)
.endif
.else
; 19863 to 19882
.if(n = 19863)
	; 35 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19864)
	; 34 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19865)
	; 36 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 19866)
	; 34 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 19867)
	; 35 bytes
	ldy #(n-1)/86
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-31
.elseif(n = 19868)
	; 35 bytes
	delay_n 155
	delay_n (n-155)
.elseif(n = 19869)
	; 34 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 19870)
	; 36 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 19871 || n = 19873 || n = 19876 || n = 19878 || n = 19881)
	; 35..36 bytes
	delay_n (n-2)
	nop
.elseif(n = 19872)
	; 35 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 19874)
	; 34 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 19875)
	; 35 bytes
	delay_n 162
	delay_n (n-162)
.elseif(n = 19877)
	; 35 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 19879)
	; 34 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 19880)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 19882)
	; 35 bytes
	delay_n 169
	delay_n (n-169)
.endif
.endif
.else
; 19883 to 19921
.if(n = 19883)
	; 35 bytes
	delay_n 170
	delay_n (n-170)
.elseif(n = 19884)
	; 34 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19885)
	; 35 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19886 || n = 19888 || n = 19893 || n = 19895 || n = 19896 || n = 19898 || n = 19901 || n = 19905 || n = 19906 || n = 19908 || n = 19911 || (n > 19912 && n < 19917) || n = 19918)
	; 31..36 bytes
	delay_n (n-2)
	nop
.elseif(n = 19887)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 19889)
	; 31 bytes
	ldy #234 ;hides 'nop'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19890 || n = 19892 || n = 19912)
	; 32..36 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19891)
	; 32 bytes
	ldy #234
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19894)
	; 34 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 19897)
	; 35 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 19899)
	; 34 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 19900 || n = 19920)
	; 34..35 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19902)
	; 35 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19903)
	; 35 bytes
	ldy #214
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-31
.elseif(n = 19904)
	; 34 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19907)
	; 35 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 19909)
	; 30 bytes
	ldy #252
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 19910)
	; 35 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 19917)
	; 34 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19919)
	; 34 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19921)
	; 31 bytes
	ldy #240
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.endif
.endif
.else
.if((n >= 19922) && (n < 19961))
; 19922 to 19960
.if(n = 19922)
	; 35 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19923 || n = 19925 || n = 19926 || n = 19928 || n = 19929 || (n > 19930 && n < 19935) || n = 19939)
	; 32..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19924)
	; 33 bytes
	ldy #229
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19927)
	; 31 bytes
	ldy #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-27
.elseif(n = 19930 || n = 19940)
	; 33..35 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19935)
	; 35 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19936 || n = 19944 || n = 19952)
	; 35..36 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19937)
	; 33 bytes
	ldy #224
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19938)
	; 35 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19941 || n = 19949 || n = 19957)
	; 35..36 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 19942)
	; 35 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19943)
	; 35 bytes
	delay_n 34
	delay_n (n-34)
.elseif(n = 19945)
	; 35 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19946 || n = 19954)
	; 35..36 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 19947)
	; 35 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 19948)
	; 36 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19950)
	; 35 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 19951)
	; 35 bytes
	ldy #24 ;hides 'clc'
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	iny
	Jne *-32
.elseif(n = 19953)
	; 35 bytes
	ldy #(n-1)/86
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-31
.elseif(n = 19955)
	; 35 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19956)
	; 35 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 19958)
	; 34 bytes
	delay_n 49
	delay_n (n-49)
.elseif(n = 19959)
	; 36 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 19960)
	; 35 bytes
	delay_n 51
	delay_n (n-51)
.endif
.else
.if((n >= 19961) && (n < 19981))
; 19961 to 19980
.if(n = 19961)
	; 35 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 19962)
	; 36 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 19963)
	; 36 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19964)
	; 35 bytes
	delay_n 55
	delay_n (n-55)
.elseif(n = 19965)
	; 35 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 19966)
	; 35 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 19967)
	; 36 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 19968)
	; 36 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 19969)
	; 33 bytes
	ldy #0
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-29
.elseif(n = 19970)
	; 35 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 19971 || n = 19973 || n = 19974 || n = 19978 || n = 19980)
	; 33..36 bytes
	delay_n (n-2)
	nop
.elseif(n = 19972 || n = 19979)
	; 34..35 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19975)
	; 35 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19976)
	; 32 bytes
	ldy #235
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dey
	Jne *-28
.elseif(n = 19977)
	; 36 bytes
	ldy #227
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $26
	.endrepeat
	dey
	Jne *-32
.endif
.else
; 19981 to 20000
.if(n = 19981 || n = 19989)
	; 35..36 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19982 || n = 19990 || (n > 19991 && n < 19996) || n = 19997)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19983)
	; 35 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 19984)
	; 36 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 19985)
	; 35 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 19986)
	; 35 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 19987)
	; 36 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19988)
	; 30 bytes
	ldy #253
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dey
	Jne *-26
.elseif(n = 19991)
	; 32 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 19996)
	; 34 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19998)
	; 34 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19999)
	; 34 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 20000)
	; 34 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif

.endif
	rest
.endmacro
