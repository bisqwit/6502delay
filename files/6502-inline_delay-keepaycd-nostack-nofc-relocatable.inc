; delay_n, delays n cycles with small code
; Will clobber X, Z+N, and V
; Will preserve A, Y, S, C, D, and I (interrupt-disable flag)
;
; This version of delay_n does not change any bytes in the $100 page.
;
; Generated code is:
; - 2 bytes or less when n <= 4
; - 3 bytes or less when n <= 6
; - 4 bytes or less when n <= 8
; - 5 bytes or less when n <= 14
; - 6 bytes or less when n <= 16
; - 7 bytes or less when n <= 31
; - 8 bytes or less when n <= 1293
; - 9 bytes or less when n <= 1303
; -11 bytes or less when n <= 2351
; -12 bytes or less when n <= 2857
; -13 bytes or less when n <= 3414
; -14 bytes or less when n <= 4569
; -15 bytes or less when n <= 5034
; -16 bytes or less when n <= 5559
; -17 bytes or less when n <= 6117
; -18 bytes or less when n <= 7549
; -20 bytes or less when n <= 8631
; -21 bytes or less when n <= 8877
; -22 bytes or less when n <= 10553
; -23 bytes or less when n <= 10822
; -24 bytes or less when n <= 11363
; -25 bytes or less when n <= 11726
; -26 bytes or less when n <= 12971
; -27 bytes or less when n <= 13657
; -29 bytes or less when n <= 15328
; -30 bytes or less when n <= 16270
; -31 bytes or less when n <= 16845
; -32 bytes or less when n <= 17727
; -33 bytes or less when n <= 18388
; -34 bytes or less when n <= 18933
; -35 bytes or less when n <= 19854
; -36 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=3.1, max=5
; - When 11 <= n <= 20: min=4, avg=5.6, max=7
; - When 21 <= n <= 30: min=5, avg=6.2, max=7
; - When 31 <= n <= 50: min=5, avg=6.5, max=8
; - When 51 <= n <= 75: min=5, avg=6.4, max=8
; - When 76 <= n <= 100: min=5, avg=6.3, max=8
; - When 101 <= n <= 200: min=4, avg=6.3, max=8
; - When 201 <= n <= 300: min=5, avg=6.4, max=8
; - When 301 <= n <= 500: min=4, avg=6.4, max=8
; - When 501 <= n <= 750: min=5, avg=6.4, max=8
; - When 751 <= n <= 1000: min=5, avg=6.4, max=8
; - When 1001 <= n <= 2000: min=4, avg=7.4, max=11
; - When 2001 <= n <= 3000: min=6, avg=9.4, max=13
; - When 3001 <= n <= 5000: min=8, avg=11.5, max=15
; - When 5001 <= n <= 7500: min=11, avg=15.0, max=18
; - When 7501 <= n <= 10000: min=14, avg=18.6, max=22
; - When 10001 <= n <= 20000: min=17, avg=27.0, max=36
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
	bit $24
.elseif((n > 3 && n < 8) || n = 9 || n = 13 || n = 14 || n = 17 || n = 20 || n = 23 || n = 25 || n = 28 || n = 30 || n = 33 || n = 35 || n = 38 || n = 40 || n = 42 || n = 47 || n = 48 || n = 52 || n = 53 || n = 55 || n = 58 || n = 60 || n = 63 || n = 65 || n = 68 || n = 70 || n = 73 || n = 75 || n = 77)
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
	tax
	pla
	pha
	txa
.elseif(n = 12)
	; 4 bytes
	rol $36,X
	ror $36,X
.elseif(n = 15)
	; 6 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 16 || n = 21 || n = 26 || n = 36 || n = 46 || n = 51 || n = 56 || n = 66)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 18)
	; 6 bytes
	tax
	.repeat 2
	 pla
	 pha
	.endrepeat
	txa
.elseif(n = 19 || n = 24 || n = 32 || n = 34 || n = 39 || n = 44 || n = 49 || n = 54 || n = 59 || n = 67 || n = 69 || n = 74)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 22 || n = 29 || n = 50 || n = 64 || n = 78)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 27)
	; 7 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	dex
	Jpl *-4
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
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	dex
	Jpl *-4
.elseif(n = 72)
	; 7 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	dex
	Jpl *-4
.endif
.else
; 79 to 156
.if(n = 79 || n = 84 || n = 89 || n = 94 || n = 102 || n = 104 || n = 109 || n = 114 || n = 119 || n = 129 || n = 139 || n = 144 || n = 149 || n = 154)
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
	ldx #(n-1)/8
	bit $24
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
.elseif(n = 137)
	; 7 bytes
	ldx #(n-9)/8
	bit $24
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
.if(n = 157 || n = 158 || n = 160 || n = 163 || n = 165 || n = 168 || n = 170 || n = 173 || n = 175 || n = 178 || n = 180 || n = 182 || n = 187 || n = 188 || n = 192 || n = 193 || n = 195 || n = 198 || n = 200 || n = 202 || n = 203 || n = 205 || n = 208 || n = 210 || n = 212 || n = 213 || n = 222 || n = 223 || n = 227 || n = 228 || n = 230 || n = 233 || n = 235 || n = 237 || n = 238 || n = 240 || n = 243 || n = 245 || n = 247 || n = 248 || n = 250 || n = 252 || n = 258 || n = 263 || n = 268 || n = 270 || n = 272 || n = 273 || n = 275 || n = 278 || n = 282 || n = 283 || n = 285 || n = 287 || n = 292 || n = 293 || n = 297 || n = 298 || n = 300 || n = 303 || n = 305 || n = 308 || n = 310)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 159 || n = 164 || n = 177 || n = 179 || n = 184 || n = 189 || n = 194 || n = 199 || n = 207 || n = 209 || n = 214 || n = 219 || n = 229 || n = 234 || n = 242 || n = 244 || n = 249 || n = 254 || n = 259 || n = 264 || n = 269 || n = 277 || n = 279 || n = 284 || n = 289 || n = 294 || n = 299 || n = 312)
	; 7..8 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 257)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 265)
	; 5 bytes
	ldx #202 ;hides 'dex'
	dex
	Jmi *-2
.elseif(n = 280)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 304)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	dex
	Jpl *-3
.endif
.endif
.else
; 313 to 625
.if(n = 313 || n = 315 || n = 317 || n = 318 || n = 320 || n = 322 || n = 327 || n = 328 || n = 332 || n = 333 || n = 335 || n = 338 || n = 340 || n = 342 || n = 343 || n = 345 || n = 348 || n = 350 || n = 353 || n = 355 || n = 357 || n = 362 || n = 363 || n = 367 || n = 368 || n = 370 || n = 373 || n = 375 || n = 378 || n = 380 || n = 383 || n = 385 || n = 387 || n = 388 || n = 390 || n = 392 || n = 402 || n = 403 || n = 405 || n = 408 || n = 410 || n = 412 || n = 413 || n = 415 || n = 418 || n = 420 || n = 422 || n = 423 || n = 425 || n = 427 || n = 432 || n = 433 || n = 437 || n = 438 || n = 440 || n = 443 || n = 445 || n = 447 || n = 448 || n = 450 || n = 453 || n = 455 || n = 458 || n = 460 || n = 462 || n = 468 || n = 472 || n = 473 || n = 475 || n = 478 || n = 480 || n = 482 || n = 483 || n = 488 || n = 490 || n = 492 || n = 493 || n = 495 || n = 502 || n = 507 || n = 508 || n = 510 || n = 513 || n = 515 || n = 517 || n = 518 || n = 520 || n = 523 || n = 525 || n = 527 || n = 528 || n = 530 || n = 535 || n = 538 || n = 542 || n = 543 || n = 545 || n = 548 || n = 552 || n = 553 || n = 555 || n = 558 || n = 560 || n = 563 || n = 565 || n = 567 || n = 570 || n = 572 || n = 573 || n = 578 || n = 580 || n = 583 || n = 585 || n = 587 || n = 588 || n = 590 || n = 593 || n = 597 || n = 598 || n = 600 || n = 602 || n = 605 || n = 607 || n = 608 || n = 612 || n = 613 || n = 615 || n = 618 || n = 620 || n = 623 || n = 625)
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
.elseif(n = 377 || n = 457 || n = 497)
	; 7 bytes
	ldx #(n-9)/8
	bit $24
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
.elseif(n = 417)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	Jmi *-4
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
.elseif(n = 537)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 562)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.endif
.endif
.else
; 626 to 1250
.if((n .mod 5 = 1) && (n = 626 || n = 636 || n = 641 || (n > 650 && n < 1247)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 627 || n = 629 || n = 634 || n = 639 || n = 644 || n = 649 || n = 654 || n = 662 || n = 664 || n = 669 || n = 674 || n = 679 || n = 684 || n = 689 || n = 699 || n = 704 || n = 709 || n = 714 || n = 719 || n = 724 || n = 732 || n = 734 || n = 739 || n = 749 || n = 754 || n = 759 || n = 767 || n = 769 || n = 774 || n = 779 || n = 784 || n = 789 || n = 794 || n = 804 || n = 809 || n = 814 || n = 819 || n = 824 || n = 829 || n = 837 || n = 839 || n = 844 || n = 849 || n = 854 || n = 859 || n = 864 || n = 872 || n = 874 || n = 879 || n = 884 || n = 889 || n = 894 || n = 907 || n = 909 || n = 914 || n = 919 || n = 924 || n = 929 || n = 934 || n = 942 || n = 944 || n = 949 || n = 954 || n = 959 || n = 964 || n = 969 || n = 979 || n = 984 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1012 || n = 1017 || n = 1019 || n = 1024 || n = 1029 || n = 1034 || n = 1039 || n = 1047 || n = 1049 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1082 || n = 1084 || n = 1089 || n = 1094 || n = 1099 || n = 1104 || n = 1109 || n = 1119 || n = 1124 || n = 1129 || n = 1134 || n = 1139 || n = 1144 || n = 1152 || n = 1154 || n = 1159 || n = 1164 || n = 1169 || n = 1174 || n = 1179 || n = 1187 || n = 1189 || n = 1194 || n = 1199 || n = 1204 || n = 1209 || n = 1214 || n = 1222 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1244 || n = 1249)
	; 6..8 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 628 || n = 630 || n = 632 || n = 633 || n = 635 || n = 637 || n = 643 || n = 648 || n = 650 || n = 653 || n = 655 || n = 658 || n = 660 || n = 663 || n = 665 || n = 668 || n = 670 || n = 675 || n = 677 || n = 678 || n = 682 || n = 683 || n = 688 || n = 690 || n = 692 || n = 693 || n = 695 || n = 698 || n = 700 || n = 702 || n = 703 || n = 705 || n = 707 || n = 713 || n = 717 || n = 718 || n = 720 || n = 723 || n = 725 || n = 728 || n = 733 || n = 735 || n = 738 || n = 740 || n = 742 || n = 747 || n = 752 || n = 753 || n = 755 || n = 758 || n = 760 || n = 762 || n = 763 || n = 765 || n = 768 || n = 770 || n = 772 || n = 773 || n = 783 || n = 787 || n = 788 || n = 790 || n = 793 || n = 795 || n = 797 || n = 798 || n = 800 || n = 803 || n = 805 || n = 807 || n = 808 || n = 810 || n = 812 || n = 818 || n = 822 || n = 823 || n = 825 || n = 828 || n = 830 || n = 832 || n = 833 || n = 835 || n = 838 || n = 840 || n = 842 || n = 843 || n = 845 || n = 852 || n = 853 || n = 857 || n = 858 || n = 860 || n = 863 || n = 867 || n = 868 || n = 870 || n = 873 || n = 875 || n = 877 || n = 878 || n = 880 || n = 882 || n = 887 || n = 888 || n = 893 || n = 895 || n = 898 || n = 900 || n = 902 || n = 903 || n = 905 || n = 908 || n = 910 || n = 912 || n = 913 || n = 915 || n = 917 || n = 922 || n = 923 || n = 927 || n = 928 || n = 930 || n = 933 || n = 938 || n = 940 || n = 943 || n = 945 || n = 948 || n = 950 || n = 952 || n = 957 || n = 958 || n = 963 || n = 965 || n = 968 || n = 970 || n = 972 || n = 973 || n = 975 || n = 978 || n = 980 || n = 983 || n = 985 || n = 987 || n = 992 || n = 993 || n = 997 || n = 998 || n = 1000 || n = 1003 || n = 1005 || n = 1007 || n = 1008 || n = 1010 || n = 1013 || n = 1015 || n = 1018 || n = 1020 || n = 1022 || n = 1028 || n = 1032 || n = 1033 || n = 1035 || n = 1038 || n = 1040 || n = 1042 || n = 1043 || n = 1045 || n = 1048 || n = 1050 || n = 1053 || n = 1055 || n = 1062 || n = 1063 || n = 1067 || n = 1068 || n = 1070 || n = 1073 || n = 1075 || n = 1077 || n = 1078 || n = 1080 || n = 1083 || n = 1085 || n = 1087 || n = 1088 || n = 1090 || n = 1092 || n = 1098 || n = 1102 || n = 1103 || n = 1105 || n = 1108 || n = 1110 || n = 1112 || n = 1113 || n = 1115 || n = 1118 || n = 1120 || n = 1122 || n = 1123 || n = 1125 || n = 1127 || n = 1132 || n = 1133 || n = 1137 || n = 1138 || n = 1140 || n = 1143 || n = 1145 || n = 1147 || n = 1148 || n = 1150 || n = 1153 || n = 1155 || n = 1157 || n = 1158 || n = 1160 || n = 1167 || n = 1168 || n = 1172 || n = 1173 || n = 1175 || n = 1178 || n = 1180 || n = 1182 || n = 1183 || n = 1185 || n = 1188 || n = 1190 || n = 1192 || n = 1193 || n = 1195 || n = 1197 || n = 1202 || n = 1203 || n = 1208 || n = 1210 || n = 1213 || n = 1215 || n = 1218 || n = 1220 || n = 1223 || n = 1225 || n = 1227 || n = 1228 || n = 1230 || n = 1232 || n = 1237 || n = 1238 || n = 1242 || n = 1243 || n = 1245 || n = 1248 || n = 1250)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 631 || n = 646)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif((n .mod 7 = 1) && (n = 638 || n = 645 || (n > 672 && n < 695) || (n > 707 && n < 723) || (n > 742 && n < 765) || n = 778 || n = 785 || n = 799 || n = 813 || n = 834 || (n > 847 && n < 863) || n = 883 || n = 904))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 640 || n = 730 || n = 775 || n = 802 || n = 847 || n = 937 || n = 1117 || n = 1207)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 642 || n = 710 || n = 745 || n = 780 || n = 815 || n = 850 || n = 885 || n = 920 || n = 955 || n = 990 || n = 1025 || n = 1060 || n = 1095 || n = 1130 || n = 1165 || n = 1200 || n = 1235)
	; 7..8 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 647)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
.elseif((n .mod 7 = 1) && (n = 652 || n = 659 || n = 729 || n = 792 || n = 820 || n = 827 || n = 869 || n = 890 || n = 897 || (n > 917 && n < 940) || (n > 952 && n < 975) || (n > 987 && n < 1010) || (n > 1022 && n < 1045) || (n > 1057 && n < 1080) || (n > 1092 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 657 || n = 737 || n = 777 || n = 817 || n = 977)
	; 7 bytes
	ldx #(n-9)/8
	bit $24
	dex
	Jpl *-3
.elseif(n = 667 || n = 685 || n = 712 || n = 865 || n = 892 || n = 982 || n = 1027 || n = 1162)
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
.elseif(n = 697 || n = 1057 || n = 1097 || n = 1217)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 727)
	; 5 bytes
	ldx #24 ;hides 'clc'
	inx
	Jpl *-2
.elseif(n = 744)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jpl *-3
.elseif(n = 748)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jmi *-2
.elseif(n = 782 || n = 947)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
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
.elseif(n = 962)
	; 6 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	Jmi *-3
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
.elseif((n .mod 9 = 1) && (n = 1252 || (n > 1287 && n < 1316) || (n > 1332 && n < 1379) || n = 1396 || n = 1405))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1253 || n = 1255 || n = 1258 || n = 1260 || n = 1262 || n = 1263 || n = 1265 || n = 1267 || n = 1272 || n = 1273 || n = 1277 || n = 1278 || n = 1280 || n = 1283 || n = 1285 || n = 1291 || n = 1293 || n = 1294 || n = 1298 || n = 1300 || n = 1302 || n = 1305 || n = 1307 || n = 1308 || n = 1312 || n = 1314 || n = 1316 || n = 1319 || n = 1322 || n = 1323 || n = 1326 || n = 1328 || n = 1339 || n = 1340 || n = 1344 || n = 1346 || n = 1347 || n = 1349 || n = 1350 || n = 1354 || (n > 1355 && n < 1359) || n = 1361 || n = 1363 || n = 1368 || n = 1370 || n = 1371 || n = 1375 || n = 1382 || n = 1384 || n = 1389 || n = 1391 || n = 1392 || n = 1395 || n = 1403 || n = 1406)
	; 6..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 1254 || (n > 1267 && n < 1402)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 8 = 1) && (n = 1257 || (n > 1312 && n < 1338) || n = 1353 || (n > 1376 && n < 1394)))
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1259 || n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1290 || n = 1292 || n = 1299 || n = 1309 || n = 1318 || n = 1320 || n = 1327 || n = 1334 || n = 1341 || n = 1348 || n = 1355 || n = 1362 || n = 1372 || n = 1379 || n = 1383 || n = 1388 || n = 1390 || n = 1397 || n = 1402 || n = 1404)
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
.elseif(n = 1287)
	; 5 bytes
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
.elseif(n = 1295 || n = 1325 || n = 1400)
	; 9 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 1301 || n = 1311 || n = 1381)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 1304)
	; 10 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 1330)
	; 8 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	inx
	Jpl *-5
.elseif(n = 1332 || n = 1343)
	; 8 bytes
	ldx #(n-12)/11
	delay_n 6
	dex
	Jpl *-4
.elseif(n = 1336)
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 1364)
	; 8 bytes
	bit <$2C ;hides 'bit $04A2'
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
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 12
	inx
	Jpl *-6
.elseif(n = 1374)
	; 8 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	inx
	Jpl *-5
.elseif(n = 1376)
	; 6 bytes
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
.elseif(n = 1386)
	; 8 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	inx
	Jpl *-5
.elseif(n = 1399)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
.endif
.else
; 1407 to 1562
.if(n = 1407 || n = 1410 || n = 1412 || n = 1413 || n = 1416 || n = 1417 || n = 1419 || n = 1424 || (n > 1425 && n < 1429) || n = 1431 || n = 1434 || n = 1435 || n = 1437 || n = 1438 || n = 1440 || n = 1445 || n = 1447 || n = 1448 || n = 1452 || n = 1454 || n = 1455 || n = 1458 || n = 1462 || n = 1466 || n = 1469 || n = 1470 || n = 1472 || n = 1473 || n = 1475 || n = 1476 || n = 1479 || n = 1480 || (n > 1481 && n < 1485) || n = 1487 || n = 1490 || n = 1494 || n = 1496 || n = 1501 || n = 1503 || n = 1508 || n = 1510 || n = 1512 || n = 1515 || n = 1517 || n = 1518 || n = 1525 || n = 1526 || n = 1529 || n = 1532 || n = 1533 || n = 1535 || n = 1536 || n = 1538 || n = 1539 || n = 1542 || n = 1543 || n = 1547 || n = 1550 || n = 1552 || n = 1554 || n = 1557 || n = 1559 || n = 1560)
	; 7..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1563)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 8 = 1) && (n = 1409 || n = 1425 || n = 1433 || n = 1449 || n = 1465 || (n > 1480 && n < 1506) || n = 1521 || (n > 1536 && n < 1562)))
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1411 || n = 1418 || n = 1439 || n = 1444 || n = 1446 || n = 1453 || n = 1460 || n = 1467 || n = 1474 || n = 1488 || n = 1498 || n = 1500 || n = 1502 || n = 1509 || n = 1516 || n = 1523 || n = 1530 || n = 1544 || n = 1551 || n = 1556)
	; 8..9 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 1421 || n = 1451 || n = 1491 || n = 1511)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 1430)
	; 9 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1442 || n = 1519)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1456 || n = 1546)
	; 9 bytes
	ldx #(n-16)/15
	delay_n 10
	dex
	Jpl *-5
.elseif(n = 1461 || n = 1493 || n = 1524)
	; 8..9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1463 || n = 1514)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 1507)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 4
	inx
	inx
	Jne *-5
.elseif(n = 1528)
	; 10 bytes
	delay_n (n-8)
	delay_n 8
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
; 1563 to 1718
.if(n = 1563 || n = 1574 || n = 1596 || n = 1651 || n = 1717)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1564 || n = 1566 || n = 1568 || n = 1571 || n = 1573 || n = 1575 || n = 1578 || n = 1580 || n = 1584 || n = 1588 || n = 1589 || n = 1592 || n = 1595 || n = 1599 || n = 1602 || n = 1605 || n = 1606 || n = 1608 || n = 1613 || n = 1615 || n = 1619 || n = 1620 || n = 1622 || n = 1627 || n = 1629 || n = 1634 || n = 1636 || n = 1638 || n = 1641 || (n > 1642 && n < 1646) || n = 1652 || n = 1659 || n = 1662 || n = 1664 || n = 1668 || n = 1669 || n = 1671 || n = 1672 || n = 1676 || (n > 1677 && n < 1681) || n = 1683 || n = 1685 || n = 1686 || n = 1690 || n = 1692 || n = 1694 || n = 1697 || n = 1699 || n = 1700 || n = 1703 || n = 1704 || (n > 1705 && n < 1709) || n = 1714 || n = 1715 || n = 1718)
	; 7..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1565 || n = 1570 || n = 1572 || n = 1579 || n = 1586 || n = 1600 || n = 1607 || n = 1614 || n = 1624 || n = 1626 || n = 1628 || n = 1635 || n = 1640 || n = 1642 || n = 1656 || n = 1658 || n = 1663 || n = 1670 || n = 1677 || n = 1687 || n = 1691 || n = 1698 || n = 1712)
	; 7..9 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 9 = 1) && (n = 1567 || (n > 1584 && n < 1631) || (n > 1647 && n < 1694) || n = 1711))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1717)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 8 = 1) && (n = 1577 || (n > 1592 && n < 1618) || n = 1633 || n = 1649 || n = 1665 || n = 1673 || n = 1689 || n = 1705 || n = 1713))
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1581 || n = 1591 || n = 1631 || n = 1661 || n = 1701)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 1582)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 1587 || n = 1598 || n = 1650)
	; 8..9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1610)
	; 6 bytes
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
.elseif(n = 1616)
	; 9 bytes
	ldx #(n-18)/17
	delay_n 12
	dex
	Jpl *-5
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
.elseif(n = 1647)
	; 9 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1654)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 1655)
	; 6 bytes
	ldx #184 ;hides 'clv'
	nop
	dex
	Jne *-3
.elseif(n = 1682)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1696)
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 1710)
	; 10 bytes
	delay_n (n-8)
	delay_n 8
.endif
.else
; 1719 to 1875
.if(n = 1719 || n = 1726 || n = 1733 || n = 1740 || n = 1759 || n = 1775 || n = 1780 || n = 1782 || n = 1789 || n = 1796 || n = 1802 || n = 1804 || n = 1813 || n = 1822 || n = 1829 || n = 1836 || n = 1840 || n = 1842 || n = 1844 || n = 1852 || n = 1858 || n = 1874)
	; 8..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 9 = 1) && ((n > 1719 && n < 1757) || (n > 1773 && n < 1874)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif((n .mod 8 = 1) && (n = 1721 || n = 1745 || (n > 1760 && n < 1786) || (n > 1808 && n < 1866)))
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 1722 || n = 1724 || n = 1725 || n = 1727 || n = 1731 || n = 1732 || n = 1734 || n = 1736 || n = 1739 || (n > 1740 && n < 1744) || n = 1746 || n = 1748 || n = 1749 || (n > 1752 && n < 1756) || n = 1757 || (n > 1761 && n < 1765) || n = 1766 || n = 1767 || n = 1770 || n = 1778 || n = 1781 || n = 1784 || n = 1787 || n = 1788 || n = 1790 || n = 1795 || n = 1797 || n = 1798 || n = 1800 || n = 1803 || n = 1807 || n = 1812 || n = 1814 || n = 1815 || n = 1821 || n = 1823 || n = 1824 || n = 1830 || n = 1832 || n = 1835 || n = 1843 || n = 1845 || n = 1848 || n = 1850 || n = 1856 || n = 1859 || n = 1862 || n = 1863 || (n > 1865 && n < 1871) || n = 1875)
	; 7..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1787)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 11 = 1) && (n = 1728 || n = 1750 || (n > 1793 && n < 1839) || n = 1860 || n = 1871))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1735 || n = 1820 || n = 1854)
	; 9 bytes
	ldx #(n-18)/17
	delay_n 12
	dex
	Jpl *-5
.elseif(n = 1752)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1760)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	nop
	inx
	Jne *-3
.elseif(n = 1768)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1771 || n = 1791 || n = 1811 || n = 1831 || n = 1851 || n = 1861)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 1773 || n = 1776 || n = 1853)
	; 8..9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1793)
	; 6 bytes
	ldx #0
	nop
	dex
	Jne *-2
.elseif(n = 1799)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
.elseif(n = 1806)
	; 10 bytes
	ldx #(n-20)/19
	nop
	delay_n 12
	dex
	Jpl *-6
.elseif(n = 1808 || n = 1834 || n = 1847)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 1818)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 10
	dex
	Jmi *-6
.elseif(n = 1826)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
.elseif(n = 1839)
	; 7 bytes
	ldx #184 ;hides 'clv'
	bit $24
	dex
	Jne *-4
.elseif(n = 1872)
	; 11 bytes
	delay_n 86
	delay_n (n-86)
.endif
.endif
.endif
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
; 1876 to 2031
.if(n = 1876 || n = 1906 || n = 1966 || n = 1996)
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif((n > 1876 && n < 1881) || n = 1883 || n = 1884 || n = 1886 || n = 1887 || n = 1890 || n = 1896 || n = 1899 || n = 1902 || n = 1907 || n = 1911 || n = 1914 || n = 1917 || n = 1919 || n = 1920 || n = 1923 || n = 1928 || n = 1934 || n = 1935 || (n > 1937 && n < 1941) || n = 1942 || n = 1943 || n = 1947 || n = 1949 || n = 1950 || n = 1952 || n = 1955 || n = 1956 || n = 1958 || n = 1960 || n = 1965 || n = 1967 || n = 1974 || n = 1976 || n = 1978 || n = 1979 || n = 1982 || n = 1983 || n = 1986 || n = 1987 || n = 1989 || n = 1995 || n = 1997 || n = 1998 || n = 2000 || n = 2005 || n = 2010 || n = 2012 || n = 2019 || n = 2022 || n = 2027 || n = 2028 || n = 2030)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 1) && ((n > 1880 && n < 1938) || (n > 1952 && n < 2010) || n = 2025))
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 2027)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1885 || n = 1892 || n = 1894 || n = 1903 || n = 1908 || n = 1912 || n = 1916 || n = 1924 || n = 1930 || n = 1932 || n = 1944 || n = 1957 || n = 1962 || n = 1975 || n = 1980 || n = 1984 || n = 2002 || n = 2004 || n = 2006 || n = 2020)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1888 || n = 1973 || n = 2024)
	; 9 bytes
	ldx #(n-18)/17
	delay_n 12
	dex
	Jpl *-5
.elseif((n .mod 11 = 1) && ((n > 1892 && n < 1927) || (n > 1947 && n < 1971) || (n > 1991 && n < 2015)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 1895 || n = 1933 || n = 1994 || n = 2013 || n = 2018)
	; 9..10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1898)
	; 11 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 1901 || n = 1931 || n = 1941 || n = 1951 || n = 1971 || n = 1991 || n = 2011 || n = 2021 || n = 2031)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 1910)
	; 10 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 1922 || n = 2007)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 1925 || n = 1964 || n = 2016)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 1946)
	; 11 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1968)
	; 7 bytes
	ldx #232 ;hides 'inx'
	nop
	inx
	inx
	Jne *-4
.elseif(n = 1988)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	dex
	Jne *-5
.elseif(n = 2015)
	; 10 bytes
	ldx #(n-20)/19
	nop
	delay_n 12
	dex
	Jpl *-6
.elseif(n = 2023)
	; 7 bytes
	ldx #184 ;hides 'clv'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2029)
	; 7 bytes
	ldx #202 ;hides 'dex'
	clv
	Jvc *+2
	Jne *-4
.endif
.else
; 2032 to 2187
.if(n = 2032)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 12
	dex
	Jmi *-6
.elseif(n = 2033 || n = 2041)
	; 7 bytes
	ldx #(n-1)/8
	bit $24
	dex
	Jne *-3
.elseif(n = 2034 || n = 2072 || n = 2167 || n = 2186)
	; 10 bytes
	ldx #(n-20)/19
	nop
	delay_n 12
	dex
	Jpl *-6
.elseif((n .mod 9 = 1) && ((n > 2034 && n < 2180)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif((n .mod 11 = 1) && ((n > 2035 && n < 2070) || (n > 2090 && n < 2169)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2037 || n = 2039 || n = 2040 || n = 2043 || n = 2045 || n = 2046 || n = 2048 || n = 2051 || n = 2054 || n = 2055 || n = 2057 || n = 2059 || n = 2060 || n = 2063 || n = 2064 || n = 2066 || n = 2067 || n = 2070 || n = 2073 || n = 2076 || n = 2079 || n = 2082 || n = 2084 || n = 2085 || n = 2088 || n = 2095 || n = 2096 || n = 2100 || n = 2103 || n = 2104 || n = 2106 || n = 2109 || n = 2112 || n = 2115 || n = 2118 || n = 2120 || n = 2122 || n = 2123 || n = 2127 || n = 2129 || n = 2130 || n = 2132 || n = 2136 || (n > 2137 && n < 2141) || n = 2145 || n = 2148 || n = 2150 || n = 2153 || n = 2154 || n = 2156 || n = 2158 || n = 2162 || n = 2163 || n = 2165 || n = 2166 || n = 2169 || n = 2172 || n = 2174 || n = 2175 || n = 2178 || n = 2181 || n = 2183 || n = 2184 || n = 2187)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2038 || n = 2050 || n = 2052 || n = 2065 || n = 2074 || n = 2081 || n = 2083 || n = 2090 || n = 2108 || n = 2110 || n = 2114 || n = 2119 || n = 2128 || n = 2137 || n = 2144 || n = 2155 || n = 2164 || n = 2173 || n = 2180 || n = 2182)
	; 8..11 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2042 || n = 2068 || n = 2094 || n = 2133 || n = 2159 || n = 2185)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2049)
	; 7 bytes
	ldx #0
	bit $24
	dex
	Jne *-3
.elseif(n = 2056 || n = 2086 || n = 2176)
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif((n .mod 10 = 1) && (n = 2061 || (n > 2100 && n < 2152) || n = 2171))
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 2075 || n = 2092)
	; 9 bytes
	ldx #(n-18)/17
	delay_n 12
	dex
	Jpl *-5
.elseif(n = 2077 || n = 2149)
	; 9 bytes
	ldx #(n-1)/12
	delay_n 4
	bit $24
	dex
	Jne *-5
.elseif(n = 2078)
	; 6 bytes
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
.elseif(n = 2087)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	inx
	Jne *-3
.elseif(n = 2093)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2097)
	; 10 bytes
	delay_n 10
	delay_n (n-10)
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
.elseif(n = 2117)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2126 || n = 2160 || n = 2177)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2142)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 2147)
	; 8 bytes
	ldx #232 ;hides 'inx'
	bit $24
	inx
	inx
	Jne *-5
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
.elseif(n = 2189)
	; 11 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2190)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2191 || n = 2211 || n = 2221 || n = 2231 || n = 2261)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 2192 || n = 2193 || n = 2195 || n = 2196 || (n > 2201 && n < 2205) || n = 2208 || n = 2213 || n = 2214 || n = 2216 || n = 2217 || n = 2219 || n = 2220 || n = 2225 || n = 2226 || n = 2229 || n = 2230 || n = 2235 || (n > 2236 && n < 2240) || n = 2243 || n = 2244 || n = 2246 || n = 2247 || n = 2252 || n = 2253 || n = 2255 || n = 2262 || n = 2264 || n = 2265)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2194)
	; 9 bytes
	ldx #(n-18)/17
	delay_n 12
	dex
	Jpl *-5
.elseif(n = 2198)
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
.elseif(n = 2200)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 4
	inx
	Jne *-4
.elseif((n .mod 11 = 1) && ((n > 2200 && n < 2257)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2205)
	; 10 bytes
	ldx #(n-20)/19
	nop
	delay_n 12
	dex
	Jpl *-6
.elseif(n = 2207)
	; 8 bytes
	ldx #184 ;hides 'clv'
	nop
	bit $24
	dex
	Jne *-5
.elseif(n = 2209 || n = 2218 || n = 2227 || n = 2248 || n = 2254 || n = 2263)
	; 9..10 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2210)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2222)
	; 10 bytes
	ldx #234 ;hides 'nop'
	delay_n 10
	dex
	dex
	Jne *-7
.elseif(n = 2228)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2232)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-4
.elseif(n = 2236)
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
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
.elseif(n = 2249)
	; 7 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	inx
	Jne *-4
.elseif(n = 2250)
	; 7 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	inx
	Jne *-4
.elseif(n = 2257)
	; 9 bytes
	ldx #(n-1)/12
	delay_n 4
	bit $24
	dex
	Jne *-5
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
.endif
.else
; 2266 to 2343
.if(n = 2266)
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
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
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2270 || n = 2271 || (n > 2272 && n < 2276) || n = 2279 || n = 2280 || (n > 2281 && n < 2285) || n = 2288 || n = 2292 || n = 2297 || n = 2298 || n = 2306 || n = 2307 || n = 2309 || n = 2310 || n = 2312 || n = 2323 || n = 2324 || n = 2327 || n = 2332 || n = 2335 || n = 2337 || n = 2340 || n = 2343)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2272 || n = 2290 || n = 2299 || n = 2308 || n = 2314 || n = 2316 || n = 2318 || n = 2320 || n = 2325 || n = 2334 || n = 2336 || n = 2342)
	; 9..11 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 2281 || n = 2291 || n = 2301 || n = 2321 || n = 2331 || n = 2341)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
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
.elseif((n .mod 11 = 1) && ((n > 2288 && n < 2334)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2293 || n = 2317 || n = 2329)
	; 9 bytes
	ldx #(n-1)/12
	delay_n 4
	bit $24
	dex
	Jne *-5
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
.elseif(n = 2302 || n = 2315 || n = 2328)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
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
.elseif(n = 2305)
	; 7 bytes
	ldx #0
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2313 || n = 2330)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2319)
	; 7 bytes
	ldx #24 ;hides 'clc'
	bit $24
	inx
	Jne *-4
.elseif(n = 2326)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	inx
	inx
	Jne *-5
.elseif(n = 2338)
	; 10 bytes
	ldx #(n-20)/19
	nop
	delay_n 12
	dex
	Jpl *-6
.elseif(n = 2339)
	; 7 bytes
	ldx #234 ;hides 'nop'
	bit $24
	dex
	Jne *-4
.endif
.endif
.else
; 2344 to 2500
.if((n .mod 11 = 1) && ((n > 2343 && n < 2499)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2345 || n = 2346 || n = 2348 || n = 2349 || n = 2352 || (n > 2356 && n < 2361) || n = 2362 || n = 2363 || n = 2368 || n = 2370 || n = 2372 || n = 2373 || n = 2375 || n = 2378 || n = 2379 || n = 2382 || n = 2383 || n = 2385 || n = 2387 || n = 2390 || n = 2392 || n = 2397 || n = 2400 || n = 2403 || n = 2405 || n = 2407 || n = 2408 || n = 2412 || n = 2417 || n = 2418 || n = 2422 || n = 2423 || n = 2426 || n = 2427 || n = 2433 || n = 2434 || n = 2436 || n = 2439 || n = 2448 || n = 2450 || n = 2453 || n = 2455 || n = 2456 || n = 2459 || n = 2460 || n = 2462 || n = 2463 || (n > 2466 && n < 2471) || n = 2477 || n = 2478 || n = 2480 || n = 2482 || n = 2486 || n = 2488 || n = 2489 || n = 2492 || n = 2493 || n = 2495 || n = 2496 || n = 2499)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347 || n = 2364 || n = 2398 || n = 2415 || n = 2449 || n = 2466 || n = 2483 || n = 2500)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2350 || n = 2369 || n = 2374 || n = 2384 || n = 2394 || n = 2396 || n = 2402 || n = 2404 || n = 2424 || n = 2428 || n = 2438 || n = 2440 || n = 2444 || n = 2457 || n = 2464 || n = 2474 || n = 2479 || n = 2494)
	; 10..11 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 10 = 1) && ((n > 2350 && n < 2412) || (n > 2430 && n < 2492)))
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 2353 || n = 2365 || n = 2389 || n = 2413 || n = 2425 || n = 2437 || n = 2473 || n = 2485)
	; 9 bytes
	ldx #(n-1)/12
	delay_n 4
	bit $24
	dex
	Jne *-5
.elseif((n .mod 13 = 1) && ((n > 2353 && n < 2420) || n = 2445 || n = 2458 || n = 2484 || n = 2497))
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2356 || n = 2386 || n = 2416 || n = 2446)
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 2376 || n = 2414 || n = 2490)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 2395 || n = 2452)
	; 10 bytes
	ldx #(n-20)/19
	nop
	delay_n 12
	dex
	Jpl *-6
.elseif(n = 2409)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	bit $24
	inx
	Jne *-5
.elseif(n = 2420)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	nop
	bit $24
	inx
	Jne *-5
.elseif(n = 2429)
	; 11 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 2430)
	; 12 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 2435)
	; 8 bytes
	ldx #202 ;hides 'dex'
	clv
	clv
	Jvc *+2
	Jne *-5
.elseif(n = 2442)
	; 12 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2447 || n = 2475)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2472)
	; 12 bytes
	delay_n 176
	delay_n (n-176)
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
.if(n = 2501 || n = 2511 || n = 2521 || n = 2541)
	; 8 bytes
	ldx #(n-1)/10
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 2502 || n = 2503 || n = 2508 || n = 2513 || n = 2515 || n = 2516 || n = 2518 || n = 2519 || n = 2522 || n = 2524 || n = 2525 || n = 2527 || n = 2530 || n = 2532 || n = 2533 || n = 2535 || n = 2538 || n = 2544 || n = 2546 || n = 2548 || n = 2555 || n = 2558 || n = 2559 || n = 2563 || n = 2565 || (n > 2569 && n < 2573) || n = 2577)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2504 || n = 2512 || n = 2514 || n = 2526 || n = 2537 || n = 2539 || n = 2543 || n = 2552 || n = 2554 || n = 2556 || n = 2560 || n = 2567 || n = 2576 || n = 2578)
	; 9..11 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2505)
	; 9 bytes
	ldx #232 ;hides 'inx'
	nop
	bit $24
	inx
	inx
	Jne *-6
.elseif(n = 2506 || n = 2536 || n = 2566)
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 2507)
	; 10 bytes
	ldx #(n-1)/14
	delay_n 6
	bit $24
	dex
	Jne *-6
.elseif((n .mod 11 = 1) && ((n > 2508 && n < 2576)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2510 || n = 2523 || n = 2549 || n = 2562)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2517 || n = 2534 || n = 2568)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2528 || n = 2547)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 2529)
	; 11 bytes
	ldx #(n-1)/16
	delay_n 8
	bit $24
	dex
	Jne *-7
.elseif(n = 2540)
	; 7 bytes
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 2545 || n = 2557 || n = 2569)
	; 9 bytes
	ldx #(n-1)/12
	delay_n 4
	bit $24
	dex
	Jne *-5
.elseif(n = 2550 || n = 2574)
	; 11..12 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2551)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2561)
	; 8 bytes
	ldx #0
	nop
	bit $24
	dex
	Jne *-4
.elseif(n = 2573)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
.endif
.else
; 2579 to 2656
.if(n = 2579 || n = 2580 || n = 2583 || n = 2587 || n = 2592 || n = 2595 || n = 2598 || n = 2603 || n = 2607 || n = 2609 || n = 2610 || n = 2612 || n = 2613 || n = 2615 || n = 2616 || n = 2618 || n = 2621 || n = 2624 || n = 2625 || n = 2631 || n = 2632 || n = 2634 || n = 2635 || n = 2637 || n = 2642 || n = 2643 || n = 2645 || n = 2646 || n = 2648 || n = 2649 || n = 2654 || n = 2655)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 15 = 1) && ((n > 2580 && n < 2627) || n = 2656))
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 2582)
	; 11 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 12
	inx
	Jpl *-8
.elseif(n = 2584 || n = 2589 || n = 2600 || n = 2620 || n = 2622 || n = 2633 || n = 2644)
	; 10..11 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2585 || n = 2602 || n = 2636 || n = 2653)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif((n .mod 11 = 1) && ((n > 2585 && n < 2653)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif((n .mod 13 = 1) && ((n > 2587 && n < 2628)))
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2590 || n = 2606)
	; 10..11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2591 || n = 2647)
	; 10 bytes
	ldx #(n-1)/14
	delay_n 6
	bit $24
	dex
	Jne *-6
.elseif((n .mod 12 = 1) && ((n > 2592 && n < 2630)))
	; 9 bytes
	ldx #(n-1)/12
	delay_n 4
	bit $24
	dex
	Jne *-5
.elseif(n = 2594)
	; 12 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 2599)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 6
	inx
	Jne *-5
.elseif(n = 2604 || n = 2623)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 2628)
	; 8 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 4
	inx
	Jne *-5
.elseif(n = 2638)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 8
	Jne *-5
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
	delay_n 6
	inx
	Jne *-5
.elseif(n = 2650)
	; 12 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2651)
	; 12 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
; 2657 to 2734
.if(n = 2657 || n = 2658 || n = 2660 || n = 2662 || n = 2665 || n = 2667 || n = 2668 || n = 2672 || n = 2673 || n = 2676 || n = 2678 || (n > 2680 && n < 2684) || n = 2688 || n = 2691 || n = 2693 || n = 2694 || n = 2698 || n = 2700 || n = 2702 || n = 2703 || n = 2706 || n = 2708 || n = 2709 || n = 2711 || n = 2712 || n = 2714 || n = 2715 || n = 2720 || n = 2722 || n = 2723 || n = 2726 || n = 2727 || n = 2733)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2659 || n = 2669 || n = 2690 || n = 2695 || n = 2710 || n = 2719 || n = 2724 || n = 2728 || n = 2732 || n = 2734)
	; 10..11 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2661 || n = 2680 || n = 2699)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif((n .mod 11 = 1) && ((n > 2662 && n < 2730)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2664 || n = 2697)
	; 11 bytes
	delay_n 124
	delay_n (n-124)
.elseif((n .mod 13 = 1) && ((n > 2665 && n < 2706)))
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2670 || n = 2687 || n = 2704 || n = 2721)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif((n .mod 15 = 1) && ((n > 2670 && n < 2732)))
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 2675 || n = 2717)
	; 10 bytes
	ldx #(n-1)/14
	delay_n 6
	bit $24
	dex
	Jne *-6
.elseif(n = 2677 || n = 2689 || n = 2713 || n = 2725)
	; 9 bytes
	ldx #(n-1)/12
	delay_n 4
	bit $24
	dex
	Jne *-5
.elseif(n = 2684)
	; 9 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	inx
	inx
	Jne *-6
.elseif(n = 2730)
	; 12 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if(n = 2735 || n = 2736 || n = 2741 || n = 2742 || n = 2748 || n = 2752 || n = 2753 || n = 2763 || n = 2764 || (n > 2765 && n < 2769))
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2737)
	; 9 bytes
	ldx #(n-1)/12
	delay_n 4
	bit $24
	dex
	Jne *-5
.elseif(n = 2738)
	; 8 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	inx
	Jne *-5
.elseif(n = 2739)
	; 8 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	nop
	inx
	Jne *-5
.elseif((n .mod 11 = 1) && ((n > 2739 && n < 2774)))
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2743 || n = 2747 || n = 2754 || n = 2758 || n = 2765)
	; 10..11 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2744 || n = 2757 || n = 2770)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2745)
	; 10 bytes
	ldx #(n-1)/14
	delay_n 6
	bit $24
	dex
	Jne *-6
.elseif(n = 2746)
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 2749)
	; 8 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	nop
	inx
	Jne *-5
.elseif(n = 2750)
	; 8 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	inx
	Jne *-5
.elseif(n = 2755)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2756)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 2759)
	; 9 bytes
	ldx #184 ;hides 'clv'
	delay_n 8
	dex
	Jne *-6
.elseif(n = 2760)
	; 8 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	inx
	Jne *-5
.elseif(n = 2761)
	; 8 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	nop
	inx
	Jne *-5
.elseif(n = 2769)
	; 11 bytes
	ldx #(n-1)/16
	delay_n 8
	bit $24
	dex
	Jne *-7
.elseif(n = 2771)
	; 8 bytes
	ldx #232 ;hides 'inx'
	bit $24
	dex
	dex
	Jne *-5
.elseif(n = 2772)
	; 8 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	inx
	Jne *-5
.endif
.else
; 2774 to 2812
.if(n = 2774 || n = 2775 || n = 2777 || n = 2778 || n = 2780 || n = 2786 || n = 2788 || n = 2790 || n = 2797 || n = 2800 || n = 2803 || n = 2808 || n = 2810 || n = 2811)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2776 || n = 2791)
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 2779 || n = 2787 || n = 2792 || n = 2798 || n = 2802 || n = 2812)
	; 10..11 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2781)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 2782)
	; 8 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	inx
	Jne *-5
.elseif(n = 2783)
	; 8 bytes
	ldx #24 ;hides 'clc'
	nop
	bit $24
	inx
	Jne *-5
.elseif(n = 2784 || n = 2795 || n = 2806)
	; 8 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2785)
	; 9 bytes
	ldx #(n-1)/12
	delay_n 4
	bit $24
	dex
	Jne *-5
.elseif(n = 2789)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
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
.elseif(n = 2796 || n = 2809)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2799)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 4
	bit $24
	inx
	Jne *-6
.elseif(n = 2801)
	; 10 bytes
	ldx #(n-1)/14
	delay_n 6
	bit $24
	dex
	Jne *-6
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
	; 8 bytes
	ldx #234 ;hides 'nop'
	nop
	bit $24
	dex
	Jne *-5
.endif
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
; 2813 to 2890
.if(n = 2813 || n = 2832 || n = 2870 || n = 2889)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 2814 || n = 2818 || n = 2819 || n = 2824 || n = 2825 || n = 2827 || n = 2828 || n = 2831 || n = 2834 || n = 2837 || n = 2838 || n = 2842 || n = 2844 || n = 2846 || n = 2849 || n = 2850 || n = 2852 || n = 2853 || n = 2855 || n = 2856 || n = 2858 || n = 2862 || n = 2863 || n = 2865 || n = 2867 || n = 2868 || n = 2872 || n = 2875 || n = 2876 || (n > 2877 && n < 2881) || n = 2883 || n = 2886)
	; 9..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2815)
	; 8 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	nop
	dex
	Jne *-5
.elseif(n = 2816)
	; 8 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	inx
	Jne *-5
.elseif(n = 2817)
	; 8 bytes
	ldx #0
	delay_n 6
	dex
	Jne *-4
.elseif(n = 2820 || n = 2826 || n = 2839 || n = 2854 || n = 2864 || n = 2877 || n = 2884 || n = 2888 || n = 2890)
	; 10..12 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 15 = 1) && ((n > 2820 && n < 2882)))
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif((n .mod 13 = 1) && ((n > 2821 && n < 2862) || n = 2887))
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2823 || n = 2840 || n = 2857 || n = 2874)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2829 || n = 2843 || n = 2871 || n = 2885)
	; 10 bytes
	ldx #(n-1)/14
	delay_n 6
	bit $24
	dex
	Jne *-6
.elseif(n = 2830)
	; 12 bytes
	ldx #122
	delay_n 6
	delay_n 12
	dex
	Jpl *-8
.elseif(n = 2833 || n = 2845 || n = 2869)
	; 9 bytes
	ldx #(n-1)/12
	delay_n 4
	bit $24
	dex
	Jne *-5
.elseif(n = 2841)
	; 9 bytes
	ldx #202 ;hides 'dex'
	clv
	clv
	clv
	Jvc *+2
	Jne *-6
.elseif(n = 2847)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	bit $24
	inx
	Jne *-6
.elseif(n = 2859)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	bit $24
	inx
	Jne *-6
.elseif(n = 2860)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 4
	bit $24
	inx
	Jne *-6
.elseif(n = 2873)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	bit $24
	inx
	Jne *-6
.elseif(n = 2882)
	; 13 bytes
	delay_n 76
	delay_n (n-76)
.endif
.else
; 2891 to 2968
.if(n = 2891 || n = 2908 || n = 2925 || n = 2942 || n = 2959)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2892 || n = 2894 || n = 2903 || n = 2914 || n = 2916 || n = 2932 || n = 2945 || n = 2949 || n = 2962 || n = 2968)
	; 11..12 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 12 = 1) && ((n > 2892 && n < 2930) || n = 2953))
	; 9 bytes
	ldx #(n-1)/12
	delay_n 4
	bit $24
	dex
	Jne *-5
.elseif(n = 2895 || n = 2897 || n = 2898 || n = 2902 || n = 2904 || n = 2907 || n = 2909 || n = 2910 || n = 2912 || n = 2915 || n = 2918 || n = 2919 || n = 2923 || n = 2928 || n = 2930 || n = 2931 || n = 2933 || n = 2934 || n = 2937 || n = 2943 || n = 2944 || n = 2947 || n = 2948 || n = 2950 || n = 2954 || n = 2958 || n = 2960 || n = 2961 || n = 2963 || n = 2964 || n = 2967)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 15 = 1) && ((n > 2895 && n < 2957)))
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 2899 || n = 2955)
	; 10 bytes
	ldx #(n-1)/14
	delay_n 6
	bit $24
	dex
	Jne *-6
.elseif(n = 2900 || n = 2913 || n = 2939 || n = 2952 || n = 2965)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2901 || n = 2921)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 2906)
	; 10 bytes
	ldx #202 ;hides 'dex'
	delay_n 10
	dex
	dex
	Jne *-7
.elseif(n = 2920)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 2922)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 2924)
	; 12 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	delay_n 12
	dex
	dex
	Jne *-9
.elseif(n = 2927 || n = 2946)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 2935)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 2936 || n = 2966)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2938)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2940)
	; 12 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 2951)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 2957)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
; 2969 to 3046
.if(n = 2969 || n = 2983 || n = 2997 || n = 3039)
	; 10 bytes
	ldx #(n-1)/14
	delay_n 6
	bit $24
	dex
	Jne *-6
.elseif(n = 2970 || n = 2973 || n = 2975 || n = 2979 || n = 2980 || n = 2982 || n = 2985 || n = 2990 || n = 2995 || n = 2998 || n = 3003 || n = 3006 || n = 3008 || n = 3014 || (n > 3017 && n < 3022) || n = 3026 || n = 3029 || n = 3032 || n = 3033 || n = 3038 || n = 3045)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 15 = 1) && ((n > 2970 && n < 3047)))
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 2972 || n = 2974 || n = 2981 || n = 2992 || n = 2994 || n = 2996 || n = 3005 || n = 3007 || n = 3028 || n = 3034 || n = 3040)
	; 10..12 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 2976 || n = 2993 || n = 3010 || n = 3027 || n = 3044)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 2977 || n = 2989 || n = 3013 || n = 3025 || n = 3037)
	; 9 bytes
	ldx #(n-1)/12
	delay_n 4
	bit $24
	dex
	Jne *-5
.elseif((n .mod 13 = 1) && ((n > 2977 && n < 3044)))
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2984 || n = 3022)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 2987)
	; 9 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	inx
	Jne *-6
.elseif(n = 2988)
	; 9 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	inx
	Jne *-6
.elseif(n = 2999)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 8
	inx
	Jne *-6
.elseif(n = 3000)
	; 9 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	inx
	Jne *-6
.elseif(n = 3002)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
.elseif(n = 3009)
	; 11 bytes
	ldx #(n-1)/16
	delay_n 8
	bit $24
	dex
	Jne *-7
.elseif(n = 3011)
	; 9 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	inx
	Jne *-6
.elseif(n = 3012)
	; 9 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	inx
	Jne *-6
.elseif(n = 3015)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 6
	inx
	Jne *-5
.elseif(n = 3023)
	; 9 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	inx
	Jne *-6
.elseif(n = 3024)
	; 9 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	inx
	Jne *-6
.elseif(n = 3035)
	; 9 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	inx
	Jne *-6
.elseif(n = 3036)
	; 9 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	inx
	Jne *-6
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
.endif
.else
; 3047 to 3125
.if(n = 3047)
	; 9 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	inx
	Jne *-6
.elseif(n = 3048)
	; 9 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	inx
	Jne *-6
.elseif(n = 3049)
	; 9 bytes
	ldx #(n-1)/12
	delay_n 4
	bit $24
	dex
	Jne *-5
.elseif(n = 3050 || n = 3051 || n = 3054 || n = 3055 || n = 3058 || n = 3062 || n = 3063 || n = 3065 || n = 3068 || n = 3070 || n = 3074 || n = 3075 || n = 3077 || n = 3083 || n = 3084 || n = 3086 || n = 3087 || n = 3090 || n = 3093 || n = 3096 || n = 3097 || n = 3099 || n = 3100 || n = 3102 || n = 3107 || n = 3110 || n = 3113 || n = 3114 || n = 3116 || n = 3118 || n = 3119 || n = 3123 || n = 3125)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3052 || n = 3064 || n = 3085 || n = 3111 || n = 3115 || n = 3120 || n = 3124)
	; 11..12 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3053 || n = 3067 || n = 3081 || n = 3109)
	; 10 bytes
	ldx #(n-1)/14
	delay_n 6
	bit $24
	dex
	Jne *-6
.elseif(n = 3056 || n = 3069 || n = 3082 || n = 3108)
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3057 || n = 3089 || n = 3105)
	; 11 bytes
	ldx #(n-1)/16
	delay_n 8
	bit $24
	dex
	Jne *-7
.elseif(n = 3059)
	; 9 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	inx
	Jne *-6
.elseif(n = 3060)
	; 9 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	inx
	Jne *-6
.elseif((n .mod 15 = 1) && ((n > 3060 && n < 3122)))
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3066)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3071)
	; 9 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	dex
	Jne *-6
.elseif(n = 3072)
	; 9 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	inx
	Jne *-6
.elseif(n = 3073)
	; 9 bytes
	ldx #0
	delay_n 4
	bit $24
	dex
	Jne *-5
.elseif(n = 3078 || n = 3095 || n = 3112)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3079)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3080)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 8
	inx
	Jne *-6
.elseif(n = 3088)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3092 || n = 3122)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3094)
	; 9 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 6
	inx
	Jne *-6
.elseif(n = 3098 || n = 3117)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 3101)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3103)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 3104)
	; 13 bytes
	ldx #106
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jpl *-9
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
; 3126 to 3281
.if(n = 3126 || n = 3128 || n = 3131 || n = 3133 || n = 3135 || n = 3138 || n = 3140 || n = 3142 || n = 3143 || (n > 3147 && n < 3151) || n = 3152 || n = 3153 || n = 3156 || n = 3157 || n = 3159 || n = 3162 || n = 3164 || n = 3165 || n = 3167 || n = 3168 || n = 3170 || n = 3171 || (n > 3174 && n < 3179) || n = 3182 || n = 3183 || n = 3185 || n = 3188 || (n > 3189 && n < 3193) || n = 3195 || n = 3198 || n = 3201 || n = 3203 || n = 3204 || n = 3209 || n = 3213 || n = 3215 || n = 3216 || (n > 3217 && n < 3221) || n = 3227 || n = 3228 || n = 3230 || n = 3232 || n = 3239 || n = 3240 || n = 3242 || n = 3243 || n = 3245 || n = 3246 || n = 3252 || n = 3253 || n = 3255 || n = 3257 || n = 3258 || n = 3260 || n = 3266 || n = 3267 || n = 3270 || n = 3273 || n = 3278 || n = 3279 || n = 3281)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3127)
	; 9 bytes
	ldx #184 ;hides 'clv'
	delay_n 10
	dex
	Jne *-6
.elseif(n = 3129 || n = 3146 || n = 3163 || n = 3180 || n = 3197 || n = 3214 || n = 3231 || n = 3248 || n = 3265)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3130 || n = 3172)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3132 || n = 3139 || n = 3154 || n = 3158 || n = 3169 || n = 3184 || n = 3189 || n = 3200 || n = 3202 || n = 3210 || n = 3217 || n = 3224 || n = 3229 || n = 3234 || n = 3244 || n = 3254 || n = 3259 || n = 3268 || n = 3272 || n = 3274 || n = 3280)
	; 11..12 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 13 = 1) && ((n > 3133 && n < 3278)))
	; 9 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-5
.elseif((n .mod 15 = 1) && ((n > 3135 && n < 3272)))
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3137 || n = 3179 || n = 3207 || n = 3221 || n = 3235)
	; 10 bytes
	ldx #(n-1)/14
	delay_n 6
	bit $24
	dex
	Jne *-6
.elseif(n = 3141 || n = 3161 || n = 3261)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3144 || n = 3194)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3145)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 3155 || n = 3174 || n = 3193 || n = 3269)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 3187 || n = 3205 || n = 3223)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 3206 || n = 3222)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3208)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3233)
	; 9 bytes
	ldx #232 ;hides 'inx'
	nop
	bit $24
	dex
	dex
	Jne *-6
.elseif(n = 3236)
	; 9 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3237)
	; 9 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3247)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 4
	bit $24
	inx
	Jne *-6
.elseif(n = 3249)
	; 9 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3250)
	; 9 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3262)
	; 9 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3263)
	; 9 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3275)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	bit $24
	dex
	Jne *-6
.elseif(n = 3276)
	; 9 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 4
	inx
	Jne *-6
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282 || n = 3299)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3283 || n = 3284 || n = 3287 || n = 3292 || n = 3294 || n = 3296 || n = 3304 || n = 3305 || n = 3308 || n = 3309 || n = 3312 || n = 3317 || n = 3318 || n = 3320)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3285)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 4
	bit $24
	inx
	Jne *-7
.elseif(n = 3286 || n = 3301 || n = 3316)
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3288)
	; 9 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3289)
	; 9 bytes
	bit <$2C ;hides 'bit $03A2'
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
.elseif(n = 3291 || n = 3319)
	; 10 bytes
	ldx #(n-1)/14
	delay_n 6
	bit $24
	dex
	Jne *-6
.elseif(n = 3293 || n = 3306 || n = 3310)
	; 11..12 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3295 || n = 3313)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 3297)
	; 11 bytes
	ldx #(n-1)/16
	delay_n 8
	bit $24
	dex
	Jne *-7
.elseif(n = 3298)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3300)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 6
	bit $24
	inx
	Jne *-7
.elseif(n = 3302)
	; 9 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3307)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 3311)
	; 11 bytes
	ldx #184 ;hides 'clv'
	delay_n 8
	bit $24
	dex
	Jne *-8
.elseif(n = 3314)
	; 9 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3315)
	; 9 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	inx
	Jne *-6
.endif
.else
; 3321 to 3359
.if(n = 3321 || n = 3341)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3322 || n = 3330 || n = 3335 || (n > 3336 && n < 3340) || (n > 3341 && n < 3345) || n = 3348 || n = 3351 || n = 3352 || (n > 3353 && n < 3358))
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3323)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 3324)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3325)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3326 || n = 3345)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 3327)
	; 9 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	dex
	Jne *-6
.elseif(n = 3328)
	; 9 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	inx
	Jne *-6
.elseif(n = 3329)
	; 9 bytes
	ldx #0
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3331 || n = 3346)
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3332 || n = 3334 || n = 3336 || n = 3349 || n = 3353)
	; 11 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3333 || n = 3350)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3340)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3347)
	; 10 bytes
	ldx #(n-1)/14
	delay_n 6
	bit $24
	dex
	Jne *-6
.elseif(n = 3358)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3359)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.endif
.endif
.else
; 3360 to 3437
.if(n = 3360 || n = 3416)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif((n .mod 15 = 1) && ((n > 3360 && n < 3437)))
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3362 || n = 3413)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3363 || n = 3365 || n = 3366 || n = 3368 || n = 3369 || (n > 3370 && n < 3375) || n = 3378 || n = 3380 || n = 3385 || n = 3386 || n = 3388 || n = 3390 || n = 3393 || (n > 3394 && n < 3399) || n = 3403 || n = 3405 || n = 3407 || n = 3408 || (n > 3409 && n < 3413) || n = 3419 || n = 3420 || n = 3422 || n = 3423 || (n > 3424 && n < 3428) || n = 3433 || n = 3437)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3364 || n = 3383 || n = 3402)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 3367 || n = 3384 || n = 3401 || n = 3418 || n = 3435)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3370 || n = 3379 || n = 3387 || n = 3392 || n = 3394 || n = 3404 || n = 3409 || n = 3424 || n = 3434)
	; 11..12 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3375 || n = 3389 || n = 3417 || n = 3431)
	; 10 bytes
	ldx #(n-1)/14
	delay_n 6
	bit $24
	dex
	Jne *-6
.elseif(n = 3377)
	; 11 bytes
	ldx #(n-1)/16
	delay_n 8
	bit $24
	dex
	Jne *-7
.elseif(n = 3381)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3382)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3399)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 10
	inx
	Jne *-6
.elseif(n = 3400)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 10
	inx
	inx
	Jne *-7
.elseif(n = 3414 || n = 3429)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3415)
	; 14 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 3428)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 3430)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 3432)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
; 3438 to 3515
.if(n = 3438 || n = 3442 || n = 3444 || n = 3447 || n = 3449 || n = 3453 || n = 3454 || n = 3456 || n = 3458 || n = 3461 || n = 3463 || n = 3465 || n = 3468 || n = 3470 || n = 3471 || n = 3474 || n = 3477 || n = 3480 || n = 3483 || n = 3488 || (n > 3489 && n < 3493) || n = 3498 || n = 3502 || n = 3505 || n = 3507 || n = 3510 || n = 3513)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3439 || n = 3443 || n = 3448 || n = 3455 || n = 3462 || n = 3467 || n = 3472 || n = 3476 || n = 3482 || n = 3484 || n = 3489 || n = 3506 || n = 3512)
	; 11..12 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3440 || n = 3459 || n = 3478 || n = 3497)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 3441)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif((n .mod 14 = 1) && (n = 3445 || (n > 3472 && n < 3516)))
	; 10 bytes
	ldx #(n-1)/14
	delay_n 6
	bit $24
	dex
	Jne *-6
.elseif(n = 3446)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3450)
	; 10 bytes
	ldx #202 ;hides 'dex'
	.repeat 6
	 nop
	.endrepeat
	Jne *-7
.elseif((n .mod 15 = 1) && ((n > 3450 && n < 3512)))
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3452 || n = 3469 || n = 3486 || n = 3503)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3457 || n = 3475 || n = 3493)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 3460 || n = 3494)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3464)
	; 9 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-6
.elseif(n = 3479)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 8
	inx
	Jne *-6
.elseif(n = 3485)
	; 10 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3495)
	; 9 bytes
	ldx #184 ;hides 'clv'
	delay_n 12
	dex
	Jne *-6
.elseif(n = 3499)
	; 10 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3500)
	; 10 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3504)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 8
	inx
	Jne *-7
.elseif(n = 3508)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3509)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 8
	dex
	Jne *-6
.elseif(n = 3514)
	; 10 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	nop
	bit $24
	inx
	Jne *-7
.endif
.else
.if((n >= 3516) && (n < 3555))
; 3516 to 3554
.if(n = 3516 || n = 3535)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 3517 || n = 3518 || n = 3522 || n = 3524 || n = 3525 || n = 3528 || (n > 3529 && n < 3534) || n = 3538 || n = 3539 || n = 3543 || n = 3545 || n = 3546 || n = 3548 || n = 3549 || n = 3552)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3519)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3520 || n = 3537 || n = 3554)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3521)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3523 || n = 3540 || n = 3544)
	; 11 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3526 || n = 3541)
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3527)
	; 10 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3529)
	; 10 bytes
	ldx #(n-1)/14
	delay_n 6
	bit $24
	dex
	Jne *-6
.elseif(n = 3534)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3536)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 8
	inx
	Jne *-7
.elseif(n = 3542)
	; 10 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3547)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 3550)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3551)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 3553)
	; 11 bytes
	ldx #(n-1)/16
	delay_n 8
	bit $24
	dex
	Jne *-7
.endif
.else
; 3555 to 3593
.if(n = 3555)
	; 10 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3556 || n = 3571 || n = 3586)
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3557)
	; 10 bytes
	ldx #(n-1)/14
	delay_n 6
	bit $24
	dex
	Jne *-6
.elseif(n = 3558 || n = 3560 || n = 3562 || n = 3563 || n = 3567 || n = 3572 || n = 3573 || (n > 3574 && n < 3579) || n = 3587 || n = 3590 || n = 3593)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3559 || n = 3574 || n = 3589 || n = 3591)
	; 11 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3561 || n = 3581)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3564)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3565)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 3566)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 3568)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3569)
	; 10 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3570)
	; 10 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3579)
	; 12 bytes
	ldx #232 ;hides 'inx'
	delay_n 8
	bit $24
	inx
	inx
	Jne *-9
.elseif(n = 3580)
	; 14 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3582)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 3583)
	; 10 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	nop
	bit $24
	dex
	Jne *-7
.elseif(n = 3584)
	; 10 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	bit $24
	inx
	Jne *-7
.elseif(n = 3585)
	; 10 bytes
	ldx #0
	delay_n 6
	bit $24
	dex
	Jne *-6
.elseif(n = 3588)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3592)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.endif
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
; 3594 to 3671
.if(n = 3594 || n = 3596 || n = 3603 || n = 3606 || n = 3607 || n = 3609 || n = 3610 || n = 3618 || n = 3620 || n = 3623 || n = 3624 || (n > 3625 && n < 3630) || n = 3632 || n = 3633 || n = 3635 || n = 3636 || n = 3638 || n = 3641 || (n > 3642 && n < 3646) || n = 3648 || n = 3650 || n = 3651 || n = 3654 || n = 3657 || n = 3658 || n = 3660 || n = 3662 || n = 3663 || (n > 3664 && n < 3668) || n = 3670)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3595 || n = 3602 || n = 3604 || n = 3608 || n = 3614 || n = 3619 || n = 3625 || n = 3634 || n = 3640 || n = 3642 || n = 3652 || n = 3659 || n = 3664 || n = 3671)
	; 11..13 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3597)
	; 13 bytes
	ldx #124
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 3598)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3599)
	; 11 bytes
	ldx #56 ;hides 'sec'
	delay_n 8
	bit $24
	inx
	Jne *-8
.elseif(n = 3600)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif((n .mod 15 = 1) && ((n > 3600 && n < 3662)))
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3605 || n = 3622 || n = 3639 || n = 3656)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3611 || n = 3630 || n = 3649 || n = 3668)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 3612)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 3613 || n = 3655)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3615)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3617)
	; 11 bytes
	ldx #(n-1)/16
	delay_n 8
	bit $24
	dex
	Jne *-7
.elseif(n = 3621)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3637)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 3647 || n = 3669)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3653)
	; 11 bytes
	ldx #202 ;hides 'dex'
	.repeat 5
	 clv
	.endrepeat
	Jvc *+2
	Jne *-8
.endif
.else
; 3672 to 3750
.if(n = 3672 || n = 3674 || n = 3675 || n = 3677 || n = 3678 || n = 3680 || n = 3682 || n = 3683 || n = 3689 || n = 3692 || n = 3693 || n = 3696 || n = 3699 || n = 3703 || n = 3705 || n = 3708 || n = 3709 || n = 3712 || n = 3715 || n = 3720 || n = 3722 || n = 3723 || n = 3726 || n = 3728 || n = 3730 || n = 3731 || n = 3737 || n = 3738 || n = 3740 || n = 3742 || n = 3743 || n = 3745 || n = 3746 || n = 3748)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3673 || n = 3690 || n = 3707 || n = 3724 || n = 3741)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif((n .mod 15 = 1) && ((n > 3675 && n < 3737)))
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3679 || n = 3694 || n = 3698 || n = 3710 || n = 3714 || n = 3727 || n = 3739 || n = 3747)
	; 11..12 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3681 || n = 3701)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3684)
	; 13 bytes
	ldx #127
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 3685 || n = 3688 || n = 3702 || n = 3733)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3686 || n = 3716 || n = 3717)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3687 || n = 3725 || n = 3744)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 3695)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	bit $24
	dex
	dex
	Jne *-7
.elseif(n = 3697 || n = 3718)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3700)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 3704)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 3711)
	; 10 bytes
	ldx #24 ;hides 'clc'
	delay_n 6
	bit $24
	inx
	Jne *-7
.elseif(n = 3713 || n = 3729)
	; 11 bytes
	ldx #(n-1)/16
	delay_n 8
	bit $24
	dex
	Jne *-7
.elseif(n = 3719)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 3732)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 3734)
	; 10 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3735)
	; 10 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3749)
	; 10 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3750)
	; 10 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 6
	inx
	Jne *-7
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
.if(n = 3751 || n = 3766 || n = 3781)
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3752 || n = 3753 || n = 3755 || n = 3756 || n = 3759 || n = 3760 || n = 3762 || n = 3767 || n = 3768 || (n > 3769 && n < 3773) || n = 3777 || n = 3783 || (n > 3784 && n < 3789))
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3754 || n = 3761 || n = 3769 || n = 3778 || n = 3784)
	; 11 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3757)
	; 11 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 6
	bit $24
	inx
	Jne *-8
.elseif(n = 3758 || n = 3775)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3763 || n = 3782)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 3764)
	; 10 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3765)
	; 10 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3773)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 3774 || n = 3789)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3776)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 3779)
	; 10 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3780)
	; 10 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 6
	inx
	Jne *-7
.endif
.else
; 3790 to 3828
.if(n = 3790)
	; 14 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 3791 || n = 3806)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3792 || n = 3809)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3793)
	; 11 bytes
	ldx #(n-1)/16
	delay_n 8
	bit $24
	dex
	Jne *-7
.elseif(n = 3794 || n = 3797 || n = 3798 || n = 3800 || n = 3803 || n = 3805 || n = 3813 || n = 3815 || n = 3816 || n = 3818 || n = 3822 || n = 3827 || n = 3828)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3795)
	; 10 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3796 || n = 3811 || n = 3826)
	; 9 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3799)
	; 9 bytes
	ldx #56 ;hides 'sec'
	delay_n 12
	inx
	Jne *-6
.elseif(n = 3801 || n = 3820)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 3802 || n = 3823)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3804 || n = 3812 || n = 3814)
	; 11..12 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3807)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 3808)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 3810)
	; 10 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3817)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 3819)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 3821)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3824)
	; 10 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3825)
	; 10 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 6
	inx
	Jne *-7
.endif
.endif
.else
; 3829 to 3906
.if(n = 3829 || n = 3844 || n = 3846 || n = 3868 || n = 3874 || n = 3880 || n = 3892 || n = 3897)
	; 11..13 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n > 3829 && n < 3834) || n = 3837 || n = 3842 || n = 3845 || (n > 3846 && n < 3851) || n = 3855 || n = 3859 || n = 3862 || n = 3864 || n = 3866 || n = 3867 || n = 3869 || n = 3875 || n = 3878 || n = 3879 || (n > 3880 && n < 3885) || n = 3891 || n = 3893 || n = 3899 || n = 3900 || n = 3903)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3834 || n = 3885 || n = 3902)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3835 || n = 3853 || n = 3871 || n = 3889)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 3836 || n = 3870 || n = 3887 || n = 3904)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3838 || n = 3872 || n = 3906)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3839 || n = 3858 || n = 3896)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 3840)
	; 10 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 6
	inx
	Jne *-7
.elseif(n = 3841)
	; 9 bytes
	ldx #0
	delay_n 10
	dex
	Jne *-5
.elseif(n = 3843 || n = 3860 || n = 3877 || n = 3894)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3851 || n = 3895)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 3852 || n = 3854)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 3856)
	; 11 bytes
	ldx #202 ;hides 'dex'
	.repeat 7
	 nop
	.endrepeat
	Jne *-8
.elseif(n = 3857 || n = 3873 || n = 3905)
	; 11 bytes
	ldx #(n-1)/16
	delay_n 8
	bit $24
	dex
	Jne *-7
.elseif(n = 3861 || n = 3901)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3863)
	; 10 bytes
	ldx #184 ;hides 'clv'
	nop
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3865 || n = 3886)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3876)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 3888)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 3890)
	; 13 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 3898)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
; 3907 to 3984
.if(n = 3907 || n = 3970)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3908 || n = 3910 || n = 3914 || n = 3918 || n = 3924 || n = 3929 || n = 3931 || n = 3946 || n = 3948 || n = 3956 || n = 3965 || n = 3973 || n = 3975 || n = 3980 || n = 3982)
	; 11..13 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3909 || n = 3913 || n = 3916 || n = 3917 || n = 3919 || n = 3920 || n = 3923 || n = 3927 || n = 3930 || n = 3932 || n = 3933 || n = 3935 || n = 3936 || n = 3938 || n = 3944 || n = 3947 || (n > 3948 && n < 3953) || n = 3955 || n = 3957 || n = 3958 || n = 3963 || n = 3964 || (n > 3965 && n < 3969) || n = 3971 || n = 3974 || n = 3976 || n = 3981 || n = 3983)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3911 || n = 3928 || n = 3945 || n = 3962 || n = 3979)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3912)
	; 14 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 3915 || n = 3934 || n = 3953 || n = 3972)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 3921 || n = 3941)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 3922 || n = 3954)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 3925 || n = 3961)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 3926)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 8
	dex
	dex
	Jne *-7
.elseif(n = 3937 || n = 3969)
	; 11 bytes
	ldx #(n-1)/16
	delay_n 8
	bit $24
	dex
	Jne *-7
.elseif(n = 3939)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 3940)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3942)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 10
	inx
	Jne *-7
.elseif(n = 3943)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 10
	inx
	Jne *-6
.elseif(n = 3959)
	; 11 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 8
	inx
	Jne *-8
.elseif(n = 3960)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	nop
	delay_n 10
	inx
	Jne *-7
.elseif(n = 3977)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 10
	dex
	Jne *-6
.elseif(n = 3978)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 10
	inx
	Jne *-7
.elseif(n = 3984)
	; 11 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	bit $24
	inx
	Jne *-8
.endif
.else
; 3985 to 4062
.if(n = 3985)
	; 11 bytes
	ldx #(n-1)/16
	delay_n 8
	bit $24
	dex
	Jne *-7
.elseif(n = 3986 || n = 3987 || n = 3993 || n = 3995 || n = 3998 || n = 4000 || n = 4002 || n = 4015 || (n > 4016 && n < 4021) || n = 4023 || n = 4028 || n = 4031 || n = 4032 || (n > 4033 && n < 4038) || n = 4043 || n = 4045 || n = 4049 || (n > 4050 && n < 4054) || n = 4056 || n = 4058)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3988 || n = 3990 || n = 4007)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 3989 || n = 4006 || n = 4040 || n = 4057)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3991 || n = 4010 || n = 4029 || n = 4048)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 3992)
	; 14 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 3994 || n = 3999 || n = 4016 || n = 4033 || n = 4044 || n = 4050)
	; 11..13 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 3996 || n = 4013 || n = 4030 || n = 4047)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 3997)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 4001 || n = 4021 || n = 4041 || n = 4061)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4003 || n = 4026)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4004 || n = 4038 || n = 4055)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4005 || n = 4027)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4008 || n = 4025 || n = 4042)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4009)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 4011 || n = 4062)
	; 14 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 4012 || n = 4054)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4014)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4022)
	; 14 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 4024)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 4039)
	; 14 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4046)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4059)
	; 12 bytes
	ldx #202 ;hides 'dex'
	.repeat 6
	 clv
	.endrepeat
	Jvc *+2
	Jne *-9
.elseif(n = 4060)
	; 14 bytes
	delay_n 81
	delay_n (n-81)
.endif
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
.if((n >= 4063) && (n < 4102))
; 4063 to 4101
.if(n = 4063)
	; 11 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	bit $24
	inx
	Jne *-8
.elseif(n = 4064 || n = 4081 || n = 4098)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4065)
	; 11 bytes
	ldx #(n-1)/16
	delay_n 8
	bit $24
	dex
	Jne *-7
.elseif(n = 4066 || n = 4068 || n = 4069 || n = 4071 || n = 4073 || n = 4077 || n = 4082 || n = 4083 || n = 4085 || n = 4088 || n = 4090 || n = 4092 || n = 4099 || n = 4100)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4067 || n = 4086)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4070 || n = 4084 || n = 4089 || n = 4101)
	; 11..12 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4072)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4074 || n = 4091)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4075 || n = 4096)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4076)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4078)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 4079)
	; 11 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	bit $24
	inx
	Jne *-8
.elseif(n = 4080)
	; 11 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	bit $24
	inx
	Jne *-8
.elseif(n = 4087)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 4093)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4094)
	; 14 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4095)
	; 11 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	bit $24
	dex
	Jne *-8
.elseif(n = 4097)
	; 11 bytes
	ldx #0
	delay_n 8
	bit $24
	dex
	Jne *-7
.endif
.else
; 4102 to 4140
.if((n > 4101 && n < 4105) || n = 4107 || n = 4109 || n = 4111 || n = 4117 || n = 4119 || n = 4120 || n = 4122 || n = 4125 || n = 4126 || (n > 4127 && n < 4131) || n = 4134 || n = 4136 || n = 4137 || n = 4139 || n = 4140)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4105 || n = 4124)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4106)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4108 || n = 4118 || n = 4127 || n = 4135)
	; 11..12 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4110)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4112)
	; 14 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4113)
	; 14 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 4114 || n = 4131)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4115 || n = 4132)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4116)
	; 11 bytes
	ldx #232 ;hides 'inx'
	nop
	delay_n 12
	inx
	inx
	Jne *-8
.elseif(n = 4121)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4123)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 4133)
	; 14 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 4138)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.endif
.endif
.else
; 4141 to 4218
.if(n = 4141 || n = 4177 || n = 4195 || n = 4213)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 4142 || n = 4193)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4143 || n = 4162 || n = 4181)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4144 || n = 4178 || n = 4212)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4145 || n = 4147 || n = 4151 || (n > 4152 && n < 4157) || n = 4163 || n = 4164 || n = 4167 || n = 4168 || (n > 4169 && n < 4174) || n = 4179 || n = 4182 || n = 4185 || (n > 4186 && n < 4191) || n = 4197 || n = 4202 || (n > 4203 && n < 4208) || n = 4215)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4146 || n = 4152 || n = 4165 || n = 4169 || n = 4184 || n = 4186 || n = 4198 || n = 4203 || n = 4214 || n = 4216)
	; 11..13 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4148)
	; 13 bytes
	ldx #143
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4149 || n = 4166 || n = 4183 || n = 4200 || n = 4217)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4150 || n = 4218)
	; 14 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 4157)
	; 11 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	bit $24
	dex
	dex
	Jne *-8
.elseif(n = 4158 || n = 4192 || n = 4209)
	; 14 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4159 || n = 4180 || n = 4201)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4160 || n = 4194)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 4161)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4174 || n = 4191 || n = 4208)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4175)
	; 11 bytes
	ldx #24 ;hides 'clc'
	delay_n 8
	bit $24
	inx
	Jne *-8
.elseif(n = 4176)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4196)
	; 14 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4199)
	; 10 bytes
	ldx #56 ;hides 'sec'
	nop
	delay_n 12
	inx
	Jne *-7
.elseif(n = 4210)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4211)
	; 11 bytes
	ldx #234 ;hides 'nop'
	delay_n 8
	bit $24
	dex
	Jne *-8
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
; 4219 to 4296
.if(n = 4219 || n = 4238 || n = 4257 || n = 4276 || n = 4295)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4220 || n = 4237 || n = 4254 || n = 4260 || n = 4271 || n = 4279 || n = 4288)
	; 11..12 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4221)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4222 || n = 4243 || n = 4264)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4223 || n = 4224 || n = 4235 || n = 4236 || n = 4239 || n = 4240 || n = 4242 || n = 4245 || n = 4252 || n = 4253 || n = 4255 || n = 4256 || n = 4258 || n = 4259 || n = 4263 || n = 4265 || n = 4269 || n = 4270 || (n > 4271 && n < 4276) || n = 4278 || n = 4280 || n = 4282 || n = 4286 || n = 4287 || (n > 4288 && n < 4293))
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4225 || n = 4247)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4226)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4227 || n = 4244)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4228 || n = 4296)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 4229 || n = 4246)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4230)
	; 14 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4231 || n = 4249 || n = 4267)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 4232)
	; 11 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 8
	inx
	Jne *-8
.elseif(n = 4233)
	; 11 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 8
	inx
	Jne *-8
.elseif(n = 4234 || n = 4251 || n = 4268 || n = 4285)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4241 || n = 4261 || n = 4281)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4248)
	; 14 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4250)
	; 11 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 8
	inx
	Jne *-8
.elseif(n = 4262)
	; 12 bytes
	ldx #202 ;hides 'dex'
	.repeat 8
	 nop
	.endrepeat
	Jne *-9
.elseif(n = 4266)
	; 11 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 8
	inx
	Jne *-8
.elseif(n = 4277)
	; 14 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4283)
	; 11 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 8
	inx
	Jne *-8
.elseif(n = 4284)
	; 11 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 8
	inx
	Jne *-8
.elseif(n = 4293)
	; 13 bytes
	ldx #148
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4294)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.endif
.else
; 4297 to 4375
.if(n = 4297 || n = 4299 || n = 4304 || (n > 4305 && n < 4310) || n = 4315 || n = 4316 || n = 4320 || n = 4321 || (n > 4322 && n < 4326) || n = 4328 || n = 4329 || n = 4335 || n = 4337 || n = 4338 || n = 4340 || n = 4342 || n = 4343 || n = 4345 || n = 4350 || n = 4354 || n = 4355 || (n > 4356 && n < 4361) || n = 4363 || n = 4373)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4298 || n = 4305 || n = 4322 || n = 4330 || n = 4339 || n = 4356 || n = 4372 || n = 4374)
	; 11..13 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4300)
	; 11 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 8
	inx
	Jne *-8
.elseif(n = 4301 || n = 4341 || n = 4361)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4302 || n = 4319 || n = 4336)
	; 9 bytes
	ldx #(n-1)/17
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4303 || n = 4375)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 4310 || n = 4344)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4311 || n = 4362)
	; 14 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4312 || n = 4346)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4313)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4314 || n = 4333 || n = 4352 || n = 4371)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4317)
	; 11 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 8
	inx
	Jne *-8
.elseif(n = 4318)
	; 11 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 8
	inx
	Jne *-8
.elseif(n = 4326)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4327 || n = 4348 || n = 4369)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4331 || n = 4365)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4332 || n = 4349 || n = 4366)
	; 14 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4334)
	; 11 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 8
	inx
	Jne *-8
.elseif(n = 4347 || n = 4364)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 4351)
	; 11 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 8
	dex
	Jne *-8
.elseif(n = 4353)
	; 9 bytes
	ldx #0
	delay_n 12
	dex
	Jne *-5
.elseif(n = 4367)
	; 14 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 4368)
	; 14 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 4370)
	; 14 bytes
	delay_n 51
	delay_n (n-51)
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
.if(n = 4376)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4377 || (n > 4381 && n < 4387) || n = 4392 || (n > 4393 && n < 4398) || (n > 4401 && n < 4407) || n = 4414)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4378 || n = 4391 || n = 4410 || n = 4412)
	; 11..13 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4379)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4380)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 12
	inx
	Jne *-7
.elseif(n = 4381 || n = 4401)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4387)
	; 14 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 4388)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 10
	dex
	dex
	Jne *-7
.elseif(n = 4389)
	; 14 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4390 || n = 4409)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4393)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 4398)
	; 14 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4399)
	; 11 bytes
	ldx #56 ;hides 'sec'
	bit $24
	delay_n 12
	inx
	Jne *-8
.elseif(n = 4400)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	nop
	delay_n 12
	inx
	Jne *-7
.elseif(n = 4407)
	; 9 bytes
	ldx #24 ;hides 'clc'
	delay_n 12
	inx
	Jne *-6
.elseif(n = 4408)
	; 14 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4411)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4413)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
; 4415 to 4453
.if(n = 4415)
	; 12 bytes
	ldx #184 ;hides 'clv'
	nop
	bit $24
	delay_n 12
	dex
	Jne *-9
.elseif(n = 4416)
	; 14 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 4417 || n = 4440)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4418)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 4419)
	; 13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4420)
	; 10 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 12
	inx
	Jne *-7
.elseif(n = 4421 || n = 4441)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4422 || n = 4424 || n = 4430 || n = 4433 || n = 4434 || n = 4436 || n = 4442 || n = 4443 || n = 4449 || n = 4451 || n = 4452)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4423)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4425)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4426)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4427)
	; 14 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4428 || n = 4447)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4429)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 4431 || n = 4435 || n = 4444 || n = 4448 || n = 4450)
	; 11..13 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4432 || n = 4453)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4437)
	; 14 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 4438)
	; 13 bytes
	ldx #153
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4439)
	; 14 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 4445)
	; 9 bytes
	ldx #234 ;hides 'nop'
	delay_n 12
	dex
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
.elseif(n = 4455 || n = 4458 || n = 4467 || n = 4468 || (n > 4469 && n < 4473) || n = 4476 || n = 4478 || n = 4487 || (n > 4488 && n < 4492))
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4456)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
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
	; 13 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 4461 || n = 4481)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4462 || n = 4479)
	; 14 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 4463 || n = 4486)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4464 || n = 4469 || n = 4477 || n = 4484 || n = 4488)
	; 12..13 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4465 || n = 4483)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 4466 || n = 4485)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4473)
	; 14 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 4474)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4475 || n = 4492)
	; 14 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 4480)
	; 14 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 4482)
	; 12 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 6
	bit $24
	inx
	Jne *-9
.endif
.else
; 4493 to 4531
.if(n = 4493)
	; 14 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 4494)
	; 14 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4495 || n = 4516)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4496)
	; 13 bytes
	ldx #155
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4497 || n = 4502 || n = 4503 || n = 4505 || n = 4506 || n = 4508 || n = 4512 || n = 4518 || n = 4520 || n = 4525 || (n > 4526 && n < 4531))
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4498 || n = 4507 || n = 4522 || n = 4524 || n = 4526)
	; 12..13 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4499)
	; 12 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 6
	bit $24
	inx
	Jne *-9
.elseif(n = 4500)
	; 12 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 6
	bit $24
	inx
	Jne *-9
.elseif(n = 4501 || n = 4519)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 4504 || n = 4523)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4509)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4510)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 4511)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4513)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 4514)
	; 14 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 4515)
	; 14 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 4517)
	; 12 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 6
	bit $24
	inx
	Jne *-9
.elseif(n = 4521)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4531)
	; 13 bytes
	delay_n 124
	delay_n (n-124)
.endif
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
.if((n >= 4532) && (n < 4571))
; 4532 to 4570
.if(n = 4532)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4533)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4534 || n = 4538 || n = 4539 || n = 4543 || n = 4544 || (n > 4545 && n < 4549) || n = 4556 || n = 4557 || n = 4559 || n = 4560 || n = 4562 || n = 4563 || (n > 4564 && n < 4569))
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4535)
	; 12 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 6
	bit $24
	inx
	Jne *-9
.elseif(n = 4536)
	; 12 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 6
	bit $24
	inx
	Jne *-9
.elseif(n = 4537 || n = 4558)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4540 || n = 4545 || n = 4564)
	; 12..13 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4541)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4542 || n = 4561)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4549)
	; 14 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 4550)
	; 14 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4551)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4552)
	; 14 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4553)
	; 12 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 6
	bit $24
	inx
	Jne *-9
.elseif(n = 4554)
	; 12 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 6
	bit $24
	inx
	Jne *-9
.elseif(n = 4555)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 4569)
	; 13 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 4570)
	; 15 bytes
	delay_n 66
	delay_n (n-66)
.endif
.else
; 4571 to 4609
.if(n = 4571)
	; 12 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 6
	bit $24
	inx
	Jne *-9
.elseif(n = 4572)
	; 12 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 6
	bit $24
	inx
	Jne *-9
.elseif(n = 4573 || n = 4591)
	; 11 bytes
	ldx #(n-1)/18
	bit $24
	delay_n 10
	dex
	Jne *-7
.elseif(n = 4574 || n = 4575 || n = 4582 || (n > 4583 && n < 4588) || n = 4592 || n = 4593 || n = 4595 || n = 4597 || n = 4601 || (n > 4602 && n < 4607))
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4576)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4577)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4578)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4579 || n = 4600)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4580 || n = 4599)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4581)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4583 || n = 4594 || n = 4602)
	; 12..13 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4588)
	; 14 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 4589)
	; 12 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 6
	bit $24
	inx
	Jne *-9
.elseif(n = 4590)
	; 12 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 6
	bit $24
	inx
	Jne *-9
.elseif(n = 4596)
	; 14 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 4598)
	; 14 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 4607)
	; 12 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 6
	bit $24
	dex
	Jne *-9
.elseif(n = 4608)
	; 12 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 6
	bit $24
	inx
	Jne *-9
.elseif(n = 4609)
	; 11 bytes
	ldx #0
	bit $24
	delay_n 10
	dex
	Jne *-7
.endif
.endif
.else
.if((n >= 4610) && (n < 4649))
; 4610 to 4648
.if(n = 4610 || n = 4611 || (n > 4612 && n < 4616) || n = 4620 || n = 4622 || n = 4623 || n = 4625 || n = 4628 || n = 4635 || n = 4639 || n = 4644 || n = 4646 || n = 4648)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4612)
	; 13 bytes
	ldx #159
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4616)
	; 14 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 4617)
	; 15 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 4618 || n = 4637)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4619)
	; 12 bytes
	ldx #232 ;hides 'inx'
	delay_n 8
	bit $24
	dex
	dex
	Jne *-9
.elseif(n = 4621 || n = 4642)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4624 || n = 4647)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4626)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4627 || n = 4640)
	; 12..14 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4629)
	; 14 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 4630)
	; 14 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4631)
	; 14 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 4632)
	; 15 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 4633)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 4634)
	; 15 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4636)
	; 14 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 4638)
	; 15 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4641)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4643)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4645)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.endif
.else
; 4649 to 4687
.if(n = 4649 || n = 4658 || n = 4660 || n = 4662 || n = 4665 || n = 4667 || n = 4669 || n = 4674 || n = 4677 || n = 4680 || n = 4683 || n = 4685 || n = 4686)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4650 || n = 4654 || n = 4659 || n = 4664 || n = 4666 || n = 4673 || n = 4678 || n = 4682)
	; 12..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4651 || n = 4676)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4652)
	; 15 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4653)
	; 13 bytes
	ldx #232 ;hides 'inx'
	nop
	bit $24
	delay_n 12
	inx
	inx
	Jne *-10
.elseif(n = 4655)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 4656 || n = 4675)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4657)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 4661 || n = 4681)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4663 || n = 4684)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4668)
	; 13 bytes
	ldx #202 ;hides 'dex'
	.repeat 9
	 nop
	.endrepeat
	Jne *-10
.elseif(n = 4670)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4671)
	; 15 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4672)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 4679)
	; 11 bytes
	ldx #234 ;hides 'nop'
	bit $24
	delay_n 10
	dex
	Jne *-8
.elseif(n = 4687)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.endif
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
; 4688 to 4765
.if((n > 4687 && n < 4692) || n = 4695 || n = 4696 || n = 4698 || n = 4700 || n = 4703 || n = 4707 || n = 4710 || n = 4711 || n = 4715 || (n > 4716 && n < 4721) || n = 4723 || n = 4725 || n = 4727 || n = 4728 || n = 4733 || n = 4734 || (n > 4735 && n < 4739) || n = 4743 || n = 4745 || n = 4746 || n = 4752 || n = 4753 || n = 4755 || n = 4756 || n = 4758 || n = 4759 || (n > 4762 && n < 4766))
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4692)
	; 15 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 4693 || n = 4739 || n = 4762)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4694 || n = 4713 || n = 4732 || n = 4751)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4697 || n = 4704 || n = 4708 || n = 4712 || n = 4716 || n = 4724 || n = 4729 || n = 4735 || n = 4744 || n = 4754 || n = 4760)
	; 12..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4699)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 4701 || n = 4721 || n = 4741 || n = 4761)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4702 || n = 4740)
	; 14 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4705 || n = 4726 || n = 4747)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4706)
	; 14 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4709)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4714)
	; 15 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 4722)
	; 15 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 4730)
	; 11 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 10
	inx
	Jne *-8
.elseif(n = 4731)
	; 11 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 10
	inx
	Jne *-8
.elseif(n = 4742)
	; 14 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4748)
	; 15 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4749)
	; 11 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 10
	inx
	Jne *-8
.elseif(n = 4750)
	; 11 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 10
	inx
	Jne *-8
.elseif(n = 4757)
	; 13 bytes
	ldx #164
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.endif
.else
.if((n >= 4766) && (n < 4805))
; 4766 to 4804
.if(n = 4766 || n = 4767 || n = 4771 || n = 4772 || n = 4774 || n = 4778 || n = 4782 || n = 4783 || n = 4790 || n = 4791 || (n > 4792 && n < 4797) || n = 4803)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4768)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4769)
	; 11 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 10
	inx
	Jne *-8
.elseif(n = 4770 || n = 4789)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4773 || n = 4784 || n = 4792 || n = 4802 || n = 4804)
	; 12..14 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4775 || n = 4797)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4776)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4777)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 4779)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4780)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 4781 || n = 4801)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4785)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4786)
	; 13 bytes
	ldx #165
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4787)
	; 11 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 10
	inx
	Jne *-8
.elseif(n = 4788)
	; 11 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 10
	inx
	Jne *-8
.elseif(n = 4798)
	; 15 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 4799)
	; 12 bytes
	ldx #56 ;hides 'sec'
	nop
	bit $24
	delay_n 12
	inx
	Jne *-9
.elseif(n = 4800)
	; 14 bytes
	delay_n 11
	delay_n (n-11)
.endif
.else
; 4805 to 4843
.if(n = 4805 || n = 4809 || n = 4813 || n = 4814 || n = 4817 || n = 4820 || n = 4822 || n = 4823 || n = 4828 || n = 4829 || n = 4832 || n = 4833 || n = 4835 || n = 4836 || n = 4842 || n = 4843)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4806)
	; 11 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 10
	inx
	Jne *-8
.elseif(n = 4807)
	; 11 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 10
	inx
	Jne *-8
.elseif(n = 4808 || n = 4827)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4810 || n = 4831)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4811 || n = 4824 || n = 4830)
	; 12..13 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4812)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4815)
	; 13 bytes
	ldx #166
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4816)
	; 14 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4818)
	; 11 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	delay_n 12
	inx
	Jne *-8
.elseif(n = 4819)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4821 || n = 4841)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4825)
	; 11 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 10
	inx
	Jne *-8
.elseif(n = 4826)
	; 11 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 10
	inx
	Jne *-8
.elseif(n = 4834)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 4837)
	; 14 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4838)
	; 14 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 4839)
	; 11 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 12
	inx
	Jne *-8
.elseif(n = 4840)
	; 11 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 4
	delay_n 12
	inx
	Jne *-8
.endif
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
; 4844 to 4882
.if(n = 4844)
	; 11 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 10
	inx
	Jne *-8
.elseif(n = 4845)
	; 11 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 10
	inx
	Jne *-8
.elseif(n = 4846)
	; 10 bytes
	ldx #(n-1)/19
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4847 || n = 4848 || n = 4857 || n = 4866 || n = 4867 || n = 4869 || n = 4870 || n = 4872 || n = 4875 || n = 4878 || n = 4879)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4849 || n = 4853 || n = 4855 || n = 4868 || n = 4874 || n = 4880)
	; 12..14 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4850)
	; 10 bytes
	ldx #232 ;hides 'inx'
	delay_n 12
	dex
	dex
	Jne *-7
.elseif(n = 4851 || n = 4876)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4852 || n = 4873)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4854 || n = 4877)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
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
	; 14 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4861 || n = 4881)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4862)
	; 11 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	delay_n 12
	inx
	Jne *-8
.elseif(n = 4863)
	; 11 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 10
	dex
	Jne *-8
.elseif(n = 4864)
	; 11 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 10
	inx
	Jne *-8
.elseif(n = 4865)
	; 10 bytes
	ldx #0
	nop
	delay_n 12
	dex
	Jne *-6
.elseif(n = 4871)
	; 10 bytes
	ldx #24 ;hides 'clc'
	nop
	delay_n 12
	inx
	Jne *-7
.elseif(n = 4882)
	; 14 bytes
	delay_n 11
	delay_n (n-11)
.endif
.else
; 4883 to 4921
.if(n = 4883 || n = 4886 || n = 4887 || n = 4889 || n = 4890 || n = 4896 || n = 4898 || n = 4903 || n = 4905 || n = 4906 || n = 4909 || n = 4911 || n = 4917 || n = 4919 || n = 4920)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4884 || n = 4891 || n = 4897 || n = 4904 || n = 4910 || n = 4916 || n = 4918)
	; 12..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4885 || n = 4907)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4888)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 4892)
	; 15 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 4893 || n = 4912)
	; 15 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 4894 || n = 4915)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4895 || n = 4914)
	; 15 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 4899)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 4900)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4901 || n = 4921)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4902)
	; 13 bytes
	ldx #169
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4908)
	; 15 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 4913)
	; 10 bytes
	ldx #234 ;hides 'nop'
	nop
	delay_n 12
	dex
	Jne *-7
.endif
.endif
.else
; 4922 to 5000
.if(n = 4922)
	; 15 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 4923 || n = 4946 || n = 4969 || n = 4992)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4924 || n = 4934 || n = 4939 || n = 4944 || n = 4949 || n = 4954 || n = 4964 || n = 4972 || n = 4984)
	; 13..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 4925 || n = 4927 || n = 4928 || n = 4933 || n = 4938 || n = 4940 || n = 4943 || n = 4945 || n = 4947 || n = 4948 || n = 4950 || n = 4953 || n = 4955 || n = 4959 || n = 4962 || n = 4963 || n = 4965 || n = 4966 || n = 4971 || n = 4975 || n = 4977 || n = 4980 || n = 4982 || n = 4983 || (n > 4984 && n < 4988) || n = 4991 || n = 4994 || n = 4997 || n = 4998)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4926 || n = 4976)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4929 || n = 4951 || n = 4973 || n = 4995)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 4930)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 4931)
	; 13 bytes
	ldx #170
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4932 || n = 4970 || n = 4974)
	; 14 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 4935)
	; 15 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 4936 || n = 4957 || n = 4978 || n = 4999)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4937 || n = 4956)
	; 15 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 4941 || n = 4961 || n = 4981)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 4942 || n = 4996)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
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
	ldx #171
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4967)
	; 13 bytes
	ldx #184 ;hides 'clv'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-10
.elseif(n = 4968)
	; 15 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 4979)
	; 13 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 8
	bit $24
	inx
	Jne *-10
.elseif(n = 4988)
	; 15 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4989)
	; 13 bytes
	ldx #172
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 4990)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4993)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5000)
	; 13 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 8
	bit $24
	inx
	Jne *-10
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
.if(n = 5001 || n = 5021)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 5002 || n = 5004 || n = 5029)
	; 13..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5003 || (n > 5004 && n < 5008) || n = 5019 || (n > 5021 && n < 5026) || n = 5027 || n = 5028 || n = 5030)
	; 12..15 bytes
	delay_n (n-2)
	nop
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
	; 14 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	delay_n 12
	inx
	inx
	Jne *-11
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
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5016)
	; 15 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5017 || n = 5039)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5018)
	; 13 bytes
	ldx #173
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5020)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 5026)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5033)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
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
	; 12 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	bit $24
	delay_n 12
	inx
	Jne *-9
.endif
.else
; 5040 to 5078
.if(n = 5040 || n = 5042 || n = 5043 || n = 5045 || n = 5046 || n = 5049 || n = 5052 || n = 5053 || n = 5055 || n = 5058 || n = 5063 || n = 5064 || (n > 5065 && n < 5070) || n = 5078)
	; 12..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5041 || n = 5062)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 5044 || n = 5065)
	; 13 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5047)
	; 13 bytes
	ldx #174
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5048)
	; 14 bytes
	ldx #202 ;hides 'dex'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-11
.elseif(n = 5050 || n = 5077)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5051 || n = 5076)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5054)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 13 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	bit $24
	delay_n 10
	inx
	Jne *-10
.elseif(n = 5060)
	; 12 bytes
	ldx #36 ;hides 'bit zp'
	nop
	bit $24
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5061)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 5070)
	; 15 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5071)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 5072)
	; 15 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5073)
	; 15 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5074)
	; 14 bytes
	ldx #202 ;hides 'dex'
	.repeat 10
	 nop
	.endrepeat
	Jne *-11
.elseif(n = 5075)
	; 16 bytes
	delay_n 76
	delay_n (n-76)
.endif
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if(n = 5079)
	; 13 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 8
	bit $24
	inx
	Jne *-10
.elseif(n = 5080)
	; 13 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 8
	bit $24
	inx
	Jne *-10
.elseif(n = 5081 || n = 5101)
	; 11 bytes
	ldx #(n-1)/20
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 5082 || n = 5085 || n = 5087 || n = 5088 || n = 5090 || n = 5091 || n = 5102 || n = 5106 || n = 5108 || n = 5109 || n = 5111 || n = 5112 || n = 5115 || n = 5117)
	; 12..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5083 || n = 5104)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 5084 || n = 5107)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5086 || n = 5092 || n = 5110)
	; 13..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5089 || n = 5113)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5093 || n = 5114)
	; 15 bytes
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
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 5098)
	; 16 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5099)
	; 13 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 8
	bit $24
	inx
	Jne *-10
.elseif(n = 5100)
	; 13 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 8
	bit $24
	inx
	Jne *-10
.elseif(n = 5103)
	; 11 bytes
	ldx #24 ;hides 'clc'
	bit $24
	delay_n 12
	inx
	Jne *-8
.elseif(n = 5105)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5116)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.endif
.else
; 5118 to 5156
.if(n = 5118 || n = 5122 || n = 5123 || n = 5127 || n = 5129 || n = 5132 || n = 5133 || n = 5135 || n = 5136 || n = 5138 || n = 5139 || n = 5141 || n = 5148 || n = 5150 || n = 5152 || n = 5155)
	; 12..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5119)
	; 13 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 8
	bit $24
	dex
	Jne *-10
.elseif(n = 5120)
	; 13 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 8
	bit $24
	inx
	Jne *-10
.elseif(n = 5121)
	; 11 bytes
	ldx #0
	bit $24
	delay_n 12
	dex
	Jne *-7
.elseif(n = 5124 || n = 5128 || n = 5140 || n = 5154 || n = 5156)
	; 13..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5125 || n = 5146)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 5126 || n = 5151)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5130 || n = 5153)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5131)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5134)
	; 13 bytes
	ldx #177
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5137)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5142)
	; 16 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 5143)
	; 16 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5144 || n = 5145)
	; 15 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 5147)
	; 11 bytes
	ldx #234 ;hides 'nop'
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5149)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.endif
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
; 5157 to 5234
.if(n = 5157 || n = 5159 || n = 5160 || n = 5162 || n = 5165 || n = 5168 || n = 5169 || n = 5172 || n = 5173 || n = 5175 || n = 5178 || n = 5180 || n = 5181 || n = 5187 || n = 5189 || n = 5190 || n = 5194 || n = 5195 || n = 5197 || n = 5203 || n = 5205 || n = 5206 || n = 5211 || n = 5213 || n = 5214 || n = 5216 || n = 5217 || n = 5219 || n = 5223 || n = 5224 || n = 5231 || n = 5232 || n = 5234)
	; 12..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5158 || n = 5185 || n = 5212)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5161)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5163)
	; 13 bytes
	ldx #178
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5164 || n = 5166 || n = 5170 || n = 5174 || n = 5179 || n = 5191 || n = 5196 || n = 5202 || n = 5204 || n = 5218 || n = 5225 || n = 5233)
	; 13..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5167 || n = 5188 || n = 5209 || n = 5230)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 5171 || n = 5193 || n = 5215)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5176 || n = 5199 || n = 5222)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5177 || n = 5198)
	; 15 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5182)
	; 15 bytes
	ldx #157
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5183)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5184)
	; 16 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 5186 || n = 5207)
	; 15 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 5192)
	; 13 bytes
	ldx #179
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5200)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5201 || n = 5226)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5208)
	; 16 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 5210)
	; 16 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 5220)
	; 15 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5221)
	; 13 bytes
	ldx #180
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5227)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 5228)
	; 11 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 12
	inx
	Jne *-8
.elseif(n = 5229)
	; 11 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 12
	inx
	Jne *-8
.endif
.else
.if((n >= 5235) && (n < 5274))
; 5235 to 5273
.if(n = 5235 || n = 5236 || n = 5241 || n = 5243 || n = 5244 || n = 5247 || n = 5252 || n = 5253 || n = 5255 || n = 5258 || n = 5260 || n = 5261 || n = 5263 || n = 5264 || n = 5273)
	; 12..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5237 || n = 5259)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5238)
	; 15 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5239 || n = 5266)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5240)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 5242 || n = 5248 || n = 5254 || n = 5262 || n = 5269)
	; 13..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5245 || n = 5268)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5246 || n = 5267)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5249)
	; 11 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 12
	inx
	Jne *-8
.elseif(n = 5250)
	; 11 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 12
	inx
	Jne *-8
.elseif(n = 5251 || n = 5272)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 5256)
	; 12 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 4
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5257)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5265)
	; 15 bytes
	ldx #188
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5270)
	; 11 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 12
	inx
	Jne *-8
.elseif(n = 5271)
	; 11 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 12
	inx
	Jne *-8
.endif
.else
; 5274 to 5312
.if(n = 5274 || n = 5277 || n = 5278 || n = 5282 || n = 5283 || (n > 5284 && n < 5288) || n = 5294 || n = 5295 || (n > 5296 && n < 5301) || n = 5306 || n = 5309 || n = 5310)
	; 12..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5275 || n = 5284 || n = 5296 || n = 5311)
	; 13..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5276 || n = 5301)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5279)
	; 12 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5280)
	; 12 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 6
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5281 || n = 5303)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5288)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5289)
	; 16 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 5290)
	; 16 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 5291)
	; 11 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 12
	inx
	Jne *-8
.elseif(n = 5292)
	; 11 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 12
	inx
	Jne *-8
.elseif(n = 5293)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 5302)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 5304)
	; 12 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 4
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5305)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5307)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 5308)
	; 13 bytes
	ldx #183
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5312)
	; 11 bytes
	ldx #232 ;hides 'inx'
	nop
	delay_n 12
	dex
	dex
	Jne *-8
.endif
.endif
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
; 5313 to 5390
.if(n = 5313)
	; 11 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 12
	inx
	Jne *-8
.elseif(n = 5314 || n = 5335 || n = 5356)
	; 11 bytes
	ldx #(n-1)/21
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 5315 || n = 5316 || n = 5318 || n = 5319 || n = 5321 || n = 5322 || n = 5324 || n = 5327 || n = 5328 || n = 5330 || n = 5331 || n = 5336 || (n > 5339 && n < 5343) || n = 5349 || n = 5352 || n = 5357 || n = 5358 || n = 5361 || n = 5362 || n = 5365 || n = 5368 || n = 5370 || n = 5371 || n = 5373 || n = 5378 || n = 5379 || n = 5382 || n = 5385 || n = 5387 || n = 5388)
	; 12..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5317 || n = 5323 || n = 5332 || n = 5338 || n = 5350 || n = 5359 || n = 5363 || n = 5372 || n = 5380 || n = 5384 || n = 5386)
	; 13..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5320 || n = 5374)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5325 || n = 5347 || n = 5369)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5326 || n = 5351)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5329 || n = 5353)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5333)
	; 11 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 12
	inx
	Jne *-8
.elseif(n = 5334)
	; 11 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 12
	inx
	Jne *-8
.elseif(n = 5337 || n = 5360 || n = 5383)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5339)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 5343)
	; 15 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5344 || n = 5345)
	; 15 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5346 || n = 5367)
	; 15 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5348 || n = 5390)
	; 16 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 5354)
	; 11 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 12
	inx
	Jne *-8
.elseif(n = 5355)
	; 11 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 12
	inx
	Jne *-8
.elseif(n = 5364)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 5366)
	; 13 bytes
	ldx #185
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5375)
	; 11 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5376)
	; 11 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 12
	inx
	Jne *-8
.elseif(n = 5377)
	; 11 bytes
	ldx #0
	delay_n 4
	delay_n 12
	dex
	Jne *-7
.elseif(n = 5381)
	; 11 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5389)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
.if((n >= 5391) && (n < 5430))
; 5391 to 5429
.if(n = 5391 || n = 5413)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5392)
	; 15 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5393 || n = 5397 || n = 5400 || n = 5403 || n = 5405 || n = 5407 || n = 5408 || (n > 5409 && n < 5413) || n = 5415 || (n > 5416 && n < 5420) || n = 5427)
	; 13..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5394 || n = 5398 || n = 5402 || n = 5404 || n = 5409 || n = 5416)
	; 14..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5395)
	; 13 bytes
	ldx #186
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5396)
	; 15 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 5399)
	; 13 bytes
	ldx #56 ;hides 'sec'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-10
.elseif(n = 5401 || n = 5426)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5406 || n = 5429)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5414)
	; 16 bytes
	delay_n 121
	delay_n (n-121)
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
	ldx #187
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5425)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5428)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.endif
.else
; 5430 to 5468
.if(n = 5430 || n = 5431 || n = 5433 || n = 5434 || n = 5437 || n = 5439 || n = 5440 || n = 5454 || n = 5456 || n = 5459 || (n > 5460 && n < 5464))
	; 13..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5432 || n = 5438 || n = 5458 || n = 5460)
	; 14..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5435 || n = 5457)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5436)
	; 15 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 5441)
	; 15 bytes
	ldx #170
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5442)
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
	ldx #165
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5450)
	; 16 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 5451)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5452)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5453)
	; 13 bytes
	ldx #188
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5455)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
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
.if((n >= 5469) && (n < 5508))
; 5469 to 5507
.if(n = 5469)
	; 16 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 5470)
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
.elseif(n = 5473 || n = 5497)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5474 || n = 5494 || n = 5495)
	; 16 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 5475 || n = 5498)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5476)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5477 || n = 5480 || n = 5481 || n = 5483 || n = 5484 || n = 5486 || n = 5490 || n = 5500 || n = 5502 || n = 5503 || (n > 5504 && n < 5508))
	; 13..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5478)
	; 13 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	delay_n 10
	inx
	Jne *-10
.elseif(n = 5479 || n = 5501)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5482)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5485 || n = 5504)
	; 14..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5487)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 5488)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 5489)
	; 15 bytes
	ldx #196
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5491)
	; 15 bytes
	ldx #183
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 5496)
	; 16 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 5499)
	; 13 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	delay_n 10
	inx
	Jne *-10
.endif
.else
; 5508 to 5546
.if(n = 5508)
	; 16 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 5509 || n = 5536)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5510)
	; 16 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 5511)
	; 13 bytes
	ldx #190
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5512 || n = 5514 || n = 5524 || n = 5529)
	; 14..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5513 || n = 5515 || n = 5516 || n = 5525 || n = 5527 || n = 5528 || (n > 5529 && n < 5533) || n = 5538 || n = 5541 || n = 5542 || n = 5546)
	; 13..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5517)
	; 15 bytes
	ldx #197
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5518)
	; 16 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 5519)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 5520)
	; 16 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 5521 || n = 5544)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5522)
	; 13 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	delay_n 10
	inx
	Jne *-10
.elseif(n = 5523 || n = 5545)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5526)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5533)
	; 16 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 5534)
	; 16 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 5535)
	; 16 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5537)
	; 15 bytes
	ldx #173
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5539)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 5540)
	; 13 bytes
	ldx #191
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5543)
	; 12 bytes
	ldx #232 ;hides 'inx'
	bit $24
	delay_n 12
	dex
	dex
	Jne *-9
.endif
.endif
.else
.if((n >= 5547) && (n < 5586))
; 5547 to 5585
.if(n = 5547 || n = 5549 || n = 5552 || n = 5553 || (n > 5554 && n < 5558) || n = 5560 || n = 5568 || n = 5571 || (n > 5572 && n < 5576) || n = 5578 || n = 5580 || n = 5582 || n = 5583 || n = 5585)
	; 13..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 5548)
	; 14 bytes
	ldx #232 ;hides 'inx'
	delay_n 10
	delay_n 12
	inx
	inx
	Jne *-11
.elseif(n = 5550 || n = 5581)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 5551 || n = 5576)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5554 || n = 5570 || n = 5572 || n = 5579 || n = 5584)
	; 14..16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5558 || n = 5562)
	; 16 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 5559)
	; 16 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 5561)
	; 16 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 5563)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5564)
	; 17 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 5565)
	; 13 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 10
	inx
	Jne *-10
.elseif(n = 5566)
	; 13 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 10
	inx
	Jne *-10
.elseif(n = 5567)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5569)
	; 13 bytes
	ldx #192
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5577)
	; 16 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 5586 to 5625
.if(n = 5586 || n = 5608)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5587)
	; 13 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 10
	inx
	Jne *-10
.elseif(n = 5588)
	; 13 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 10
	inx
	Jne *-10
.elseif(n = 5589 || n = 5611)
	; 12 bytes
	ldx #(n-1)/22
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5590 || n = 5613)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5591 || n = 5592 || (n > 5593 && n < 5598) || n = 5600 || n = 5602 || n = 5603 || (n > 5604 && n < 5608) || n = 5619 || (n > 5620 && n < 5624))
	; 13..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5593)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5598)
	; 13 bytes
	ldx #193
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5599)
	; 14 bytes
	ldx #56 ;hides 'sec'
	delay_n 9
	delay_n 12
	inx
	Jne *-11
.elseif(n = 5601)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif((n .mod 2 = 0) && (n = 5604 || (n > 5613 && n < 5621)))
	; 14..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5609)
	; 13 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	delay_n 10
	inx
	Jne *-10
.elseif(n = 5610)
	; 13 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	delay_n 10
	inx
	Jne *-10
.elseif(n = 5612)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 5615)
	; 12 bytes
	ldx #234 ;hides 'nop'
	nop
	bit $24
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5617)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5624)
	; 16 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5625)
	; 16 bytes
	delay_n 12
	delay_n (n-12)
.endif
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
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5627)
	; 13 bytes
	ldx #194
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5628 || n = 5629 || n = 5634 || n = 5635 || n = 5637 || n = 5638 || n = 5640 || n = 5642 || n = 5645 || n = 5646 || (n > 5647 && n < 5651) || n = 5653 || n = 5655 || n = 5658 || n = 5660 || n = 5661 || n = 5663 || n = 5664 || n = 5666 || n = 5667 || n = 5673 || n = 5675 || n = 5678 || n = 5680 || n = 5681 || n = 5684 || n = 5686 || n = 5687 || n = 5690 || n = 5691 || n = 5693 || n = 5696 || n = 5699 || n = 5700 || n = 5702)
	; 13..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5630 || n = 5639 || n = 5647 || n = 5654 || n = 5662 || n = 5668 || n = 5672 || n = 5679 || n = 5688 || n = 5692)
	; 14..16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5631)
	; 13 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 10
	dex
	Jne *-10
.elseif(n = 5632)
	; 13 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 10
	inx
	Jne *-10
.elseif(n = 5633)
	; 12 bytes
	ldx #0
	nop
	bit $24
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5636 || n = 5659 || n = 5682)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5641 || n = 5665 || n = 5689)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5643 || n = 5674)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 5644 || n = 5671 || n = 5698)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5652)
	; 17 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5656)
	; 13 bytes
	ldx #195
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5657)
	; 15 bytes
	ldx #202
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5669 || n = 5695)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 5670)
	; 16 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5677)
	; 15 bytes
	ldx #172
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5683)
	; 16 bytes
	ldx #202 ;hides 'dex'
	.repeat 10
	 clv
	.endrepeat
	Jvc *+2
	Jne *-13
.elseif(n = 5685)
	; 13 bytes
	ldx #196
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5694)
	; 13 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 6
	delay_n 12
	inx
	Jne *-10
.elseif(n = 5697)
	; 15 bytes
	ldx #178
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5703)
	; 13 bytes
	ldx #184 ;hides 'clv'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.endif
.else
; 5704 to 5781
.if(n = 5704 || n = 5706 || n = 5708 || n = 5717 || n = 5731 || n = 5740 || n = 5764 || n = 5770 || n = 5777)
	; 14..16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5705 || n = 5728 || n = 5751 || n = 5774)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5707 || n = 5709 || n = 5711 || n = 5712 || n = 5715 || n = 5716 || n = 5718 || (n > 5721 && n < 5725) || n = 5729 || n = 5730 || (n > 5731 && n < 5736) || n = 5738 || n = 5739 || n = 5742 || n = 5745 || n = 5748 || (n > 5752 && n < 5759) || n = 5763 || n = 5765 || n = 5766 || n = 5769 || n = 5771 || n = 5775 || n = 5778 || n = 5780 || n = 5781)
	; 13..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5710)
	; 15 bytes
	ldx #173
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5713 || n = 5737 || n = 5761)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5714)
	; 13 bytes
	ldx #197
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5719)
	; 13 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 4
	delay_n 12
	inx
	Jne *-10
.elseif(n = 5720)
	; 13 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 8
	delay_n 12
	inx
	Jne *-10
.elseif(n = 5721 || n = 5747)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 5725 || n = 5752 || n = 5779)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5726)
	; 12 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5727)
	; 12 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	nop
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5736 || n = 5767)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 5741)
	; 15 bytes
	ldx #205
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5743)
	; 13 bytes
	ldx #198
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5744)
	; 17 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5746)
	; 13 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 6
	delay_n 12
	inx
	Jne *-10
.elseif(n = 5749)
	; 12 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	nop
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5750)
	; 12 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5759)
	; 16 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5760 || n = 5768)
	; 16..17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 5762)
	; 16 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5772)
	; 12 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5773)
	; 12 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	nop
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5776)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
.if((n >= 5782) && (n < 5821))
; 5782 to 5820
.if(n = 5782 || n = 5788 || n = 5800 || n = 5802 || n = 5804 || n = 5812)
	; 14..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5783 || n = 5784 || n = 5787 || n = 5789 || n = 5790 || n = 5798 || n = 5803 || n = 5805 || n = 5807 || n = 5808 || n = 5810 || n = 5811 || n = 5813 || n = 5814)
	; 13..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5785 || n = 5809)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5786)
	; 16 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5791)
	; 15 bytes
	ldx #193
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 5792)
	; 17 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 5793)
	; 15 bytes
	ldx #181
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5794)
	; 17 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 5795)
	; 12 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	nop
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5796)
	; 12 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5797 || n = 5820)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5799)
	; 12 bytes
	ldx #24 ;hides 'clc'
	delay_n 6
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5801)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5806)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5815)
	; 16 bytes
	ldx #171
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 5816)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5817)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5818)
	; 12 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5819)
	; 12 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	nop
	delay_n 12
	inx
	Jne *-9
.endif
.else
; 5821 to 5859
.if(n = 5821 || n = 5822 || n = 5824 || n = 5827 || n = 5828 || n = 5831 || n = 5832 || n = 5834 || n = 5835 || (n > 5836 && n < 5840) || n = 5844 || n = 5845 || n = 5847 || n = 5848 || n = 5850 || n = 5853 || n = 5855 || n = 5856 || n = 5858)
	; 13..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 5823 || n = 5836 || n = 5846 || n = 5852 || n = 5854)
	; 14..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5825)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 5826 || n = 5851)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5829)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 5830)
	; 13 bytes
	ldx #201
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5833)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5840)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5841)
	; 12 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5842)
	; 12 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5843)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5849)
	; 12 bytes
	ldx #234 ;hides 'nop'
	delay_n 6
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5857)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5859)
	; 13 bytes
	ldx #202
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.endif
.endif
.else
.if((n >= 5860) && (n < 5899))
; 5860 to 5898
.if(n = 5860)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5861 || n = 5862 || n = 5867 || n = 5868 || (n > 5869 && n < 5874) || n = 5878 || n = 5880 || n = 5882 || n = 5883 || n = 5885 || n = 5890 || n = 5891 || (n > 5892 && n < 5897))
	; 13..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5863 || n = 5869 || n = 5879 || n = 5884 || n = 5892)
	; 14..15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5864)
	; 12 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5865)
	; 12 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5866)
	; 12 bytes
	ldx #(n-1)/23
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5874 || n = 5897)
	; 16 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5875)
	; 15 bytes
	ldx #178
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5876)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5877)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 5881)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5886)
	; 16 bytes
	ldx #202 ;hides 'dex'
	.repeat 12
	 nop
	.endrepeat
	Jne *-13
.elseif(n = 5887)
	; 12 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	nop
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5888)
	; 12 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	delay_n 12
	inx
	Jne *-9
.elseif(n = 5889)
	; 12 bytes
	ldx #0
	delay_n 6
	delay_n 12
	dex
	Jne *-8
.elseif(n = 5898)
	; 16 bytes
	delay_n 124
	delay_n (n-124)
.endif
.else
; 5899 to 5937
.if(n = 5899 || n = 5927 || n = 5936)
	; 16..17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5900 || n = 5912)
	; 16..17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 5901 || n = 5926)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5902)
	; 17 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 5903 || n = 5907 || n = 5910 || n = 5915 || n = 5916 || n = 5918 || n = 5919 || n = 5921 || n = 5923 || n = 5924 || n = 5928 || n = 5930 || n = 5931 || (n > 5932 && n < 5936))
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5904 || n = 5920 || n = 5925 || n = 5932)
	; 15..16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5905 || n = 5929)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5906)
	; 14 bytes
	ldx #232 ;hides 'inx'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	inx
	Jne *-11
.elseif(n = 5908)
	; 15 bytes
	ldx #179
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5909)
	; 15 bytes
	ldx #211
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5911)
	; 15 bytes
	ldx #197
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 5913)
	; 14 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 7
	delay_n 12
	inx
	Jne *-11
.elseif(n = 5914)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5917)
	; 13 bytes
	ldx #204
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5922)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 5937)
	; 15 bytes
	ldx #212
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
.if((n >= 5938) && (n < 6016))
.if((n >= 5938) && (n < 5977))
; 5938 to 5976
.if(n = 5938 || n = 5963)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5939)
	; 14 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	bit $24
	delay_n 12
	inx
	Jne *-11
.elseif(n = 5940)
	; 14 bytes
	ldx #36 ;hides 'bit zp'
	nop
	delay_n 7
	delay_n 12
	inx
	Jne *-11
.elseif(n = 5941 || n = 5968)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5942 || n = 5943 || n = 5945 || n = 5947 || n = 5948 || n = 5950 || n = 5952 || n = 5955 || (n > 5956 && n < 5960) || n = 5969 || n = 5970 || n = 5972 || n = 5973)
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5944 || n = 5949 || n = 5954 || n = 5956 || n = 5971)
	; 15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5946)
	; 13 bytes
	ldx #205
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5951 || n = 5976)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 5953)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
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
.elseif(n = 5964)
	; 17 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 5965)
	; 15 bytes
	ldx #213
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5966)
	; 16 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 5967)
	; 14 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 7
	delay_n 12
	inx
	Jne *-11
.elseif(n = 5974)
	; 15 bytes
	ldx #181
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5975)
	; 13 bytes
	ldx #206
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.endif
.else
; 5977 to 6015
.if(n = 5977)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif((n > 5977 && n < 5981) || n = 5982 || n = 5983 || n = 5986 || n = 5988 || n = 5997 || n = 6002 || n = 6003 || n = 6006 || (n > 6007 && n < 6012))
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5981 || n = 6007)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 5984 || n = 6015)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 5985)
	; 15 bytes
	ldx #187
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5987 || n = 5998)
	; 15..16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 5989 || n = 5990 || n = 6012 || n = 6013)
	; 16 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 5991)
	; 17 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 5992)
	; 17 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 5993)
	; 15 bytes
	ldx #214
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 5994)
	; 17 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 5995)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 5996)
	; 17 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 5999)
	; 13 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6000)
	; 13 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6001)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6004)
	; 13 bytes
	ldx #207
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6005)
	; 13 bytes
	ldx #232 ;hides 'inx'
	nop
	bit $24
	delay_n 12
	dex
	dex
	Jne *-10
.elseif(n = 6014)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 6016) && (n < 6055))
; 6016 to 6054
.if(n = 6016 || n = 6045)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6017 || n = 6019 || n = 6020 || n = 6027 || n = 6028 || n = 6030 || n = 6032 || n = 6035 || (n > 6036 && n < 6040) || n = 6042 || n = 6050 || n = 6053)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6018 || n = 6029 || n = 6034 || n = 6036 || n = 6052 || n = 6054)
	; 15..16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6021)
	; 15 bytes
	ldx #215
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6022 || n = 6049)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6023)
	; 13 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6024)
	; 13 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6025)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6026 || n = 6051)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6031)
	; 13 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6033)
	; 13 bytes
	ldx #208
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6040)
	; 15 bytes
	ldx #183
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6041)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 6043)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6044)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6046)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6047)
	; 13 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6048)
	; 13 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	delay_n 12
	inx
	Jne *-10
.endif
.else
; 6055 to 6093
.if((n > 6054 && n < 6059) || n = 6063 || n = 6064 || (n > 6065 && n < 6069) || n = 6074 || n = 6075 || n = 6078 || n = 6080 || n = 6082 || n = 6087 || n = 6089 || n = 6090 || n = 6093)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6059 || n = 6085)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 6060)
	; 17 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 6061)
	; 15 bytes
	ldx #202
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 6062)
	; 13 bytes
	ldx #209
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6065 || n = 6079 || n = 6086 || n = 6088)
	; 15..16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6069)
	; 17 bytes
	ldx #164
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6070)
	; 17 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 6071)
	; 13 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6072)
	; 13 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6073)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6076)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6077)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6081)
	; 15 bytes
	ldx #190
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6083)
	; 13 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	delay_n 12
	dex
	Jne *-10
.elseif(n = 6084)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6091)
	; 13 bytes
	ldx #210
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6092)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.endif
.endif
.endif
.else
.if((n >= 6094) && (n < 6172))
; 6094 to 6171
.if(n = 6094 || n = 6100 || n = 6104 || n = 6106 || n = 6114 || n = 6116 || n = 6129 || n = 6133 || n = 6140 || n = 6142 || n = 6148 || n = 6152 || n = 6154 || n = 6166)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6095)
	; 13 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6096)
	; 13 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6097 || n = 6121)
	; 13 bytes
	ldx #(n-1)/24
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6098 || n = 6099 || n = 6102 || n = 6105 || n = 6107 || n = 6109 || n = 6110 || n = 6112 || n = 6115 || n = 6117 || (n > 6121 && n < 6126) || n = 6127 || n = 6128 || n = 6131 || n = 6132 || (n > 6133 && n < 6137) || n = 6141 || n = 6146 || n = 6147 || n = 6150 || n = 6153 || n = 6155 || n = 6156 || n = 6158 || n = 6159 || n = 6161 || n = 6162 || n = 6164 || n = 6165 || n = 6167)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6101 || n = 6126 || n = 6151)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6103 || n = 6130 || n = 6157)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6108 || n = 6139 || n = 6170)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6111 || n = 6137 || n = 6163)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 6113)
	; 15 bytes
	ldx #191
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6118)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6119)
	; 13 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6120)
	; 13 bytes
	ldx #211
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6138 || n = 6169)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6143)
	; 13 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 12
	dex
	Jne *-10
.elseif(n = 6144)
	; 13 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6145)
	; 13 bytes
	ldx #0
	delay_n 7
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6149)
	; 13 bytes
	ldx #212
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6160)
	; 14 bytes
	ldx #36 ;hides 'bit zp'
	nop
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-11
.elseif(n = 6168)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6171)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.endif
.else
; 6172 to 6250
.if(n = 6172)
	; 15 bytes
	ldx #187
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6173 || n = 6179 || n = 6181 || n = 6187 || n = 6192 || n = 6202 || n = 6204 || n = 6210 || n = 6214 || n = 6218 || n = 6220 || n = 6229 || n = 6235 || n = 6239)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6174)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 6175 || n = 6197 || n = 6200)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 6176 || n = 6201 || n = 6226)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6177)
	; 15 bytes
	ldx #193
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6178)
	; 13 bytes
	ldx #213
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6180 || n = 6182 || n = 6183 || n = 6185 || n = 6186 || n = 6190 || n = 6191 || n = 6193 || n = 6198 || n = 6203 || n = 6205 || n = 6206 || n = 6208 || n = 6209 || n = 6212 || n = 6213 || n = 6216 || n = 6219 || n = 6227 || n = 6228 || n = 6230 || n = 6231 || n = 6233 || n = 6234 || n = 6237 || n = 6240 || (n > 6241 && n < 6245) || n = 6247)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6184 || n = 6211 || n = 6238)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6188)
	; 14 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-11
.elseif(n = 6189 || n = 6215 || n = 6241)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 6194 || n = 6221 || n = 6246 || n = 6248)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6195 || n = 6222)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6196)
	; 16 bytes
	ldx #177
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 6199)
	; 13 bytes
	ldx #56 ;hides 'sec'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-10
.elseif(n = 6207)
	; 13 bytes
	ldx #214
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6217)
	; 15 bytes
	ldx #222
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6223)
	; 16 bytes
	ldx #183
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 6224)
	; 13 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6225)
	; 13 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6232)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6236)
	; 13 bytes
	ldx #215
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6245)
	; 15 bytes
	ldx #223
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6249)
	; 13 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 4
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6250)
	; 13 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 4
	delay_n 12
	inx
	Jne *-10
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
.if((n >= 6251) && (n < 6290))
; 6251 to 6289
.if(n = 6251 || n = 6276)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6252 || n = 6253 || (n > 6254 && n < 6259) || n = 6267 || n = 6269 || n = 6270 || n = 6272 || n = 6277 || n = 6278 || (n > 6279 && n < 6284))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6254 || n = 6266 || n = 6268 || n = 6279)
	; 15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6259 || n = 6284)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6260 || n = 6261 || n = 6285 || n = 6286)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6262 || n = 6287)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6263)
	; 13 bytes
	ldx #24 ;hides 'clc'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-10
.elseif(n = 6264)
	; 15 bytes
	ldx #232 ;hides 'inx'
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	inx
	Jne *-12
.elseif(n = 6265)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6271)
	; 15 bytes
	ldx #190
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6273)
	; 15 bytes
	ldx #224
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6274)
	; 13 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 4
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6275)
	; 13 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 4
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6288)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6289)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.endif
.else
; 6290 to 6328
.if(n = 6290 || n = 6296 || n = 6298 || n = 6302 || n = 6303 || (n > 6304 && n < 6309) || n = 6321 || n = 6327 || n = 6328)
	; 14..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 6291)
	; 16 bytes
	ldx #185
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 6292 || n = 6319)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6293)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 6294)
	; 13 bytes
	ldx #217
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6295 || n = 6297 || n = 6304 || n = 6320 || n = 6322)
	; 15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6299)
	; 13 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 4
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6300)
	; 13 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 4
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6301 || n = 6326)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6309)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6310 || n = 6311)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6312)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6313)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6314)
	; 18 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 6315)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6316)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 6317)
	; 13 bytes
	ldx #234 ;hides 'nop'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6318)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6323)
	; 13 bytes
	ldx #218
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6324)
	; 13 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 4
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6325)
	; 13 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 4
	delay_n 12
	inx
	Jne *-10
.endif
.endif
.else
.if((n >= 6329) && (n < 6368))
; 6329 to 6367
.if(n = 6329 || n = 6340 || n = 6355 || n = 6359)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6330 || n = 6332 || n = 6333 || n = 6339 || n = 6341 || n = 6347 || n = 6348 || n = 6353 || n = 6354 || n = 6358 || n = 6360 || n = 6363)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6331)
	; 15 bytes
	ldx #211
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 6334)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6335 || n = 6338 || n = 6364)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6336)
	; 16 bytes
	ldx #181
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 6337)
	; 15 bytes
	ldx #192
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6342 || n = 6367)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6343)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 6344)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6345)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 6346)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6349)
	; 13 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6350)
	; 13 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6351)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6352)
	; 13 bytes
	ldx #219
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6356)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6357)
	; 15 bytes
	ldx #227
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6361)
	; 15 bytes
	ldx #212
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 6362)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6365)
	; 17 bytes
	ldx #172
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6366)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.endif
.else
; 6368 to 6406
.if(n = 6368)
	; 18 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 6369)
	; 15 bytes
	ldx #199
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6370)
	; 15 bytes
	ldx #193
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6371 || n = 6397)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 6372 || n = 6377 || n = 6378 || n = 6380 || n = 6382 || n = 6383 || n = 6385 || n = 6386 || n = 6389 || n = 6392 || n = 6393 || n = 6395 || n = 6402 || n = 6403 || n = 6405 || n = 6406)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6373 || n = 6400)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6374)
	; 13 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6375)
	; 13 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6376)
	; 13 bytes
	ldx #(n-1)/25
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6379 || n = 6384 || n = 6390 || n = 6394 || n = 6404)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6381)
	; 13 bytes
	ldx #220
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6387)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6388)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6391)
	; 15 bytes
	ldx #213
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 6396)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 6398)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 6399)
	; 13 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	delay_n 12
	dex
	Jne *-10
.elseif(n = 6401)
	; 13 bytes
	ldx #0
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-9
.endif
.endif
.endif
.else
.if((n >= 6407) && (n < 6485))
; 6407 to 6484
.if(n = 6407 || n = 6408 || n = 6411 || n = 6412 || (n > 6413 && n < 6418) || n = 6420 || n = 6422 || n = 6424 || n = 6425 || n = 6428 || n = 6429 || n = 6431 || n = 6432 || n = 6434 || n = 6435 || n = 6438 || n = 6440 || n = 6441 || (n > 6442 && n < 6447) || n = 6451 || n = 6453 || n = 6455 || n = 6456 || (n > 6457 && n < 6461) || n = 6463 || n = 6470 || n = 6472 || n = 6476 || n = 6477 || n = 6479 || n = 6482 || n = 6483)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6409)
	; 15 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 9
	delay_n 12
	inx
	Jne *-12
.elseif(n = 6410)
	; 13 bytes
	ldx #221
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6413 || n = 6426 || n = 6430 || n = 6442 || n = 6452 || n = 6457 || n = 6471 || n = 6478 || n = 6484)
	; 15..16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6418 || n = 6449 || n = 6480)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6419)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6421)
	; 15 bytes
	ldx #214
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 6423 || n = 6475)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 6427 || n = 6454 || n = 6481)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6433)
	; 15 bytes
	ldx #201
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6436)
	; 15 bytes
	ldx #195
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6437 || n = 6464)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6439)
	; 13 bytes
	ldx #222
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6447 || n = 6448 || n = 6450)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 6461)
	; 16 bytes
	ldx #190
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 6462)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6465)
	; 15 bytes
	ldx #202
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6466)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6467)
	; 14 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	delay_n 12
	dex
	dex
	Jne *-11
.elseif(n = 6468)
	; 13 bytes
	ldx #223
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6469)
	; 15 bytes
	ldx #196
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6473)
	; 14 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	bit $24
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6474)
	; 14 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	nop
	bit $24
	delay_n 12
	inx
	Jne *-11
.endif
.else
.if((n >= 6485) && (n < 6524))
; 6485 to 6523
.if((n > 6484 && n < 6489) || n = 6499 || (n > 6502 && n < 6507) || n = 6510 || n = 6512 || n = 6513 || (n > 6514 && n < 6518))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6489)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6490)
	; 18 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6491 || n = 6507 || n = 6518)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6492 || n = 6519)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6493 || n = 6509 || n = 6520)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6494 || n = 6521 || n = 6522)
	; 18 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 6495)
	; 14 bytes
	ldx #24 ;hides 'clc'
	delay_n 9
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6496)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6497)
	; 13 bytes
	ldx #224
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6498 || n = 6514)
	; 16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6500)
	; 14 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	bit $24
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6501)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 6502)
	; 15 bytes
	ldx #197
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6508)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6511)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6523)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.endif
.else
; 6524 to 6562
.if(n = 6524 || n = 6534)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 6525)
	; 14 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	bit $24
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6526)
	; 13 bytes
	ldx #225
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6527 || n = 6553)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 6528 || (n > 6529 && n < 6534) || n = 6537 || n = 6539 || n = 6540 || n = 6543 || n = 6544 || n = 6546 || n = 6548 || n = 6554 || n = 6557 || n = 6559 || n = 6560)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6529 || n = 6538 || n = 6545 || n = 6556 || n = 6558)
	; 15..16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6535 || n = 6562)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6536)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6541)
	; 15 bytes
	ldx #218
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 6542)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6547)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6549)
	; 18 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 6550)
	; 17 bytes
	ldx #177
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6551)
	; 14 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	nop
	bit $24
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6552)
	; 14 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	bit $24
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6555)
	; 13 bytes
	ldx #226
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6561)
	; 15 bytes
	ldx #205
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.endif
.endif
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
.if((n >= 6563) && (n < 6602))
; 6563 to 6601
.if(n = 6563)
	; 16 bytes
	ldx #193
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 6564 || n = 6566 || n = 6567 || n = 6569 || n = 6572 || n = 6574 || n = 6575 || n = 6580 || n = 6583 || n = 6585 || n = 6586 || n = 6588 || n = 6590 || n = 6591 || (n > 6592 && n < 6596))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6565 || n = 6576 || n = 6582 || n = 6587 || n = 6592)
	; 15..16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6568)
	; 15 bytes
	ldx #199
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6570)
	; 14 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 10
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6571)
	; 15 bytes
	ldx #219
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 6573)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6577)
	; 14 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	bit $24
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6578)
	; 14 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	nop
	bit $24
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6579)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 6581)
	; 15 bytes
	ldx #235
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6584)
	; 13 bytes
	ldx #227
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6589)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6596)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6597)
	; 16 bytes
	ldx #194
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 6598)
	; 18 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6599)
	; 14 bytes
	ldx #56 ;hides 'sec'
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-11
.elseif(n = 6600)
	; 14 bytes
	ldx #36 ;hides 'bit zp'
	nop
	delay_n 10
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6601)
	; 15 bytes
	ldx #200
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.endif
.else
; 6602 to 6640
.if(n = 6602 || (n > 6605 && n < 6609) || n = 6610 || n = 6611 || n = 6615 || n = 6617 || n = 6618 || (n > 6619 && n < 6623) || n = 6627 || n = 6632 || n = 6633 || n = 6636 || n = 6637 || n = 6639 || n = 6640)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6603)
	; 14 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	bit $24
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6604 || n = 6635)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6605 || n = 6631)
	; 14 bytes
	ldx #(n-1)/26
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 6609)
	; 15 bytes
	ldx #236
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6612 || n = 6619 || n = 6638)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6613)
	; 13 bytes
	ldx #228
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6614)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 6616)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6623)
	; 16 bytes
	ldx #184 ;hides 'clv'
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6624)
	; 17 bytes
	ldx #179
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6625)
	; 15 bytes
	ldx #207
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6626)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6628)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6629)
	; 14 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	bit $24
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6630)
	; 14 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	bit $24
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6634)
	; 15 bytes
	ldx #201
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.endif
.endif
.else
.if((n >= 6641) && (n < 6680))
; 6641 to 6679
.if(n = 6641 || n = 6644 || n = 6645 || (n > 6646 && n < 6651) || n = 6658 || n = 6659 || n = 6662 || n = 6663 || n = 6668 || n = 6672 || n = 6673 || (n > 6674 && n < 6679))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6642)
	; 13 bytes
	ldx #229
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6643 || n = 6670)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6646 || n = 6660 || n = 6664 || n = 6669 || n = 6674)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6651)
	; 16 bytes
	ldx #190
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 6652 || n = 6653)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6654)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6655)
	; 14 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	nop
	bit $24
	delay_n 12
	dex
	Jne *-11
.elseif(n = 6656)
	; 14 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	bit $24
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6657)
	; 14 bytes
	ldx #0
	delay_n 9
	delay_n 12
	dex
	Jne *-10
.elseif(n = 6661)
	; 15 bytes
	ldx #222
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 6665)
	; 15 bytes
	ldx #238
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6666)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6667)
	; 15 bytes
	ldx #202
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6671)
	; 13 bytes
	ldx #230
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6679)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
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
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6685)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6686)
	; 16 bytes
	ldx #191
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 6687 || n = 6714)
	; 18 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 6688 || n = 6695 || n = 6699 || n = 6701 || n = 6702 || (n > 6703 && n < 6707))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6689)
	; 15 bytes
	ldx #209
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6690 || n = 6717)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 6691)
	; 15 bytes
	ldx #223
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 6692 || n = 6694 || n = 6696 || n = 6703)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6693)
	; 15 bytes
	ldx #239
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6697)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6698)
	; 14 bytes
	ldx #232 ;hides 'inx'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	dex
	Jne *-11
.elseif(n = 6700)
	; 13 bytes
	ldx #231
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6708 || n = 6711)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6713)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 6715)
	; 18 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 6716)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
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
.if(n = 6719 || n = 6746)
	; 18 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 6720 || n = 6747)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 6721)
	; 15 bytes
	ldx #240
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6722)
	; 14 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 6
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6723)
	; 14 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 6
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6724 || n = 6751)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6725 || n = 6726 || n = 6731 || (n > 6732 && n < 6736) || n = 6752 || n = 6753 || (n > 6754 && n < 6758))
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 6727)
	; 13 bytes
	ldx #24 ;hides 'clc'
	delay_n 10
	delay_n 12
	inx
	Jne *-10
.elseif(n = 6728)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6729)
	; 13 bytes
	ldx #232
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6730 || n = 6732 || n = 6754)
	; 15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6736 || n = 6741)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6737 || n = 6739)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6738)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6740)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 6742)
	; 18 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 6743)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 6744)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 6745)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 6748)
	; 17 bytes
	ldx #173
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6749)
	; 14 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 6
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6750)
	; 14 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 6
	delay_n 12
	inx
	Jne *-11
.endif
.else
; 6758 to 6796
.if(n = 6758)
	; 13 bytes
	ldx #233
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6759 || n = 6790)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6760 || (n > 6761 && n < 6766) || n = 6768 || n = 6779 || n = 6780 || (n > 6781 && n < 6785) || n = 6789 || n = 6791 || n = 6792 || (n > 6793 && n < 6797))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6761 || n = 6769 || n = 6781 || n = 6788 || n = 6793)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6766)
	; 15 bytes
	ldx #205
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6767)
	; 16 bytes
	ldx #199
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
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
	ldx #183
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6773)
	; 18 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 6774)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 6775)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 6776)
	; 14 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 6
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6777)
	; 14 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 6
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6778)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6785)
	; 13 bytes
	ldx #234 ;hides 'nop'
	delay_n 10
	delay_n 12
	dex
	Jne *-10
.elseif(n = 6786)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6787)
	; 13 bytes
	ldx #234
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.endif
.endif
.else
.if((n >= 6797) && (n < 6836))
; 6797 to 6835
.if(n = 6797 || n = 6815)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6798 || n = 6827)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6799)
	; 15 bytes
	ldx #206
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6800)
	; 18 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 6801 || n = 6806 || n = 6807 || n = 6809 || n = 6810 || n = 6812 || n = 6813 || n = 6818 || n = 6820 || n = 6822 || n = 6823 || n = 6825 || n = 6834)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6802 || n = 6808 || n = 6814 || n = 6819 || n = 6824 || n = 6835)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6803)
	; 14 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 6
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6804)
	; 14 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 6
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6805 || n = 6832)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6811)
	; 15 bytes
	ldx #227
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 6816)
	; 13 bytes
	ldx #235
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6817)
	; 15 bytes
	ldx #213
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6821)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6826)
	; 16 bytes
	ldx #195
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 6828)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6829)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 6830)
	; 14 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 6
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6831)
	; 14 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 6
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6833)
	; 15 bytes
	ldx #244
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.endif
.else
; 6836 to 6875
.if((n > 6835 && n < 6840) || n = 6843 || n = 6847 || (n > 6848 && n < 6852) || n = 6854 || n = 6856 || n = 6860 || n = 6861 || n = 6863 || n = 6864 || n = 6866 || n = 6867 || n = 6873)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6840)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6841)
	; 15 bytes
	ldx #228
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 6842)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6844)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6845)
	; 13 bytes
	ldx #236
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6846)
	; 17 bytes
	ldx #185
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6848 || n = 6855 || n = 6862 || n = 6868)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6852)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6853 || n = 6875)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 6857)
	; 14 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 6
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6858)
	; 14 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 6
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6859)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6865)
	; 15 bytes
	ldx #208
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6869)
	; 16 bytes
	ldx #202
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 6870)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6871)
	; 15 bytes
	ldx #229
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 6872)
	; 18 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 6874)
	; 13 bytes
	ldx #237
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.endif
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
.if(n = 6876 || (n > 6877 && n < 6881) || n = 6887 || n = 6888 || (n > 6889 && n < 6894) || n = 6900 || n = 6905 || (n > 6906 && n < 6911))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6877 || n = 6889 || n = 6906)
	; 15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6881)
	; 15 bytes
	ldx #215
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6882 || n = 6902)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 6883 || n = 6914)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6884)
	; 14 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 6
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6885)
	; 14 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 6
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6886)
	; 13 bytes
	ldx #(n-1)/27
	delay_n 10
	delay_n 12
	dex
	Jne *-9
.elseif(n = 6894)
	; 17 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6895)
	; 18 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 6896)
	; 16 bytes
	ldx #197
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 6897)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 6898)
	; 15 bytes
	ldx #209
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6899)
	; 18 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 6901)
	; 15 bytes
	ldx #230
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 6903)
	; 13 bytes
	ldx #238
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6904)
	; 17 bytes
	ldx #177
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6911)
	; 14 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 6
	delay_n 12
	dex
	Jne *-11
.elseif(n = 6912)
	; 14 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 6
	delay_n 12
	inx
	Jne *-11
.elseif(n = 6913)
	; 13 bytes
	ldx #0
	delay_n 10
	delay_n 12
	dex
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
	ldx $A6
.elseif(n = 6920)
	; 17 bytes
	ldx #187
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6921)
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
	; 15 bytes
	ldx #232 ;hides 'inx'
	delay_n 9
	delay_n 12
	dex
	dex
	Jne *-12
.elseif(n = 6930)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6931)
	; 15 bytes
	ldx #210
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6932)
	; 13 bytes
	ldx #239
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6940)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 6941)
	; 18 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 6943)
	; 17 bytes
	ldx #178
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6945)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6946)
	; 18 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 6952)
	; 18 bytes
	delay_n 66
	delay_n (n-66)
.endif
.endif
.else
.if((n >= 6954) && (n < 6993))
; 6954 to 6992
.if(n = 6954)
	; 18 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 6955)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 6956 || n = 6969 || n = 6983)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 6957)
	; 17 bytes
	ldx #188
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6958)
	; 18 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 6959)
	; 15 bytes
	ldx #24 ;hides 'clc'
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-12
.elseif(n = 6960 || n = 6987)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 6961)
	; 13 bytes
	ldx #240
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6962 || n = 6964 || n = 6979)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 6963 || (n > 6964 && n < 6969) || n = 6974 || n = 6975 || n = 6978 || n = 6980 || n = 6981 || n = 6992)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6970)
	; 18 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 6971)
	; 15 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 7
	delay_n 12
	inx
	Jne *-12
.elseif(n = 6972)
	; 15 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 7
	delay_n 12
	inx
	Jne *-12
.elseif(n = 6973)
	; 15 bytes
	ldx #249
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6976)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 6977)
	; 15 bytes
	ldx #218
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6982)
	; 17 bytes
	ldx #179
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6984)
	; 18 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 6985)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 6986)
	; 18 bytes
	delay_n 181
	delay_n (n-181)
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
	ldx #241
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 6991)
	; 15 bytes
	ldx #233
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.endif
.else
; 6993 to 7031
.if(n = 6993 || n = 7004 || n = 7022)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n > 6993 && n < 6997) || n = 7002 || n = 7003 || (n > 7008 && n < 7015) || n = 7021 || (n > 7022 && n < 7027) || n = 7031)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6997)
	; 15 bytes
	ldx #212
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 6998)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 6999)
	; 15 bytes
	ldx #56 ;hides 'sec'
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7000)
	; 15 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 7
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7001)
	; 15 bytes
	ldx #250
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7005)
	; 16 bytes
	ldx #206
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7006)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7007)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7008)
	; 14 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-11
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
	ldx #242
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 7020)
	; 18 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 7027)
	; 15 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 7
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7028)
	; 15 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 7
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7029)
	; 15 bytes
	ldx #251
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7030)
	; 15 bytes
	ldx #213
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.endif
.endif
.endif
.else
.if((n >= 7032) && (n < 7110))
.if((n >= 7032) && (n < 7071))
; 7032 to 7070
.if(n = 7032 || n = 7034 || (n > 7041 && n < 7047) || n = 7050 || (n > 7051 && n < 7055) || n = 7058 || n = 7059 || n = 7061 || n = 7065 || n = 7067 || n = 7070)
	; 14..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7033 || n = 7051 || n = 7066)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7035 || n = 7062)
	; 18 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 7036)
	; 16 bytes
	ldx #201
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7037)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 7038 || n = 7069)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7039)
	; 15 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 10
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7040)
	; 14 bytes
	ldx #36 ;hides 'bit zp'
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-11
.elseif(n = 7041)
	; 15 bytes
	ldx #220
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7047)
	; 18 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 7048)
	; 13 bytes
	ldx #243
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 7049)
	; 18 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 7055)
	; 15 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 7
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7056)
	; 15 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 7
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7057)
	; 15 bytes
	ldx #252
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7060)
	; 17 bytes
	ldx #181
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7063)
	; 15 bytes
	ldx #214
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7064)
	; 18 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7068)
	; 17 bytes
	ldx #191
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.endif
.else
; 7071 to 7109
.if(n = 7071 || (n > 7073 && n < 7077) || n = 7078 || n = 7079 || n = 7081 || n = 7082 || n = 7086 || n = 7087 || n = 7098 || n = 7102 || n = 7104 || n = 7108)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7072)
	; 14 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-11
.elseif(n = 7073)
	; 15 bytes
	ldx #221
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7077)
	; 13 bytes
	ldx #244
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 7080 || n = 7103 || n = 7109)
	; 15..16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7083)
	; 15 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 7
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7084)
	; 15 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 7
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7085)
	; 15 bytes
	ldx #253
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
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
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7094)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7095)
	; 18 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 7096)
	; 15 bytes
	ldx #215
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7097)
	; 18 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 7099)
	; 17 bytes
	ldx #182
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7100)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7101)
	; 18 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7105)
	; 15 bytes
	ldx #222
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7106)
	; 13 bytes
	ldx #245
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 7107)
	; 16 bytes
	ldx #209
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.endif
.endif
.else
.if((n >= 7110) && (n < 7149))
; 7110 to 7148
.if(n = 7110 || n = 7114 || n = 7115 || n = 7133 || n = 7136 || n = 7137 || n = 7139 || n = 7142 || n = 7143)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7111)
	; 15 bytes
	ldx #237
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 7112)
	; 15 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 7
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7113)
	; 15 bytes
	ldx #254
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
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
	; 18 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 7127)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 7128)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7129)
	; 15 bytes
	ldx #216
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7130)
	; 18 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7131)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7132 || n = 7134 || n = 7138 || n = 7144)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7135)
	; 13 bytes
	ldx #246
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 7140)
	; 15 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 7
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7141)
	; 15 bytes
	ldx #255
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
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
	; 18 bytes
	delay_n 136
	delay_n (n-136)
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
	; 14 bytes
	ldx #232 ;hides 'inx'
	delay_n 10
	delay_n 12
	dex
	dex
	Jne *-11
.elseif(n = 7161)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7162)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7163 || n = 7165 || n = 7167)
	; 15..16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7164)
	; 13 bytes
	ldx #247
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 7166 || n = 7168 || n = 7170 || n = 7173 || n = 7178 || n = 7181)
	; 14..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7169)
	; 15 bytes
	ldx #0
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7171)
	; 15 bytes
	ldx #239
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 7172)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 7174)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7175)
	; 16 bytes
	ldx #211
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7176)
	; 16 bytes
	ldx #205
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7177)
	; 17 bytes
	ldx #184
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7179)
	; 17 bytes
	ldx #194
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.endif
.endif
.endif
.endif
.else
.if((n >= 7188) && (n < 7344))
.if((n >= 7188) && (n < 7266))
.if((n >= 7188) && (n < 7227))
; 7188 to 7226
.if(n = 7188)
	; 18 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7189)
	; 18 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7190 || n = 7219)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7191)
	; 13 bytes
	ldx #24 ;hides 'clc'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-10
.elseif(n = 7192)
	; 18 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7193)
	; 13 bytes
	ldx #248
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 7194 || n = 7196 || n = 7225)
	; 15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7195 || (n > 7196 && n < 7201) || n = 7203 || n = 7213 || n = 7218 || n = 7223 || n = 7224 || n = 7226)
	; 14..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7201)
	; 15 bytes
	ldx #240
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
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
.elseif(n = 7207)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 7208)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 7209)
	; 16 bytes
	ldx #212
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7210)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7211)
	; 16 bytes
	ldx #206
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7212)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
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
	ldx #195
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7217)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7220)
	; 15 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7221)
	; 15 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7222)
	; 13 bytes
	ldx #249
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.endif
.else
; 7227 to 7265
.if(n = 7227 || n = 7229 || n = 7230 || n = 7235 || n = 7237 || n = 7245 || n = 7248 || n = 7252 || n = 7257 || n = 7259 || n = 7263)
	; 15..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7228)
	; 15 bytes
	ldx #219
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7231)
	; 15 bytes
	ldx #241
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 7232)
	; 17 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7233)
	; 15 bytes
	ldx #226
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7234)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7236 || n = 7254 || n = 7256 || n = 7258 || n = 7264)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7238)
	; 18 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7239)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7240)
	; 18 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 7241)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7242)
	; 18 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 7243)
	; 16 bytes
	ldx #213
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7244)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7246)
	; 16 bytes
	ldx #207
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7247)
	; 18 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7249)
	; 15 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7250)
	; 15 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7251)
	; 13 bytes
	ldx #250
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 7253)
	; 13 bytes
	ldx #234 ;hides 'nop'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7255)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7260)
	; 16 bytes
	ldx #36 ;hides 'bit zp'
	nop
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-13
.elseif(n = 7261)
	; 15 bytes
	ldx #220
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7262)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7265)
	; 15 bytes
	ldx #227
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.endif
.endif
.else
.if((n >= 7266) && (n < 7305))
; 7266 to 7304
.if(n = 7266)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 7267 || n = 7269 || n = 7282 || n = 7284 || n = 7285 || n = 7287 || n = 7288 || n = 7290 || n = 7293 || n = 7295 || n = 7296 || n = 7298 || n = 7299 || n = 7301)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7268 || n = 7283 || n = 7289 || n = 7300)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7270)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7271)
	; 18 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 7272)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 7273)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
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
	; 16 bytes
	ldx #214
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7278)
	; 15 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7279)
	; 15 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7280)
	; 13 bytes
	ldx #251
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 7281)
	; 16 bytes
	ldx #208
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7286)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7291)
	; 15 bytes
	ldx #243
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 7292)
	; 17 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7294)
	; 15 bytes
	ldx #221
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7297)
	; 15 bytes
	ldx #228
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7302)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
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
.elseif(n = 7307)
	; 15 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7308)
	; 15 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7309)
	; 13 bytes
	ldx #252
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 7310 || n = 7311 || (n > 7312 && n < 7316) || n = 7318 || n = 7319 || n = 7322 || n = 7323 || n = 7325 || n = 7331 || n = 7339 || n = 7340 || n = 7342 || n = 7343)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7312 || n = 7320 || n = 7324 || n = 7330 || n = 7332 || n = 7341)
	; 15..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7316)
	; 16 bytes
	ldx #209
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7317)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7321)
	; 15 bytes
	ldx #244
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 7326)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7327)
	; 15 bytes
	ldx #222
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7328)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7329)
	; 15 bytes
	ldx #229
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7333)
	; 17 bytes
	ldx #188
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7336)
	; 15 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7337)
	; 15 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7338)
	; 13 bytes
	ldx #253
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.endif
.endif
.endif
.else
.if((n >= 7344) && (n < 7422))
.if((n >= 7344) && (n < 7383))
; 7344 to 7382
.if(n = 7344 || n = 7347 || n = 7350 || (n > 7351 && n < 7356) || n = 7362 || n = 7363 || n = 7368 || n = 7369 || (n > 7370 && n < 7375) || n = 7381)
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7345)
	; 16 bytes
	ldx #216
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7346 || n = 7375)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 7348 || n = 7379)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7349 || n = 7378)
	; 17 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7351)
	; 15 bytes
	ldx #245
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 7356)
	; 18 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 7357)
	; 18 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 7358)
	; 18 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 7359)
	; 18 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 7360)
	; 15 bytes
	ldx #223
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7361)
	; 15 bytes
	ldx #230
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7364)
	; 17 bytes
	ldx #199
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7365)
	; 15 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7366)
	; 15 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7367)
	; 13 bytes
	ldx #254
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 7370 || n = 7382)
	; 15..16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7376)
	; 18 bytes
	delay_n 96
	delay_n (n-96)
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
.if((n > 7382 && n < 7386) || n = 7388 || n = 7397 || n = 7398 || (n > 7399 && n < 7404) || n = 7412 || (n > 7413 && n < 7417))
	; 14..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7386)
	; 16 bytes
	ldx #211
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7387)
	; 18 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 7389 || n = 7418)
	; 18 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7390 || n = 7419)
	; 18 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7391)
	; 16 bytes
	ldx #232 ;hides 'inx'
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	dex
	Jne *-13
.elseif(n = 7392)
	; 18 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7393)
	; 15 bytes
	ldx #224
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7394)
	; 15 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7395)
	; 15 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7396)
	; 13 bytes
	ldx #255
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 7399 || n = 7413)
	; 15..16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7404)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
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
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7411)
	; 15 bytes
	ldx #247
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 7417)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7420)
	; 18 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7421)
	; 16 bytes
	ldx #212
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.endif
.endif
.else
.if((n >= 7422) && (n < 7461))
.if((n >= 7422) && (n < 7441))
; 7422 to 7440
.if(n = 7422)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7423)
	; 15 bytes
	ldx #24 ;hides 'clc'
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7424)
	; 15 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7425)
	; 13 bytes
	ldx #0
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-9
.elseif(n = 7426)
	; 15 bytes
	ldx #225
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7427 || (n > 7428 && n < 7433) || n = 7440)
	; 14..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7428)
	; 15 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7433)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
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
	ldx #201
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
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
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7442)
	; 18 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7443 || n = 7445 || n = 7448 || n = 7449 || n = 7458)
	; 15..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7444 || n = 7460)
	; 16..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7446)
	; 15 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7447)
	; 16 bytes
	ldx #219
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7450)
	; 17 bytes
	ldx #191
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
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
	ldx #213
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7457)
	; 15 bytes
	ldx #233
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7459)
	; 15 bytes
	ldx #226
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.endif
.endif
.else
; 7461 to 7500
.if(n = 7461 || n = 7473 || n = 7474 || (n > 7475 && n < 7479) || (n > 7481 && n < 7485) || n = 7493 || n = 7494 || n = 7496)
	; 15..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7462 || n = 7475 || n = 7490 || n = 7495)
	; 16..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7463)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.elseif(n = 7468 || n = 7497)
	; 18 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 7469)
	; 16 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 9
	delay_n 12
	inx
	Jne *-13
.elseif(n = 7470)
	; 16 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 9
	delay_n 12
	inx
	Jne *-13
.elseif(n = 7471)
	; 15 bytes
	ldx #249
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 7472)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7479)
	; 15 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7480)
	; 15 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7481)
	; 16 bytes
	ldx #220
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7485)
	; 18 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 7486)
	; 18 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 7487)
	; 15 bytes
	ldx #234 ;hides 'nop'
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-12
.elseif(n = 7488)
	; 18 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 7489)
	; 15 bytes
	ldx #234
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7491)
	; 16 bytes
	ldx #214
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7492)
	; 15 bytes
	ldx #227
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7498)
	; 18 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 7499)
	; 16 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 9
	delay_n 12
	inx
	Jne *-13
.elseif(n = 7500)
	; 16 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 9
	delay_n 12
	inx
	Jne *-13
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
	; 15 bytes
	ldx #250
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 7502 || n = 7505 || (n > 7506 && n < 7511) || (n > 7515 && n < 7519) || n = 7523 || n = 7527 || n = 7532 || n = 7533 || n = 7535 || n = 7536 || n = 7538 || n = 7539)
	; 15..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7503 || n = 7534)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7504)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7506 || n = 7524 || n = 7537)
	; 16..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7511)
	; 18 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7512)
	; 17 bytes
	ldx #203
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7513)
	; 18 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 7514)
	; 15 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7515)
	; 16 bytes
	ldx #221
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7519)
	; 18 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 7520)
	; 17 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 7521)
	; 15 bytes
	ldx #235
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7522)
	; 18 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 7525)
	; 15 bytes
	ldx #228
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7526)
	; 16 bytes
	ldx #215
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7528)
	; 17 bytes
	ldx #193
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7529)
	; 16 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 9
	delay_n 12
	inx
	Jne *-13
.elseif(n = 7530)
	; 16 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 9
	delay_n 12
	inx
	Jne *-13
.elseif(n = 7531)
	; 15 bytes
	ldx #251
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.endif
.else
; 7540 to 7578
.if(n = 7540 || n = 7550 || n = 7551 || n = 7554 || n = 7555 || n = 7557 || n = 7560 || n = 7562 || n = 7563 || n = 7567 || (n > 7568 && n < 7572) || n = 7574)
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
.elseif(n = 7543)
	; 17 bytes
	ldx #184 ;hides 'clv'
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 7544 || n = 7575)
	; 18 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7545)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 16 bytes
	ldx #222
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7552)
	; 18 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 7553)
	; 15 bytes
	ldx #236
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7556 || n = 7564 || n = 7568)
	; 16..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7558)
	; 15 bytes
	ldx #229
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7559)
	; 16 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 9
	delay_n 12
	inx
	Jne *-13
.elseif(n = 7561)
	; 15 bytes
	ldx #252
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 7565)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7566)
	; 18 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7572)
	; 18 bytes
	delay_n 176
	delay_n (n-176)
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
.if(n = 7579 || n = 7584 || n = 7587 || n = 7592 || n = 7593 || n = 7595 || n = 7598 || (n > 7599 && n < 7604) || n = 7608 || n = 7610 || n = 7613)
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
.elseif(n = 7583)
	; 16 bytes
	ldx #223
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7585)
	; 15 bytes
	ldx #237
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7586)
	; 17 bytes
	ldx #205
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7588 || n = 7594 || n = 7599 || n = 7609)
	; 16..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7589)
	; 16 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 9
	delay_n 12
	inx
	Jne *-13
.elseif(n = 7590)
	; 16 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 9
	delay_n 12
	inx
	Jne *-13
.elseif(n = 7591)
	; 15 bytes
	ldx #230
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7596)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7597)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
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
	ldx #195
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7607)
	; 18 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7612)
	; 18 bytes
	ldx #177
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 7614)
	; 19 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7615)
	; 19 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 7616)
	; 18 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 7617)
	; 15 bytes
	ldx #238
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.endif
.else
; 7618 to 7656
.if(n = 7618 || n = 7619 || n = 7623 || n = 7626 || n = 7628 || n = 7629 || (n > 7630 && n < 7635) || n = 7647 || n = 7653 || n = 7656)
	; 15..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 7620)
	; 16 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 9
	delay_n 12
	inx
	Jne *-13
.elseif(n = 7621)
	; 15 bytes
	ldx #254
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 7622)
	; 14 bytes
	ldx #232 ;hides 'inx'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-11
.elseif(n = 7624)
	; 15 bytes
	ldx #231
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7625 || n = 7630 || n = 7648 || n = 7652 || n = 7654)
	; 16..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7627)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7635)
	; 18 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7636)
	; 19 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 7637)
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
	ldx #196
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7646)
	; 19 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7649)
	; 15 bytes
	ldx #239
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7650)
	; 16 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 9
	delay_n 12
	inx
	Jne *-13
.elseif(n = 7651)
	; 15 bytes
	ldx #255
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 7655)
	; 14 bytes
	ldx #24 ;hides 'clc'
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-11
.endif
.endif
.endif
.else
.if((n >= 7657) && (n < 7735))
.if((n >= 7657) && (n < 7696))
; 7657 to 7695
.if(n = 7657)
	; 15 bytes
	ldx #232
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7658 || n = 7689)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7659 || n = 7660 || (n > 7661 && n < 7665) || n = 7667 || n = 7670 || n = 7671 || n = 7682 || n = 7683 || n = 7686 || n = 7687 || n = 7691 || (n > 7692 && n < 7696))
	; 15..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7661 || n = 7688 || n = 7692)
	; 16..18 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7665)
	; 16 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 7666)
	; 16 bytes
	ldx #219
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7668)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7669)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 19 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7677)
	; 18 bytes
	ldx #202
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 7678)
	; 19 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7679)
	; 16 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 9
	delay_n 12
	dex
	Jne *-13
.elseif(n = 7680)
	; 16 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 9
	delay_n 12
	inx
	Jne *-13
.elseif(n = 7681)
	; 15 bytes
	ldx #0
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-11
.elseif(n = 7684)
	; 17 bytes
	ldx #197
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7685)
	; 16 bytes
	ldx #226
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7690)
	; 15 bytes
	ldx #233
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.endif
.else
; 7696 to 7734
.if(n = 7696 || n = 7702 || n = 7703 || n = 7707 || n = 7711 || n = 7715 || n = 7717 || n = 7722 || n = 7724 || n = 7725 || (n > 7726 && n < 7730))
	; 15..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7697)
	; 17 bytes
	ldx #208
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7698)
	; 18 bytes
	ldx #179
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 7699)
	; 17 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-14
.elseif(n = 7700)
	; 16 bytes
	ldx #36 ;hides 'bit zp'
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 7701)
	; 16 bytes
	ldx #220
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7704 || n = 7712 || n = 7716 || n = 7726)
	; 17..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7705)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7706)
	; 19 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7708)
	; 19 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7709)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7710)
	; 19 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 7713)
	; 15 bytes
	ldx #241
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7714)
	; 19 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7718)
	; 15 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 10
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7719)
	; 15 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 10
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7720)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7721)
	; 14 bytes
	ldx #234 ;hides 'nop'
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7723)
	; 15 bytes
	ldx #234
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7730 || n = 7731)
	; 18 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7732 || n = 7733)
	; 18 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7734)
	; 17 bytes
	ldx #209
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.endif
.endif
.else
.if((n >= 7735) && (n < 7774))
; 7735 to 7773
.if(n = 7735)
	; 16 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 7736)
	; 16 bytes
	ldx #221
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7737 || n = 7738 || n = 7740 || n = 7743 || n = 7747 || n = 7752 || n = 7753 || n = 7755 || n = 7757 || n = 7758 || n = 7760 || n = 7764 || n = 7773)
	; 15..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7739 || n = 7748 || n = 7754 || n = 7759 || n = 7765)
	; 16..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7741)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7742)
	; 19 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 7744)
	; 19 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7745)
	; 15 bytes
	ldx #242
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7746)
	; 18 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 7749)
	; 15 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 10
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7750)
	; 15 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 10
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7751)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7756)
	; 15 bytes
	ldx #235
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7761)
	; 18 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7762)
	; 17 bytes
	ldx #199
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7763)
	; 18 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7766)
	; 19 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 7767)
	; 19 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7768)
	; 19 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7769)
	; 19 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7770)
	; 19 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 7771)
	; 16 bytes
	ldx #222
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7772)
	; 19 bytes
	delay_n 21
	delay_n (n-21)
.endif
.else
; 7774 to 7812
.if(n = 7774 || n = 7785 || n = 7792 || n = 7802 || n = 7804)
	; 16..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7775 || n = 7779 || n = 7783 || n = 7784 || n = 7786 || n = 7788 || n = 7791 || n = 7793 || n = 7795 || n = 7803 || n = 7805 || n = 7810)
	; 15..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 7776 || n = 7807)
	; 19 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7777)
	; 15 bytes
	ldx #243
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7778)
	; 19 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 7780)
	; 15 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 10
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7781)
	; 15 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 10
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7782)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7787)
	; 16 bytes
	ldx #229
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7789)
	; 15 bytes
	ldx #236
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 19 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7799)
	; 17 bytes
	ldx #56 ;hides 'sec'
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-14
.elseif(n = 7800)
	; 19 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7801)
	; 17 bytes
	ldx #200
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7806)
	; 16 bytes
	ldx #223
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7808)
	; 17 bytes
	ldx #211
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7809)
	; 15 bytes
	ldx #244
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7811)
	; 15 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 10
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7812)
	; 15 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 10
	delay_n 12
	inx
	Jne *-12
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
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7814 || n = 7815 || (n > 7816 && n < 7821) || n = 7823 || n = 7824 || n = 7826 || n = 7828 || n = 7834 || n = 7845 || n = 7846 || (n > 7847 && n < 7852))
	; 15..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7816 || n = 7825 || n = 7835 || n = 7847)
	; 16..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7821)
	; 16 bytes
	ldx #230
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7822)
	; 15 bytes
	ldx #237
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7827)
	; 18 bytes
	ldx #182
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 7829)
	; 18 bytes
	ldx #206
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 7830)
	; 19 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 7831)
	; 19 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7832)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7833)
	; 19 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7836)
	; 19 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 7837)
	; 19 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7838)
	; 19 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7839)
	; 19 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7840)
	; 17 bytes
	ldx #201
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7841)
	; 15 bytes
	ldx #245
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7842)
	; 15 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 10
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7843)
	; 15 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 10
	delay_n 12
	inx
	Jne *-12
.endif
.else
.if((n >= 7852) && (n < 7871))
; 7852 to 7870
.if(n = 7852)
	; 18 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7853)
	; 16 bytes
	ldx #232 ;hides 'inx'
	bit $24
	delay_n 10
	delay_n 12
	dex
	dex
	Jne *-13
.elseif(n = 7854)
	; 18 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7855)
	; 15 bytes
	ldx #238
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx $A6
.elseif(n = 7862)
	; 19 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7863)
	; 19 bytes
	delay_n 81
	delay_n (n-81)
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
	; 18 bytes
	ldx #207
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 7868)
	; 19 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 7869)
	; 19 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7870)
	; 18 bytes
	ldx #183
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 15 bytes
	ldx #246
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7874)
	; 15 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 10
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7875)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7876)
	; 16 bytes
	ldx #225
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7877 || (n > 7878 && n < 7882) || n = 7886 || n = 7890)
	; 15..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 7878)
	; 16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7882)
	; 17 bytes
	ldx #213
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7883)
	; 18 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7884)
	; 16 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 7885)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7887)
	; 15 bytes
	ldx #24 ;hides 'clc'
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 7888)
	; 15 bytes
	ldx #239
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7889)
	; 16 bytes
	ldx #232
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.endif
.endif
.endif
.else
.if((n >= 7891) && (n < 7930))
; 7891 to 7929
.if(n = 7891 || n = 7909 || n = 7924)
	; 16..17 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n > 7891 && n < 7895) || n = 7907 || n = 7908 || n = 7910 || n = 7912 || n = 7913 || n = 7915 || n = 7922 || n = 7923 || (n > 7924 && n < 7928))
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
	; 15 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 10
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7905)
	; 15 bytes
	ldx #247
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7906)
	; 14 bytes
	ldx #(n-1)/31
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7911)
	; 16 bytes
	ldx #226
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7914)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7917)
	; 18 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 7918)
	; 17 bytes
	ldx #203
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7919)
	; 16 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 7920)
	; 16 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 7921)
	; 15 bytes
	ldx #240
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.elseif(n = 7935)
	; 15 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 10
	delay_n 12
	dex
	Jne *-12
.elseif(n = 7936)
	; 15 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 10
	delay_n 12
	inx
	Jne *-12
.elseif(n = 7937)
	; 14 bytes
	ldx #0
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-10
.elseif(n = 7938 || n = 7939 || (n > 7940 && n < 7945) || n = 7948 || n = 7950 || n = 7956 || (n > 7957 && n < 7962))
	; 15..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7940)
	; 16 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7945)
	; 18 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7946)
	; 16 bytes
	ldx #227
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #241
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7955)
	; 15 bytes
	ldx #234 ;hides 'nop'
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7957)
	; 16 bytes
	ldx #234
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7962)
	; 19 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 7966)
	; 19 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 7967)
	; 17 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-14
.elseif(n = 7968)
	; 17 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-14
.endif
.endif
.endif
.else
.if((n >= 7969) && (n < 8047))
.if((n >= 7969) && (n < 8008))
; 7969 to 8007
.if(n = 7969)
	; 15 bytes
	ldx #249
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7970 || n = 7971 || (n > 7972 && n < 7976) || n = 7983 || n = 7985 || n = 7989 || n = 7992 || n = 7995 || n = 7997 || n = 7998 || n = 8002 || n = 8003 || (n > 8004 && n < 8008))
	; 16..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 7972 || n = 7984 || n = 7990 || n = 7994 || n = 8004)
	; 17..18 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 7976)
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
	ldx #228
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7982)
	; 19 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 7986)
	; 19 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 7987)
	; 15 bytes
	ldx #242
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 7988)
	; 19 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 7991)
	; 16 bytes
	ldx #235
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 7993)
	; 17 bytes
	ldx #216
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7996)
	; 17 bytes
	ldx #205
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 7999)
	; 17 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8000)
	; 17 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8001)
	; 15 bytes
	ldx #250
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
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
.elseif(n = 8014 || n = 8018 || n = 8022 || n = 8024 || n = 8026 || n = 8027 || n = 8034 || n = 8035 || n = 8038 || n = 8039 || n = 8044)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8015 || n = 8046)
	; 19 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 8016)
	; 16 bytes
	ldx #229
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8017)
	; 19 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 8019 || n = 8023 || n = 8028 || n = 8036 || n = 8040)
	; 17..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8020)
	; 15 bytes
	ldx #243
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8021)
	; 19 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 8025)
	; 16 bytes
	ldx #236
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8029)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8030)
	; 17 bytes
	ldx #217
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8031)
	; 17 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8032)
	; 17 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8033)
	; 15 bytes
	ldx #251
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8037)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8042)
	; 18 bytes
	ldx #187
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8045)
	; 19 bytes
	delay_n 124
	delay_n (n-124)
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
.elseif(n = 8050 || n = 8055 || n = 8057 || n = 8058 || n = 8060 || n = 8061 || n = 8066 || n = 8067 || (n > 8068 && n < 8072) || n = 8076 || n = 8080)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8051)
	; 16 bytes
	ldx #230
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8052 || n = 8083)
	; 19 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 8053)
	; 15 bytes
	ldx #244
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8054 || n = 8056 || n = 8062 || n = 8068 || n = 8077 || n = 8081)
	; 17..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8059)
	; 16 bytes
	ldx #237
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8063)
	; 17 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8064)
	; 17 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8065)
	; 15 bytes
	ldx #252
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
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
	ldx #207
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8075)
	; 19 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8078)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8079)
	; 19 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 8082)
	; 19 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 8084)
	; 15 bytes
	ldx #232 ;hides 'inx'
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-12
.elseif(n = 8085)
	; 18 bytes
	ldx #188
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.endif
.else
; 8086 to 8125
.if(n = 8086)
	; 15 bytes
	ldx #245
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8087 || n = 8089 || n = 8100 || n = 8107 || n = 8116 || n = 8122)
	; 17..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8088 || (n > 8089 && n < 8093) || n = 8095 || n = 8098 || n = 8099 || n = 8101 || n = 8102 || n = 8105 || n = 8106 || n = 8108 || n = 8110 || n = 8115 || n = 8117 || n = 8120 || n = 8124 || n = 8125)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8093)
	; 16 bytes
	ldx #238
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8094)
	; 19 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8096)
	; 17 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8097)
	; 15 bytes
	ldx #253
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8103)
	; 17 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8104)
	; 17 bytes
	ldx #219
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8109)
	; 19 bytes
	delay_n 12
	delay_n (n-12)
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
	ldx #208
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8114)
	; 20 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 8118)
	; 19 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 8119)
	; 15 bytes
	ldx #246
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8121)
	; 16 bytes
	ldx #232
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
; 8126 to 8164
.if(n = 8126 || n = 8131 || (n > 8132 && n < 8137) || n = 8142 || n = 8143 || n = 8145 || n = 8147 || n = 8154 || n = 8157 || n = 8158 || n = 8162 || n = 8163)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8127)
	; 16 bytes
	ldx #239
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8128)
	; 17 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8129)
	; 15 bytes
	ldx #254
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8130 || n = 8132 || n = 8144 || n = 8155 || n = 8164)
	; 17..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8137)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8138)
	; 20 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8139)
	; 17 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8140)
	; 17 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8141)
	; 17 bytes
	ldx #220
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8146)
	; 19 bytes
	ldx #181
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8148)
	; 20 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 8149)
	; 19 bytes
	ldx #194
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
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
	ldx #247
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8153)
	; 20 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 8156)
	; 16 bytes
	ldx #233
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8159)
	; 17 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8160)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8161)
	; 15 bytes
	ldx #255
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.endif
.else
; 8165 to 8203
.if((n > 8164 && n < 8169) || n = 8175 || n = 8179 || n = 8180 || (n > 8181 && n < 8185) || n = 8187 || n = 8190 || n = 8195 || n = 8197 || n = 8198 || n = 8203)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8169)
	; 19 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8170)
	; 20 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 8171)
	; 18 bytes
	ldx #190
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8172 || n = 8200)
	; 19 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8173)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8174 || n = 8176 || n = 8181 || n = 8188 || n = 8192 || n = 8194 || n = 8196 || n = 8202)
	; 17..20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8177)
	; 17 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8178)
	; 17 bytes
	ldx #221
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8185)
	; 15 bytes
	ldx #248
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8186)
	; 20 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 8189)
	; 15 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8191)
	; 16 bytes
	ldx #234
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8193)
	; 15 bytes
	ldx #0
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8199)
	; 17 bytes
	ldx #56 ;hides 'sec'
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8201)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.endif
.endif
.else
.if((n >= 8204) && (n < 8243))
; 8204 to 8242
.if(n = 8204 || n = 8212 || n = 8221)
	; 17..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8205 || n = 8206 || n = 8211 || n = 8213 || n = 8219 || n = 8220 || (n > 8221 && n < 8226) || n = 8228 || (n > 8230 && n < 8236))
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8207 || n = 8240)
	; 20 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 8208 || n = 8210)
	; 19 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 8209)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8214)
	; 18 bytes
	ldx #191
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8215)
	; 17 bytes
	ldx #222
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8216)
	; 15 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 8217)
	; 15 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 8218)
	; 15 bytes
	ldx #249
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8226)
	; 16 bytes
	ldx #235
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8227)
	; 19 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8229)
	; 16 bytes
	ldx #242
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8230)
	; 17 bytes
	ldx #211
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8236)
	; 19 bytes
	ldx #183
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8237)
	; 20 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8238)
	; 20 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 8239)
	; 20 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 8241)
	; 19 bytes
	ldx #206
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8242)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.endif
.else
; 8243 to 8281
.if(n = 8243 || n = 8276)
	; 19 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 8244 || n = 8246 || n = 8252 || n = 8253 || n = 8255 || n = 8256 || n = 8258 || n = 8259 || n = 8265 || n = 8267 || n = 8268 || n = 8271 || n = 8273 || n = 8274)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8245 || n = 8281)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8247)
	; 18 bytes
	ldx #217
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8248 || n = 8254 || n = 8264 || n = 8266 || n = 8272)
	; 17..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8249)
	; 15 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 8250)
	; 15 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 8251)
	; 15 bytes
	ldx #250
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8257)
	; 18 bytes
	ldx #192
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8260)
	; 19 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8261)
	; 16 bytes
	ldx #236
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8262)
	; 19 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8263)
	; 16 bytes
	ldx #243
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8269)
	; 17 bytes
	ldx #212
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8270)
	; 20 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8275)
	; 19 bytes
	ldx #197
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8277)
	; 20 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8278)
	; 20 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 8279)
	; 18 bytes
	ldx #184 ;hides 'clv'
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8280)
	; 20 bytes
	delay_n 161
	delay_n (n-161)
.endif
.endif
.endif
.else
.if((n >= 8282) && (n < 8360))
.if((n >= 8282) && (n < 8321))
; 8282 to 8320
.if(n = 8282)
	; 15 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 8283)
	; 15 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 8284)
	; 15 bytes
	ldx #251
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8285 || n = 8286 || n = 8288 || n = 8290 || n = 8291 || n = 8293 || n = 8298 || n = 8299 || (n > 8300 && n < 8304) || n = 8310 || n = 8312 || n = 8314 || n = 8318 || n = 8319)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8287 || n = 8292 || n = 8311 || n = 8320)
	; 17..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8289)
	; 17 bytes
	ldx #224
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
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
	ldx #237
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8297)
	; 16 bytes
	ldx #244
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8300)
	; 18 bytes
	ldx #193
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8304)
	; 20 bytes
	delay_n 86
	delay_n (n-86)
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
	ldx #213
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8309 || n = 8313)
	; 19 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 8315)
	; 15 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 8316)
	; 15 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 8317)
	; 15 bytes
	ldx #252
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.endif
.else
; 8321 to 8359
.if(n = 8321 || n = 8323 || n = 8328 || n = 8330 || n = 8332 || n = 8333 || (n > 8334 && n < 8338) || n = 8345 || n = 8352 || (n > 8353 && n < 8358))
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8322)
	; 17 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8324)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8325 || n = 8358)
	; 19 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8326)
	; 17 bytes
	ldx #225
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8327 || n = 8329 || n = 8334 || n = 8346 || n = 8353)
	; 17..20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8331)
	; 16 bytes
	ldx #238
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8338)
	; 20 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 8339)
	; 20 bytes
	delay_n 121
	delay_n (n-121)
.elseif((n > 8339 && n < 8343))
	; 19 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 8343)
	; 18 bytes
	ldx #194
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8344)
	; 20 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 8347)
	; 17 bytes
	ldx #214
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8348)
	; 15 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 8349)
	; 15 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 8350)
	; 15 bytes
	ldx #253
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8351)
	; 16 bytes
	ldx #24 ;hides 'clc'
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 8359)
	; 17 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.endif
.endif
.else
.if((n >= 8360) && (n < 8399))
; 8360 to 8398
.if(n = 8360)
	; 17 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 8
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8361)
	; 18 bytes
	ldx #220
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8362 || n = 8364 || n = 8367 || n = 8368 || n = 8370 || n = 8372 || n = 8384 || n = 8385 || n = 8387 || n = 8388 || n = 8390 || n = 8391)
	; 16..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 8363)
	; 17 bytes
	ldx #226
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8365)
	; 16 bytes
	ldx #246
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8366)
	; 16 bytes
	ldx #239
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8369 || n = 8392)
	; 18..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8371)
	; 19 bytes
	ldx #186
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif((n > 8372 && n < 8376))
	; 19 bytes
	delay_n 124
	delay_n (n-124)
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
.elseif(n = 8379)
	; 20 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 8380)
	; 20 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8381)
	; 15 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 8382)
	; 15 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 8383)
	; 15 bytes
	ldx #254
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8386)
	; 17 bytes
	ldx #215
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8389)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
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
	; 17 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.endif
.else
; 8399 to 8437
.if(n = 8399)
	; 16 bytes
	ldx #247
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8400)
	; 17 bytes
	ldx #227
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8401)
	; 16 bytes
	ldx #240
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8402 || n = 8403 || n = 8405 || n = 8407 || n = 8408 || n = 8410 || n = 8411 || n = 8417 || n = 8418 || (n > 8419 && n < 8423) || n = 8427 || n = 8430 || n = 8431 || n = 8435)
	; 16..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8404 || n = 8409 || n = 8419 || n = 8426 || n = 8428 || n = 8432)
	; 17..20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8406)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8412)
	; 19 bytes
	ldx #232 ;hides 'inx'
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	inx
	Jne *-16
.elseif(n = 8413)
	; 20 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 8414)
	; 15 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 8415)
	; 15 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 8416)
	; 15 bytes
	ldx #255
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8423)
	; 16 bytes
	ldx #234 ;hides 'nop'
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8424)
	; 19 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8425)
	; 17 bytes
	ldx #216
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8429)
	; 18 bytes
	ldx #196
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8433)
	; 16 bytes
	ldx #248
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8434)
	; 20 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 8436)
	; 16 bytes
	ldx #241
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8437)
	; 17 bytes
	ldx #228
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
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
.if((n > 8437 && n < 8443) || n = 8450 || n = 8451 || (n > 8452 && n < 8457) || n = 8463 || n = 8468 || n = 8469 || n = 8473 || n = 8475 || n = 8476)
	; 16..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 8443)
	; 19 bytes
	ldx #201
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8444)
	; 20 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 8445)
	; 20 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 8446 || n = 8458 || n = 8459)
	; 19..20 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8447)
	; 15 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8448)
	; 15 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-12
.elseif(n = 8449)
	; 15 bytes
	ldx #0
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-11
.elseif(n = 8452 || n = 8470)
	; 17..18 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8457)
	; 19 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8460)
	; 19 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8461)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8462)
	; 20 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 8464)
	; 17 bytes
	ldx #217
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8465)
	; 17 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-14
.elseif(n = 8466)
	; 17 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-14
.elseif(n = 8467)
	; 16 bytes
	ldx #249
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8471)
	; 16 bytes
	ldx #242
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8472)
	; 18 bytes
	ldx #197
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8474)
	; 17 bytes
	ldx #229
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.endif
.else
; 8477 to 8515
.if(n = 8477 || n = 8491 || n = 8504 || n = 8509 || n = 8514)
	; 18..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n > 8477 && n < 8481) || n = 8490 || n = 8492 || n = 8502 || n = 8505 || n = 8507 || n = 8508 || n = 8510 || n = 8512 || n = 8513)
	; 17..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8481)
	; 19 bytes
	ldx #212
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
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
	; 19 bytes
	ldx #202
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
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
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8489)
	; 20 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8493)
	; 19 bytes
	ldx #193
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
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
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8498)
	; 20 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 8499)
	; 17 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-14
.elseif(n = 8500)
	; 17 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-14
.elseif(n = 8501)
	; 16 bytes
	ldx #250
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8503)
	; 17 bytes
	ldx #218
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8506)
	; 16 bytes
	ldx #243
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8511)
	; 17 bytes
	ldx #230
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8515)
	; 18 bytes
	ldx #198
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.endif
.endif
.else
.if((n >= 8516) && (n < 8555))
; 8516 to 8554
.if(n = 8516 || n = 8517 || n = 8523 || n = 8531 || n = 8536 || n = 8537 || n = 8539 || (n > 8542 && n < 8546) || n = 8547 || n = 8550 || (n > 8551 && n < 8555))
	; 17..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8518 || n = 8532 || n = 8538 || n = 8549)
	; 18..20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8519)
	; 20 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 8520)
	; 20 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 8521)
	; 19 bytes
	ldx #213
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8522)
	; 20 bytes
	delay_n 106
	delay_n (n-106)
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
	; 19 bytes
	ldx #203
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8528)
	; 20 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 8529)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8530)
	; 20 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 8533)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8534)
	; 17 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-14
.elseif(n = 8535)
	; 16 bytes
	ldx #251
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8540)
	; 19 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 8541)
	; 16 bytes
	ldx #244
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8542)
	; 17 bytes
	ldx #219
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8546)
	; 16 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-13
.elseif(n = 8548)
	; 17 bytes
	ldx #231
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8551)
	; 18 bytes
	ldx #225
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.endif
.else
; 8555 to 8593
.if(n = 8555)
	; 20 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 8556 || n = 8593)
	; 20 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8557)
	; 20 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 8558)
	; 18 bytes
	ldx #199
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8559 || n = 8592)
	; 20 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 8560 || n = 8563 || (n > 8570 && n < 8576) || n = 8578 || n = 8580 || n = 8582 || n = 8587 || n = 8590 || n = 8591)
	; 17..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8561)
	; 19 bytes
	ldx #214
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
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
.elseif(n = 8566)
	; 20 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8567)
	; 17 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-14
.elseif(n = 8568)
	; 17 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-14
.elseif(n = 8569)
	; 16 bytes
	ldx #252
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8570)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8576)
	; 16 bytes
	ldx #245
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8577)
	; 20 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 8579 || n = 8584 || n = 8586 || n = 8588)
	; 18..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8581)
	; 17 bytes
	ldx #220
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8583)
	; 16 bytes
	ldx #24 ;hides 'clc'
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 8585)
	; 17 bytes
	ldx #232
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8589)
	; 18 bytes
	ldx #226
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.endif
.endif
.endif
.else
.if((n >= 8594) && (n < 8672))
.if((n >= 8594) && (n < 8633))
; 8594 to 8632
.if(n = 8594)
	; 20 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8595)
	; 20 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 8596)
	; 19 bytes
	ldx #191
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8597 || n = 8630)
	; 20 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 8598 || n = 8604 || n = 8608 || n = 8609 || n = 8613 || (n > 8614 && n < 8619) || n = 8621 || n = 8624 || n = 8626 || n = 8628 || n = 8629 || n = 8631)
	; 17..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8599)
	; 17 bytes
	ldx #56 ;hides 'sec'
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8600)
	; 20 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8601)
	; 17 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-14
.elseif(n = 8602)
	; 17 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-14
.elseif(n = 8603)
	; 16 bytes
	ldx #253
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8605)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8606 || n = 8614 || n = 8623 || n = 8625)
	; 18..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8607)
	; 18 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 8610)
	; 20 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 8611)
	; 16 bytes
	ldx #246
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8612)
	; 20 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 8619)
	; 18 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 8620)
	; 17 bytes
	ldx #221
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8622)
	; 17 bytes
	ldx #233
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8627)
	; 18 bytes
	ldx #227
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8632)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 8633 to 8671
.if(n = 8633 || n = 8634)
	; 20 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 8635)
	; 17 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-14
.elseif(n = 8636)
	; 17 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-14
.elseif(n = 8637)
	; 16 bytes
	ldx #254
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8638 || n = 8639 || n = 8642 || n = 8643 || n = 8645 || n = 8648 || n = 8650 || n = 8651 || n = 8654 || n = 8656 || n = 8658 || n = 8661 || n = 8663 || n = 8664 || n = 8666 || n = 8667)
	; 17..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8640 || n = 8649 || n = 8655 || n = 8660 || n = 8662)
	; 18..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8641)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8644)
	; 18 bytes
	ldx #201
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8646)
	; 16 bytes
	ldx #247
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8647)
	; 19 bytes
	ldx #184 ;hides 'clv'
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 8652)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8653)
	; 19 bytes
	ldx #206
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8657)
	; 16 bytes
	ldx #234 ;hides 'nop'
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8659)
	; 17 bytes
	ldx #234
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8665)
	; 18 bytes
	ldx #228
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8668)
	; 20 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8669)
	; 17 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-14
.elseif(n = 8670)
	; 17 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-14
.elseif(n = 8671)
	; 16 bytes
	ldx #255
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.endif
.endif
.else
.if((n >= 8672) && (n < 8711))
; 8672 to 8710
.if(n = 8672 || n = 8673 || n = 8675 || n = 8676 || n = 8678 || n = 8679 || n = 8682 || n = 8683 || n = 8685 || n = 8688 || n = 8689 || n = 8695 || n = 8697 || n = 8700 || n = 8702 || n = 8706 || n = 8707 || n = 8709 || n = 8710)
	; 17..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8674 || n = 8680 || n = 8684 || n = 8690 || n = 8699 || n = 8701 || n = 8708)
	; 18..20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8677)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8681)
	; 16 bytes
	ldx #248
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8686)
	; 19 bytes
	ldx #193
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8687)
	; 18 bytes
	ldx #202
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8691)
	; 20 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8692)
	; 20 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8693)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8694)
	; 21 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 8696)
	; 17 bytes
	ldx #235
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8698)
	; 17 bytes
	ldx #223
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8703)
	; 17 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 10
	delay_n 12
	dex
	Jne *-14
.elseif(n = 8704)
	; 17 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 10
	delay_n 12
	inx
	Jne *-14
.elseif(n = 8705)
	; 16 bytes
	ldx #0
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.endif
.else
; 8711 to 8750
.if(n = 8711 || n = 8712 || n = 8717 || n = 8718 || (n > 8719 && n < 8724) || n = 8732 || n = 8735 || n = 8736 || n = 8738 || n = 8739 || n = 8742 || n = 8743 || n = 8745)
	; 17..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 8713)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8714)
	; 16 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 8715)
	; 16 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 8716)
	; 16 bytes
	ldx #249
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8719 || n = 8740 || n = 8744)
	; 18..20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8724)
	; 20 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8725 || n = 8726 || n = 8747)
	; 20..21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8727 || n = 8748)
	; 20..21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8728)
	; 20 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8729)
	; 21 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 8730)
	; 18 bytes
	ldx #203
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8731)
	; 19 bytes
	ldx #194
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8733)
	; 17 bytes
	ldx #236
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8734)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8737)
	; 17 bytes
	ldx #224
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8741)
	; 18 bytes
	ldx #230
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8746)
	; 21 bytes
	ldx #165
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 8749)
	; 16 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 8750)
	; 16 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
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
	ldx #250
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8752 || n = 8753 || (n > 8754 && n < 8759) || n = 8762 || n = 8764 || n = 8765 || n = 8772 || n = 8774 || n = 8778 || (n > 8779 && n < 8784) || n = 8787 || n = 8788)
	; 17..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8754 || n = 8789)
	; 18 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8759)
	; 20 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8760)
	; 18 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-15
.elseif(n = 8761)
	; 19 bytes
	ldx #219
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8763)
	; 20 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8766)
	; 21 bytes
	delay_n 61
	delay_n (n-61)
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
	ldx #237
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8771)
	; 21 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 8773)
	; 18 bytes
	ldx #204
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8775)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8776)
	; 17 bytes
	ldx #225
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8777)
	; 17 bytes
	ldx #232 ;hides 'inx'
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-14
.elseif(n = 8779)
	; 18 bytes
	ldx #231
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8784)
	; 16 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 8785)
	; 16 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 8786)
	; 16 bytes
	ldx #251
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.endif
.else
; 8790 to 8828
.if((n > 8789 && n < 8794) || (n > 8801 && n < 8805) || n = 8809 || (n > 8810 && n < 8815) || n = 8817 || n = 8818 || n = 8822 || n = 8823 || (n > 8824 && n < 8829))
	; 17..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8794)
	; 20 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8795 || n = 8805)
	; 20 bytes
	delay_n 124
	delay_n (n-124)
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
	; 18 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 8800)
	; 18 bytes
	ldx #36 ;hides 'bit zp'
	nop
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-15
.elseif(n = 8801)
	; 19 bytes
	ldx #220
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8806)
	; 21 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 8807)
	; 17 bytes
	ldx #238
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8808)
	; 21 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 8810 || n = 8824)
	; 18..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8815)
	; 17 bytes
	ldx #226
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8816)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8819)
	; 16 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 8820)
	; 16 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 8821)
	; 16 bytes
	ldx #252
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.endif
.endif
.else
.if((n >= 8829) && (n < 8868))
; 8829 to 8867
.if(n = 8829 || n = 8864)
	; 20 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8830 || n = 8831)
	; 20 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8832 || n = 8867)
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
.elseif(n = 8835 || n = 8842 || n = 8843 || n = 8846 || (n > 8847 && n < 8852) || n = 8858 || n = 8860 || n = 8862 || n = 8865)
	; 17..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8836)
	; 21 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 8837)
	; 20 bytes
	ldx #188
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 8838 || n = 8839)
	; 20 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 8840)
	; 18 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-15
.elseif(n = 8841)
	; 19 bytes
	ldx #221
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8844)
	; 17 bytes
	ldx #239
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8845)
	; 19 bytes
	ldx #201
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8847)
	; 19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8852)
	; 21 bytes
	ldx #167
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 8853)
	; 21 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 8854)
	; 16 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 8855)
	; 16 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 8856)
	; 16 bytes
	ldx #253
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8857)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8859)
	; 18 bytes
	ldx #206
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8861)
	; 19 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 8863)
	; 19 bytes
	ldx #211
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8866)
	; 19 bytes
	ldx #197
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.endif
.else
; 8868 to 8906
.if(n = 8868 || n = 8883 || (n > 8884 && n < 8889) || n = 8892 || n = 8895 || n = 8897 || n = 8899 || n = 8900 || n = 8903 || n = 8904 || n = 8906)
	; 17..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8869)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8870)
	; 21 bytes
	ldx #181
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 8871)
	; 21 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 8872)
	; 21 bytes
	delay_n 16
	delay_n (n-16)
.elseif((n > 8872 && n < 8876))
	; 20 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 8876)
	; 21 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 8877)
	; 21 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 8878)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 8879)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 8880 || n = 8882)
	; 21 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8881)
	; 17 bytes
	ldx #240
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8884 || n = 8894 || n = 8896 || n = 8901)
	; 18..19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8889)
	; 16 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 8890)
	; 16 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 8891)
	; 16 bytes
	ldx #254
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8893)
	; 17 bytes
	ldx #228
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8898)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8902)
	; 18 bytes
	ldx #207
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8905)
	; 19 bytes
	ldx #212
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.endif
.endif
.endif
.else
.if((n >= 8907) && (n < 8985))
.if((n >= 8907) && (n < 8946))
; 8907 to 8945
.if(n = 8907 || n = 8913 || n = 8915 || n = 8920 || n = 8922 || n = 8923 || n = 8927 || n = 8928 || n = 8930 || n = 8934 || (n > 8935 && n < 8939) || n = 8941 || n = 8943 || n = 8944)
	; 17..21 bytes
	delay_n (n-2)
	nop
.elseif((n > 8907 && n < 8911))
	; 20 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 8911)
	; 19 bytes
	ldx #198
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8912)
	; 21 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 8914 || n = 8921 || n = 8935 || n = 8942)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8916 || n = 8917)
	; 21 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8918)
	; 17 bytes
	ldx #241
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8919)
	; 21 bytes
	ldx #182
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 8924)
	; 16 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 8925)
	; 16 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 8926)
	; 16 bytes
	ldx #255
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8929)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8931)
	; 18 bytes
	ldx #235
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8932)
	; 17 bytes
	ldx #229
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8933)
	; 19 bytes
	ldx #203
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8939)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8940)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8945)
	; 18 bytes
	ldx #208
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.endif
.else
; 8946 to 8984
.if(n = 8946 || n = 8947 || n = 8949 || n = 8950 || n = 8957 || n = 8962 || n = 8963 || n = 8966 || n = 8967 || n = 8970 || n = 8973 || n = 8975 || n = 8976 || n = 8981 || n = 8982 || n = 8984)
	; 17..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8948 || n = 8958 || n = 8968 || n = 8974 || n = 8983)
	; 19..20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8951)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8952)
	; 21 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8953)
	; 21 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 8954)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 8955)
	; 17 bytes
	ldx #242
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8956)
	; 19 bytes
	ldx #199
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8959)
	; 16 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8960)
	; 16 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-13
.elseif(n = 8961)
	; 16 bytes
	ldx #0
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-12
.elseif(n = 8964)
	; 17 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 8965)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8969)
	; 18 bytes
	ldx #236
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8971)
	; 17 bytes
	ldx #230
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8972)
	; 20 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 8977)
	; 19 bytes
	ldx #204
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8978)
	; 20 bytes
	ldx #191
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 8979)
	; 19 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 8980)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.endif
.endif
.else
.if((n >= 8985) && (n < 9024))
; 8985 to 9023
.if((n > 8984 && n < 8988) || n = 8990 || n = 8993 || n = 8994 || (n > 8995 && n < 8999) || n = 9002 || n = 9003 || n = 9005 || n = 9006 || n = 9009 || n = 9012 || (n > 9013 && n < 9017) || n = 9022 || n = 9023)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 8988)
	; 18 bytes
	ldx #209
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 8989)
	; 19 bytes
	ldx #214
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 8991 || n = 8995 || n = 9004 || n = 9011 || n = 9013)
	; 19..20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 8992)
	; 17 bytes
	ldx #243
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 8999)
	; 17 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9000)
	; 17 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9001)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9007)
	; 18 bytes
	ldx #237
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9008)
	; 17 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-14
.elseif(n = 9010)
	; 17 bytes
	ldx #231
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9017)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 9018)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9019)
	; 19 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 9020)
	; 19 bytes
	ldx #36 ;hides 'bit zp'
	nop
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 9021)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.endif
.else
; 9024 to 9062
.if(n = 9024 || n = 9032 || n = 9040 || n = 9050 || n = 9052)
	; 19 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n > 9024 && n < 9028) || n = 9031 || n = 9033 || n = 9034 || n = 9038 || n = 9039 || n = 9042 || n = 9043 || n = 9048 || n = 9051 || (n > 9052 && n < 9057))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9028)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 9029)
	; 17 bytes
	ldx #244
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9030)
	; 21 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 9035)
	; 17 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9036)
	; 17 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9037)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9041)
	; 19 bytes
	ldx #226
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9044)
	; 21 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 9045)
	; 18 bytes
	ldx #238
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9046)
	; 19 bytes
	ldx #201
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9047)
	; 17 bytes
	ldx #24 ;hides 'clc'
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9049)
	; 17 bytes
	ldx #232
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9057 || n = 9059)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9058 || n = 9060)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9061)
	; 19 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 9062)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 9064 || n = 9067 || n = 9068 || n = 9070 || (n > 9074 && n < 9081) || n = 9085 || n = 9087 || n = 9090 || (n > 9091 && n < 9096))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9065 || n = 9069 || n = 9086 || n = 9091)
	; 19..20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9066)
	; 17 bytes
	ldx #245
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9071)
	; 17 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9072)
	; 17 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9073)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9074)
	; 18 bytes
	ldx #211
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9081)
	; 19 bytes
	ldx #227
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9082)
	; 21 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 9083)
	; 18 bytes
	ldx #239
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9084 || n = 9089)
	; 20..21 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 9088)
	; 17 bytes
	ldx #233
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
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
	; 21 bytes
	delay_n 141
	delay_n (n-141)
.endif
.else
; 9102 to 9140
.if(n = 9102)
	; 21 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 9103)
	; 17 bytes
	ldx #246
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9104 || n = 9116)
	; 21 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 9105 || n = 9110 || n = 9111 || n = 9113 || n = 9114 || n = 9119 || n = 9122 || n = 9123 || n = 9126 || n = 9129 || (n > 9130 && n < 9135) || n = 9138)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9106 || n = 9112 || n = 9118 || n = 9120 || n = 9124 || n = 9128 || n = 9130)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9107)
	; 17 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9108)
	; 17 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9109)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9115)
	; 19 bytes
	ldx #217
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9117)
	; 18 bytes
	ldx #212
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9121)
	; 18 bytes
	ldx #240
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9125)
	; 17 bytes
	ldx #234 ;hides 'nop'
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9127)
	; 17 bytes
	ldx #234
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9135 || n = 9137)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9136)
	; 19 bytes
	ldx #203
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9139)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9140)
	; 17 bytes
	ldx #247
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.endif
.endif
.else
.if((n >= 9141) && (n < 9180))
; 9141 to 9179
.if(n = 9141)
	; 21 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 9142 || (n > 9145 && n < 9152) || n = 9162 || n = 9164 || n = 9165 || n = 9168 || (n > 9169 && n < 9174))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9143)
	; 17 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9144)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9145)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9152 || n = 9156 || n = 9178)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9153)
	; 19 bytes
	ldx #208
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9154 || n = 9176)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9155)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 9157)
	; 19 bytes
	ldx #218
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9158)
	; 22 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 9159)
	; 18 bytes
	ldx #241
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9160)
	; 18 bytes
	ldx #213
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9161)
	; 19 bytes
	ldx #229
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9163 || n = 9169)
	; 19..20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9166)
	; 17 bytes
	ldx #235
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 9177)
	; 17 bytes
	ldx #248
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9179)
	; 17 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.endif
.else
; 9180 to 9218
.if(n = 9180)
	; 17 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9181)
	; 17 bytes
	ldx #(n-1)/36
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9182 || n = 9183 || n = 9186 || n = 9187 || (n > 9188 && n < 9193) || n = 9200 || n = 9202 || n = 9207 || (n > 9208 && n < 9212) || n = 9218)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9184 || n = 9188 || n = 9204 || n = 9206 || n = 9208)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9185)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9193)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9194)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9195)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9196)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9197)
	; 18 bytes
	ldx #242
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9198)
	; 18 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9199)
	; 19 bytes
	ldx #219
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9201)
	; 19 bytes
	ldx #230
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9203)
	; 18 bytes
	ldx #214
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9205)
	; 17 bytes
	ldx #236
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9212)
	; 17 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9213)
	; 17 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9214)
	; 17 bytes
	ldx #249
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9215)
	; 17 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9216)
	; 17 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9217)
	; 17 bytes
	ldx #0
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.endif
.endif
.endif
.else
.if((n >= 9219) && (n < 9297))
.if((n >= 9219) && (n < 9258))
; 9219 to 9257
.if(n = 9219 || (n > 9220 && n < 9224) || n = 9228 || (n > 9229 && n < 9234) || n = 9237 || n = 9242 || n = 9243 || n = 9245 || n = 9246 || n = 9248 || n = 9252 || n = 9253 || (n > 9254 && n < 9258))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9220 || n = 9229 || n = 9238 || n = 9247 || n = 9254)
	; 19..20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9224 || n = 9236)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9225)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9226)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9227)
	; 21 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 9234)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9235)
	; 18 bytes
	ldx #243
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9239)
	; 18 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-15
.elseif(n = 9240)
	; 18 bytes
	ldx #36 ;hides 'bit zp'
	nop
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9241)
	; 19 bytes
	ldx #231
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9244)
	; 17 bytes
	ldx #237
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9249)
	; 17 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9250)
	; 17 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9251)
	; 17 bytes
	ldx #250
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.endif
.else
; 9258 to 9296
.if(n = 9258 || n = 9269 || n = 9271 || n = 9272 || n = 9274 || n = 9275 || n = 9277 || n = 9284 || n = 9285 || n = 9290 || (n > 9291 && n < 9295))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9259 || n = 9296)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9260)
	; 20 bytes
	ldx #197
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 9261)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9262)
	; 21 bytes
	ldx #189
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 9263 || n = 9295)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9264 || n = 9268)
	; 21 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 9265)
	; 21 bytes
	ldx #193
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 9266)
	; 22 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 9267)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9270 || n = 9276 || n = 9291)
	; 19..20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9273)
	; 18 bytes
	ldx #244
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9278)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9279)
	; 18 bytes
	ldx #24 ;hides 'clc'
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9280)
	; 22 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 9281)
	; 19 bytes
	ldx #232
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9282)
	; 18 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9283)
	; 17 bytes
	ldx #238
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9286)
	; 17 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9287)
	; 17 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9288)
	; 17 bytes
	ldx #251
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9289)
	; 18 bytes
	ldx #216
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.elseif(n = 9301 || (n > 9302 && n < 9306) || n = 9309)
	; 21 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 9302)
	; 22 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 9306)
	; 22 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 9307)
	; 20 bytes
	ldx #198
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 9308)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9310 || (n > 9311 && n < 9316) || n = 9317 || n = 9318 || n = 9326 || n = 9327 || (n > 9328 && n < 9332) || n = 9334)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9311)
	; 18 bytes
	ldx #245
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9316)
	; 19 bytes
	ldx #207
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9319 || n = 9328 || n = 9335)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9321)
	; 19 bytes
	ldx #233
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9322)
	; 17 bytes
	ldx #239
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9323)
	; 17 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9324)
	; 17 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9325)
	; 17 bytes
	ldx #252
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9332)
	; 18 bytes
	ldx #217
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9333)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.endif
.else
; 9336 to 9375
.if(n = 9336 || n = 9341 || n = 9351 || (n > 9352 && n < 9357) || n = 9363 || n = 9364 || n = 9366 || n = 9368 || n = 9369)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9337 || n = 9374)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9338 || n = 9340 || n = 9350)
	; 21 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 9339)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 9342)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9343)
	; 22 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 9344)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 9345)
	; 22 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 9346)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 9347)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9348)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 9349)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9352 || n = 9365)
	; 19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9357 || n = 9370)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9358)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 9359)
	; 18 bytes
	ldx #234 ;hides 'nop'
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9360)
	; 17 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9361)
	; 17 bytes
	ldx #240
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9362)
	; 17 bytes
	ldx #253
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9367)
	; 19 bytes
	ldx #223
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9371 || n = 9372)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9373)
	; 19 bytes
	ldx #213
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9375)
	; 18 bytes
	ldx #218
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.if(n = 9376 || n = 9378 || n = 9393 || n = 9403)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9377 || (n > 9378 && n < 9382) || n = 9389 || n = 9391 || n = 9392 || (n > 9393 && n < 9397) || n = 9401 || n = 9402 || n = 9404 || n = 9405 || n = 9407 || n = 9408 || n = 9411 || n = 9413)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9382)
	; 21 bytes
	ldx #177
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 9383)
	; 21 bytes
	ldx #184 ;hides 'clv'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 9384)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 9385)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 9386)
	; 22 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 9387)
	; 18 bytes
	ldx #247
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9388)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9390)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9397)
	; 17 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9398)
	; 17 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9399)
	; 17 bytes
	ldx #254
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9400)
	; 17 bytes
	ldx #241
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9406)
	; 19 bytes
	ldx #209
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9409)
	; 19 bytes
	ldx #224
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9410)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9412)
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
	; 19 bytes
	ldx #214
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9418)
	; 18 bytes
	ldx #219
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9419 || n = 9420 || (n > 9421 && n < 9425) || n = 9427 || n = 9429 || n = 9430 || n = 9433 || n = 9437 || n = 9438 || n = 9440 || n = 9441 || (n > 9442 && n < 9446) || n = 9453)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9421 || n = 9428 || n = 9442)
	; 19..20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9425)
	; 18 bytes
	ldx #248
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9426)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 9431)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9432)
	; 21 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 9434)
	; 17 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9435)
	; 17 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9436)
	; 17 bytes
	ldx #255
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9439)
	; 17 bytes
	ldx #242
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #201
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 9451)
	; 19 bytes
	ldx #210
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.endif
.endif
.else
.if((n >= 9454) && (n < 9493))
; 9454 to 9492
.if(n = 9454 || n = 9466 || n = 9476 || n = 9481)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9455 || n = 9464 || n = 9465 || (n > 9466 && n < 9470) || n = 9474 || n = 9475 || n = 9477 || n = 9479 || n = 9480 || (n > 9481 && n < 9485))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9456)
	; 22 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 9457)
	; 21 bytes
	ldx #197
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 9458)
	; 21 bytes
	ldx #193
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 9459)
	; 20 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 9460)
	; 20 bytes
	ldx #36 ;hides 'bit zp'
	nop
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-17
.elseif(n = 9461)
	; 18 bytes
	ldx #220
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9462)
	; 18 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9463)
	; 18 bytes
	ldx #249
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9470)
	; 18 bytes
	ldx #232 ;hides 'inx'
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	dex
	Jne *-15
.elseif(n = 9471)
	; 17 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9472)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9473)
	; 17 bytes
	ldx #0
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9478)
	; 17 bytes
	ldx #243
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9485 || n = 9490)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9486)
	; 21 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 9487)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.if(n = 9493)
	; 19 bytes
	ldx #226
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9494)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 9495)
	; 20 bytes
	ldx #202
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 9496)
	; 19 bytes
	ldx #211
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9497 || n = 9498 || n = 9502 || n = 9503 || n = 9506 || (n > 9507 && n < 9511) || n = 9515 || n = 9518 || n = 9519 || (n > 9520 && n < 9525))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9499)
	; 18 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9500)
	; 18 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9501)
	; 18 bytes
	ldx #250
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9504)
	; 18 bytes
	ldx #221
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9505)
	; 19 bytes
	ldx #216
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9507 || n = 9514 || n = 9516 || n = 9520)
	; 19..20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9511)
	; 17 bytes
	ldx #24 ;hides 'clc'
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9512)
	; 22 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 9513)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9517)
	; 17 bytes
	ldx #244
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.elseif(n = 9530 || n = 9531)
	; 22 bytes
	delay_n 131
	delay_n (n-131)
.endif
.endif
.endif
.else
.if((n >= 9532) && (n < 9610))
.if((n >= 9532) && (n < 9571))
; 9532 to 9570
.if(n = 9532)
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
.elseif(n = 9535)
	; 19 bytes
	ldx #227
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9536)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 9537)
	; 18 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9538)
	; 18 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9539)
	; 18 bytes
	ldx #251
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9540 || n = 9543 || n = 9545 || n = 9549 || (n > 9550 && n < 9554) || n = 9558 || n = 9560 || n = 9562 || n = 9563)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9541)
	; 19 bytes
	ldx #212
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9542)
	; 20 bytes
	ldx #203
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 9544 || n = 9550 || n = 9557 || n = 9559)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9546)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 9547)
	; 18 bytes
	ldx #222
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9548)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 9554)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9555)
	; 21 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 9556)
	; 17 bytes
	ldx #245
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9561)
	; 19 bytes
	ldx #239
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
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
.elseif(n = 9569)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.endif
.else
; 9571 to 9609
.if(n = 9571 || n = 9578 || n = 9579 || (n > 9580 && n < 9584) || n = 9588 || n = 9591 || n = 9592 || n = 9594 || n = 9597 || n = 9599 || n = 9600 || n = 9602 || n = 9603)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9572 || n = 9609)
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
.elseif(n = 9575)
	; 18 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9576)
	; 18 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9577)
	; 18 bytes
	ldx #252
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9580 || n = 9596 || n = 9598)
	; 19..20 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #213
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9587)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9589)
	; 20 bytes
	ldx #204
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 9590)
	; 18 bytes
	ldx #223
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9593)
	; 17 bytes
	ldx #234 ;hides 'nop'
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9595)
	; 17 bytes
	ldx #246
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9601)
	; 19 bytes
	ldx #240
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9604 || n = 9606)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9605)
	; 21 bytes
	ldx #196
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 9607)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9608)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.endif
.endif
.else
.if((n >= 9610) && (n < 9649))
; 9610 to 9648
.if(n = 9610)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 9611)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9612)
	; 22 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 9613)
	; 18 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9614)
	; 18 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9615)
	; 18 bytes
	ldx #253
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9616 || n = 9617 || n = 9620 || n = 9621 || n = 9623 || n = 9635 || n = 9638 || n = 9640 || n = 9642 || n = 9643)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9618 || n = 9622 || n = 9637 || n = 9639)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9619)
	; 19 bytes
	ldx #229
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9624)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9625)
	; 22 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 9626)
	; 22 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 9627 || n = 9630)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 9628)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 9629)
	; 22 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 9631)
	; 19 bytes
	ldx #214
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9632)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 9633)
	; 18 bytes
	ldx #224
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9634)
	; 17 bytes
	ldx #247
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9636)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9641)
	; 19 bytes
	ldx #241
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
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
	ldx #182
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 9648)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 9649 to 9687
.if(n = 9649)
	; 21 bytes
	ldx #201
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 9650)
	; 22 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9651)
	; 18 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9652)
	; 18 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9653)
	; 18 bytes
	ldx #254
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9654 || n = 9655 || (n > 9656 && n < 9660) || n = 9663 || n = 9665 || n = 9675 || n = 9678 || n = 9679 || (n > 9680 && n < 9685) || n = 9686)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9656 || n = 9664)
	; 20..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9660)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9661)
	; 19 bytes
	ldx #230
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9662)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 9666)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 9667)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 9668)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9669 || n = 9674)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 9670)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 9671)
	; 22 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 9672)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 9673)
	; 17 bytes
	ldx #248
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9676)
	; 18 bytes
	ldx #225
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9677)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9680)
	; 18 bytes
	ldx #36 ;hides 'bit zp'
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9685)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9687)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.endif
.endif
.else
.if((n >= 9688) && (n < 9844))
.if((n >= 9688) && (n < 9766))
.if((n >= 9688) && (n < 9727))
; 9688 to 9726
.if(n = 9688)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9689)
	; 18 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9690)
	; 18 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9691)
	; 18 bytes
	ldx #255
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9692 || n = 9693 || n = 9695 || n = 9696 || n = 9705 || n = 9709 || n = 9713 || n = 9714 || n = 9716 || n = 9717 || n = 9720 || n = 9722 || n = 9723 || n = 9726)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9694 || n = 9704 || n = 9706 || n = 9715 || n = 9721)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9697)
	; 21 bytes
	ldx #202
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
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
	ldx #183
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 9701)
	; 19 bytes
	ldx #232 ;hides 'inx'
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-16
.elseif(n = 9702)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 9703)
	; 19 bytes
	ldx #231
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9707)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 9708)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 9710)
	; 18 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9711)
	; 18 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9712)
	; 17 bytes
	ldx #249
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9718)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9719)
	; 18 bytes
	ldx #226
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9724)
	; 18 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9725)
	; 19 bytes
	ldx #221
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.endif
.else
; 9727 to 9765
.if(n = 9727)
	; 18 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9728)
	; 18 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9729)
	; 18 bytes
	ldx #0
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9730 || n = 9731 || (n > 9732 && n < 9736) || n = 9747 || n = 9752 || n = 9753 || (n > 9754 && n < 9759) || n = 9761 || n = 9763 || n = 9764)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9732 || n = 9746 || n = 9748 || n = 9754 || n = 9765)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 9743)
	; 19 bytes
	ldx #24 ;hides 'clc'
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-16
.elseif(n = 9744)
	; 22 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 9745)
	; 19 bytes
	ldx #232
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9749)
	; 18 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9750)
	; 18 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9751)
	; 17 bytes
	ldx #250
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9759)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9760)
	; 21 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 9762)
	; 18 bytes
	ldx #227
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.endif
.endif
.else
.if((n >= 9766) && (n < 9805))
; 9766 to 9804
.if(n = 9766)
	; 19 bytes
	ldx #217
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9767 || n = 9768 || n = 9770 || n = 9771 || n = 9773 || n = 9779 || n = 9791 || n = 9792 || (n > 9793 && n < 9798) || n = 9802 || n = 9804)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9769)
	; 19 bytes
	ldx #222
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9772 || n = 9793 || n = 9803)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #208
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.elseif(n = 9785)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 9786)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 9787)
	; 19 bytes
	ldx #233
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9788)
	; 18 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9789)
	; 18 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9790)
	; 17 bytes
	ldx #251
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9798)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9799)
	; 20 bytes
	ldx #56 ;hides 'sec'
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 9800)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9801)
	; 19 bytes
	ldx #245
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.endif
.else
; 9805 to 9843
.if(n = 9805)
	; 18 bytes
	ldx #228
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9806 || n = 9807 || n = 9809 || n = 9815 || n = 9817 || n = 9826 || n = 9830 || n = 9831 || (n > 9832 && n < 9837) || n = 9843)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9808 || n = 9814 || n = 9816 || n = 9832)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9810 || n = 9839)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9811)
	; 19 bytes
	ldx #218
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9812)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9813)
	; 19 bytes
	ldx #223
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
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
	ldx #209
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 9825)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 9827)
	; 18 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9828)
	; 18 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9829)
	; 17 bytes
	ldx #252
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
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
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9842)
	; 21 bytes
	delay_n 124
	delay_n (n-124)
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
	ldx $A6
.elseif((n > 9844 && n < 9848) || n = 9850 || n = 9852 || n = 9854 || (n > 9857 && n < 9862) || n = 9869 || n = 9870 || (n > 9871 && n < 9876))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9848)
	; 18 bytes
	ldx #229
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9849 || n = 9876)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9853 || n = 9880)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9855)
	; 20 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 9856)
	; 19 bytes
	ldx #219
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9857)
	; 19 bytes
	ldx #224
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
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
	; 18 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9867)
	; 18 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9868)
	; 17 bytes
	ldx #253
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
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
	; 19 bytes
	ldx #247
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9882)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.endif
.else
; 9883 to 9921
.if(n = 9883 || n = 9884 || (n > 9885 && n < 9890) || n = 9893 || (n > 9894 && n < 9898) || n = 9902 || n = 9903 || n = 9908 || n = 9909 || n = 9911 || n = 9912 || n = 9914 || n = 9915 || n = 9920)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9885 || n = 9894 || n = 9904 || n = 9910 || n = 9916)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9890)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9891)
	; 18 bytes
	ldx #230
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #202
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 9900)
	; 20 bytes
	ldx #36 ;hides 'bit zp'
	nop
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 9901)
	; 19 bytes
	ldx #220
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9905)
	; 18 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9906)
	; 18 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9907)
	; 17 bytes
	ldx #254
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9913)
	; 19 bytes
	ldx #236
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9918)
	; 20 bytes
	ldx #211
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 9919)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9921)
	; 19 bytes
	ldx #248
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
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
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9924)
	; 21 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 9925 || (n > 9926 && n < 9931) || n = 9936 || (n > 9937 && n < 9941) || n = 9947 || n = 9948 || (n > 9949 && n < 9954) || n = 9957)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9926 || n = 9935 || n = 9937 || n = 9949)
	; 19..20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9931 || n = 9954)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9932)
	; 18 bytes
	ldx #232 ;hides 'inx'
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-15
.elseif(n = 9933 || n = 9956)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9934)
	; 18 bytes
	ldx #231
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
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
	; 18 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9945)
	; 18 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9946)
	; 17 bytes
	ldx #255
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9955)
	; 19 bytes
	ldx #237
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9958)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9959)
	; 19 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 9960)
	; 19 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.endif
.else
; 9961 to 10000
.if(n = 9961)
	; 19 bytes
	ldx #249
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9962 || n = 9963 || n = 9966 || (n > 9967 && n < 9972) || n = 9979 || n = 9981 || n = 9982 || n = 9986 || n = 9987 || n = 9989 || n = 9990 || n = 9992 || n = 9993)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 9964)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9965)
	; 20 bytes
	ldx #212
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 9967 || n = 9978 || n = 9980 || n = 9988 || n = 9994)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 9972)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
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
	ldx #24 ;hides 'clc'
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-14
.elseif(n = 9976)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9977)
	; 18 bytes
	ldx #232
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 9983)
	; 18 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9984)
	; 18 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 6
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 9985)
	; 17 bytes
	ldx #0
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 9991)
	; 19 bytes
	ldx #222
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9996)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 9997)
	; 19 bytes
	ldx #238
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 9998)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 9999)
	; 19 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10000)
	; 19 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
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
; 10001 to 10039
.if(n = 10001)
	; 19 bytes
	ldx #250
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10002 || n = 10003 || n = 10007 || (n > 10008 && n < 10012) || n = 10014 || n = 10022 || (n > 10023 && n < 10027) || n = 10035 || n = 10037 || n = 10038)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10004 || n = 10008 || n = 10023)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10005)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 10006 || n = 10031)
	; 21 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 10012)
	; 20 bytes
	ldx #213
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10013)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10015 || n = 10030)
	; 21..22 bytes
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
	ldx #189
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10019)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10020)
	; 18 bytes
	ldx #233
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10021)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10027)
	; 22 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 10028)
	; 22 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 10029)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10032)
	; 22 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 10033)
	; 19 bytes
	ldx #228
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10034)
	; 22 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 10036)
	; 19 bytes
	ldx #223
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10039)
	; 19 bytes
	ldx #239
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.endif
.else
; 10040 to 10078
.if(n = 10040)
	; 19 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10041)
	; 19 bytes
	ldx #251
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10042 || n = 10043 || n = 10045 || n = 10048 || (n > 10049 && n < 10054) || n = 10065 || (n > 10066 && n < 10070) || n = 10076 || n = 10078)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10044 || n = 10049 || n = 10064 || n = 10066)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10046)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 10047 || n = 10070)
	; 21 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 10054)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10055)
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
	ldx #214
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10060)
	; 22 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 10061)
	; 17 bytes
	ldx #234 ;hides 'nop'
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10062)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10063)
	; 18 bytes
	ldx #234
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10071)
	; 21 bytes
	ldx #190
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10074)
	; 19 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10075)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10077)
	; 19 bytes
	ldx #229
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.endif
.endif
.else
.if((n >= 10079) && (n < 10118))
.if((n >= 10079) && (n < 10098))
; 10079 to 10097
.if(n = 10079)
	; 19 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10080)
	; 19 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10081)
	; 19 bytes
	ldx #224
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10082 || n = 10083 || n = 10085 || n = 10089 || (n > 10090 && n < 10095))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10084 || n = 10090)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10086)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10087)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 10088)
	; 21 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 10095)
	; 21 bytes
	ldx #206
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10096)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 10097)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 10098 to 10117
.if(n = 10098)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10099)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10100)
	; 22 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 10101)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10102)
	; 22 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10103)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10104)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10105)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10106)
	; 18 bytes
	ldx #235
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10107)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10108 || (n > 10109 && n < 10113))
	; 19..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10109)
	; 20 bytes
	delay_n (n-3)
	ldx $A6
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
.endif
.else
.if((n >= 10118) && (n < 10137))
; 10118 to 10136
.if(n = 10118)
	; 22 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 10119)
	; 19 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10120)
	; 19 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10121)
	; 19 bytes
	ldx #253
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10122 || n = 10125 || n = 10127 || n = 10130 || (n > 10131 && n < 10136))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10123)
	; 19 bytes
	ldx #241
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10124)
	; 21 bytes
	ldx #191
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10126)
	; 19 bytes
	ldx #225
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10128)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 10129 || n = 10131)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10136)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.endif
.else
; 10137 to 10156
.if(n = 10137)
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
.elseif(n = 10141)
	; 22 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10142)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10143)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 10144)
	; 21 bytes
	ldx #207
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10145)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10146)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10147)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10148)
	; 22 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 10149)
	; 18 bytes
	ldx #236
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10150)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10151 || (n > 10152 && n < 10156))
	; 19..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10152)
	; 20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10156)
	; 22 bytes
	delay_n 171
	delay_n (n-171)
.endif
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
	; 19 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10160)
	; 19 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10161)
	; 19 bytes
	ldx #254
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10162 || n = 10163 || n = 10167 || n = 10168 || n = 10171 || (n > 10172 && n < 10176))
	; 18..20 bytes
	delay_n (n-2)
	nop
.elseif(n = 10164 || n = 10172)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10165)
	; 19 bytes
	ldx #242
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10166)
	; 19 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10169)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 10170)
	; 21 bytes
	delay_n 124
	delay_n (n-124)
.endif
.else
; 10176 to 10195
.if(n = 10176 || n = 10194)
	; 19..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10177)
	; 21 bytes
	ldx #192
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10178)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10179)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10180)
	; 21 bytes
	delay_n 11
	delay_n (n-11)
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
	; 21 bytes
	delay_n 124
	delay_n (n-124)
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
	ldx #237
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10193)
	; 21 bytes
	ldx #208
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10195)
	; 20 bytes
	delay_n (n-3)
	ldx $A6
.endif
.endif
.else
; 10196 to 10234
.if((n > 10195 && n < 10199) || n = 10202 || n = 10203 || n = 10205 || n = 10206 || n = 10211 || n = 10212 || n = 10214 || n = 10215 || n = 10217 || n = 10218)
	; 18..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 10199)
	; 19 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10200)
	; 19 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10201)
	; 19 bytes
	ldx #255
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10204 || n = 10213 || n = 10219)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10207)
	; 19 bytes
	ldx #243
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10208)
	; 19 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10209)
	; 19 bytes
	ldx #232
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10210)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 10216)
	; 19 bytes
	ldx #227
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
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
	; 21 bytes
	ldx #213
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
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
	ldx #193
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.else
.if((n >= 10235) && (n < 10274))
; 10235 to 10273
.if(n = 10235)
	; 18 bytes
	ldx #238
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10236)
	; 22 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10237 || n = 10242 || n = 10243 || n = 10245 || n = 10252 || n = 10253 || (n > 10254 && n < 10259) || n = 10263 || n = 10265)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10238 || n = 10244 || n = 10254 || n = 10264)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10239)
	; 19 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10240)
	; 19 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10241)
	; 19 bytes
	ldx #0
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10246)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10247)
	; 20 bytes
	ldx #218
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10248)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10249)
	; 19 bytes
	ldx #244
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10250)
	; 19 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10251)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 10259)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10260)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10261)
	; 19 bytes
	ldx #228
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 21 bytes
	ldx #214
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
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
	ldx #239
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10279)
	; 22 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10280 || n = 10282 || n = 10284 || n = 10293 || n = 10294 || n = 10296 || n = 10298 || n = 10299 || n = 10301 || n = 10307 || n = 10308 || n = 10310)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10281 || n = 10295 || n = 10309)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10283)
	; 21 bytes
	ldx #194
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 19 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10291)
	; 19 bytes
	ldx #245
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10292)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 10297)
	; 19 bytes
	ldx #234
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10300)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10303)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10304)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10305)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10306)
	; 19 bytes
	ldx #229
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
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
	ldx #240
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx $A6
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
	; 19 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-16
.endif
.else
; 10332 to 10351
.if(n = 10332)
	; 19 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10333)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 10334 || n = 10335 || (n > 10336 && n < 10340) || n = 10342 || n = 10343)
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10336)
	; 19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10340)
	; 20 bytes
	ldx #36 ;hides 'bit zp'
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 10341)
	; 19 bytes
	ldx #235
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
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
	ldx #230
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.endif
.endif
.else
; 10352 to 10390
.if(n = 10352)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10353 || n = 10355 || n = 10356 || n = 10366 || (n > 10367 && n < 10371) || n = 10376 || (n > 10377 && n < 10382) || n = 10387 || n = 10390)
	; 18..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 10354)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #241
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10365)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10367 || n = 10377)
	; 19..20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10371)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10372)
	; 19 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10373)
	; 19 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10374)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 10375)
	; 19 bytes
	ldx #247
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10382)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10383)
	; 22 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 10384)
	; 21 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10385)
	; 19 bytes
	ldx #236
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10386)
	; 21 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10388)
	; 20 bytes
	ldx #221
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10389)
	; 21 bytes
	ldx #212
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.endif
.endif
.else
.if((n >= 10391) && (n < 10430))
; 10391 to 10429
.if(n = 10391 || n = 10398 || n = 10400 || n = 10409 || n = 10411 || n = 10412 || n = 10416 || n = 10417 || (n > 10418 && n < 10423))
	; 18..22 bytes
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
	; 18 bytes
	ldx #232 ;hides 'inx'
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-15
.elseif(n = 10395)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10396)
	; 19 bytes
	ldx #231
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10397 || n = 10399 || n = 10410 || n = 10418)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10406)
	; 22 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10407)
	; 18 bytes
	ldx #242
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10408)
	; 22 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10413)
	; 19 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10414)
	; 19 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10415)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 10423)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
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
	; 19 bytes
	ldx #237
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.endif
.else
; 10430 to 10468
.if(n = 10430)
	; 22 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 10431 || n = 10433 || n = 10437 || n = 10440 || n = 10443 || n = 10445 || n = 10452 || n = 10457 || n = 10458 || (n > 10459 && n < 10464))
	; 18..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 10432 || n = 10442 || n = 10444 || n = 10453 || n = 10459)
	; 19..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10434)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10435)
	; 20 bytes
	ldx #222
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10436)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10438)
	; 21 bytes
	ldx #213
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10439)
	; 18 bytes
	ldx #24 ;hides 'clc'
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-15
.elseif(n = 10441)
	; 19 bytes
	ldx #232
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
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
	ldx #243
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10451)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10454)
	; 19 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10455)
	; 19 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10456)
	; 17 bytes
	ldx #(n-1)/41
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 10464)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10465)
	; 21 bytes
	ldx #218
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
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
	; 19 bytes
	ldx #238
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10474)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10475 || n = 10477 || n = 10484)
	; 20..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10476)
	; 21 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #223
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10483)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10485)
	; 22 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 10486)
	; 19 bytes
	ldx #233
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10487)
	; 21 bytes
	ldx #214
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.endif
.else
; 10488 to 10507
.if(n = 10488 || n = 10490 || n = 10491 || n = 10495 || n = 10498 || n = 10499 || (n > 10500 && n < 10505))
	; 18..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10489)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10492)
	; 22 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10493)
	; 18 bytes
	ldx #244
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10494)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10496)
	; 19 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10497)
	; 17 bytes
	ldx #0
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-13
.elseif(n = 10500)
	; 19 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10505)
	; 21 bytes
	delay_n (n-8)
	delay_n 8
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
	; 20 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 10513)
	; 21 bytes
	ldx #219
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10514 || n = 10519 || n = 10521)
	; 20..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10515)
	; 22 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 10516)
	; 22 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10517)
	; 19 bytes
	ldx #239
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10518)
	; 22 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10520)
	; 21 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #234 ;hides 'nop'
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10530)
	; 22 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10531)
	; 19 bytes
	ldx #234
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10532 || n = 10534 || n = 10539 || n = 10546)
	; 20..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10533 || n = 10537 || n = 10538 || n = 10540 || n = 10544 || n = 10545)
	; 19..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10535)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10536)
	; 18 bytes
	ldx #245
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10541)
	; 20 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 10542)
	; 20 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 10543)
	; 19 bytes
	ldx #251
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.endif
.endif
.endif
.else
.if((n >= 10547) && (n < 10586))
.if((n >= 10547) && (n < 10566))
; 10547 to 10565
.if(n = 10547 || n = 10549 || n = 10562 || n = 10563 || n = 10565)
	; 20..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 10548)
	; 21 bytes
	ldx #199
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
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
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10559)
	; 20 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 10560)
	; 20 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 10561)
	; 19 bytes
	ldx #240
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10564)
	; 21 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 10569 || n = 10575 || n = 10578 || n = 10581 || n = 10583)
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
.elseif(n = 10574 || n = 10580)
	; 21..22 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 10576)
	; 19 bytes
	ldx #235
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10577)
	; 22 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 10579)
	; 18 bytes
	ldx #246
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10582)
	; 20 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10584)
	; 20 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 9
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 10585)
	; 19 bytes
	ldx #252
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.endif
.endif
.else
; 10586 to 10625
.if(n = 10586 || n = 10587 || n = 10589 || n = 10594 || n = 10595 || n = 10600 || n = 10607 || n = 10610 || n = 10611 || n = 10614 || n = 10615 || n = 10619 || n = 10620 || n = 10624)
	; 19..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10588 || n = 10604 || n = 10625)
	; 20..23 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #56 ;hides 'sec'
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 10601)
	; 21 bytes
	ldx #200
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10602)
	; 22 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 10603)
	; 22 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10605)
	; 19 bytes
	ldx #241
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10606)
	; 22 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10608)
	; 20 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 10609)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
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
	ldx #193
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #236
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10622)
	; 18 bytes
	ldx #247
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10623)
	; 20 bytes
	ldx #226
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
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
.if(n = 10626 || n = 10628 || n = 10629 || n = 10631 || n = 10635 || n = 10636 || n = 10639 || n = 10640 || n = 10644 || n = 10645 || n = 10651 || n = 10653 || n = 10655 || n = 10656 || n = 10659 || n = 10662)
	; 20..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10627)
	; 19 bytes
	ldx #253
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10630 || n = 10652)
	; 21 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #217
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.elseif(n = 10649)
	; 19 bytes
	ldx #242
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10650)
	; 22 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 10654)
	; 21 bytes
	ldx #201
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10657)
	; 21 bytes
	ldx #222
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10658)
	; 22 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10660)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10661)
	; 23 bytes
	ldx #205
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 10663)
	; 22 bytes
	delay_n 166
	delay_n (n-166)
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
	ldx #248
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10666)
	; 19 bytes
	ldx #237
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10667 || n = 10672 || n = 10675)
	; 19..21 bytes
	delay_n (n-2)
	nop
.elseif(n = 10668 || n = 10674)
	; 20..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10669)
	; 19 bytes
	ldx #254
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10670)
	; 20 bytes
	ldx #227
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10671)
	; 19 bytes
	ldx #24 ;hides 'clc'
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10673)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10676)
	; 22 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10677)
	; 22 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10678)
	; 22 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 10679)
	; 23 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 10680)
	; 23 bytes
	ldx #181
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 10681)
	; 23 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10682)
	; 23 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 10683)
	; 21 bytes
	ldx #218
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.endif
.else
; 10684 to 10703
.if(n = 10684 || n = 10688)
	; 22..23 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10685 || n = 10687 || n = 10690 || n = 10695 || (n > 10696 && n < 10700))
	; 20..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10686 || n = 10696)
	; 21..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10689)
	; 23 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10691)
	; 23 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10692)
	; 23 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10693)
	; 19 bytes
	ldx #243
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10694)
	; 23 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 10700)
	; 23 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 10701)
	; 22 bytes
	ldx #214
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 10702)
	; 23 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 10703)
	; 22 bytes
	delay_n 124
	delay_n (n-124)
.endif
.endif
.endif
.else
.if((n >= 10704) && (n < 10743))
; 10704 to 10742
.if(n = 10704)
	; 23 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10705)
	; 21 bytes
	ldx #223
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10706)
	; 19 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10707)
	; 19 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10708)
	; 18 bytes
	ldx #249
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10709 || n = 10710 || n = 10712 || n = 10713 || n = 10715 || n = 10716 || n = 10721 || n = 10723 || n = 10733 || n = 10734 || n = 10736 || n = 10739 || n = 10741 || n = 10742)
	; 19..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10711)
	; 19 bytes
	ldx #238
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10714 || n = 10722 || n = 10735 || n = 10740)
	; 21..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10717)
	; 20 bytes
	ldx #228
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10718)
	; 22 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10719)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10720)
	; 22 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10724)
	; 23 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10725)
	; 23 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 10726)
	; 22 bytes
	ldx #195
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 10727)
	; 23 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10728)
	; 23 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10729)
	; 23 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10730)
	; 23 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10731)
	; 21 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 10732)
	; 21 bytes
	ldx #219
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10737)
	; 19 bytes
	ldx #244
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10738)
	; 23 bytes
	delay_n 116
	delay_n (n-116)
.endif
.else
.if((n >= 10743) && (n < 10762))
; 10743 to 10761
.if(n = 10743 || n = 10752 || n = 10753 || n = 10755 || n = 10757 || n = 10758)
	; 19..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 10744)
	; 23 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 10745)
	; 23 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 10746)
	; 22 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 10747)
	; 23 bytes
	ldx #199
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 10748)
	; 23 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 10749)
	; 19 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10750)
	; 19 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10751)
	; 18 bytes
	ldx #250
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10754 || n = 10759)
	; 20..21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10756)
	; 19 bytes
	ldx #239
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10760)
	; 21 bytes
	ldx #203
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10763)
	; 19 bytes
	ldx #234 ;hides 'nop'
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10764)
	; 20 bytes
	ldx #229
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10765)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif((n > 10765 && n < 10770))
	; 21..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 10770)
	; 23 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 10771)
	; 23 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10772)
	; 23 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10773)
	; 23 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10774)
	; 23 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 10775)
	; 23 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10776)
	; 23 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 10777)
	; 23 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10778)
	; 23 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 10779)
	; 21 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 10780)
	; 21 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 10781)
	; 19 bytes
	ldx #245
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.endif
.endif
.endif
.endif
.else
.if((n >= 10782) && (n < 10860))
.if((n >= 10782) && (n < 10821))
; 10782 to 10820
.if(n = 10782 || n = 10783 || (n > 10784 && n < 10788) || n = 10795 || n = 10796 || (n > 10797 && n < 10801) || n = 10803 || n = 10805 || n = 10807 || n = 10815 || n = 10817)
	; 19..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10784 || n = 10797 || n = 10804 || n = 10814 || n = 10816)
	; 20..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10788)
	; 23 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 10789 || n = 10790)
	; 22..23 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 10791)
	; 23 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 10792)
	; 19 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10793)
	; 19 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10794)
	; 18 bytes
	ldx #251
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10801)
	; 19 bytes
	ldx #240
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 23 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 10810)
	; 23 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10811)
	; 20 bytes
	ldx #230
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10812)
	; 23 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 10813)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10818)
	; 23 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10819)
	; 23 bytes
	delay_n 111
	delay_n (n-111)
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
.elseif(n = 10824)
	; 23 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10825)
	; 19 bytes
	ldx #246
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10826)
	; 23 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 10827 || n = 10829 || n = 10831 || n = 10832 || n = 10834 || n = 10838 || n = 10839 || (n > 10840 && n < 10845) || n = 10848 || (n > 10849 && n < 10853))
	; 19..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10828 || n = 10833 || n = 10840 || n = 10849 || n = 10859)
	; 20..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10830)
	; 21 bytes
	ldx #221
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10835)
	; 19 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10836)
	; 19 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10837)
	; 18 bytes
	ldx #252
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10845)
	; 22 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10846)
	; 19 bytes
	ldx #241
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10847)
	; 22 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10853)
	; 23 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10854)
	; 23 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 10855)
	; 23 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 10856)
	; 19 bytes
	ldx #232 ;hides 'inx'
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-16
.elseif(n = 10857)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10858)
	; 20 bytes
	ldx #231
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.endif
.endif
.else
.if((n >= 10860) && (n < 10899))
; 10860 to 10898
.if(n = 10860 || n = 10862 || n = 10868 || n = 10871 || n = 10873 || n = 10874 || n = 10876 || n = 10881 || n = 10882 || (n > 10883 && n < 10888) || n = 10893 || n = 10895 || n = 10896)
	; 19..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10861 || n = 10867 || n = 10872 || n = 10883 || n = 10894)
	; 20..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10863)
	; 23 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10864)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10865)
	; 23 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 10866)
	; 21 bytes
	ldx #205
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10869)
	; 19 bytes
	ldx #247
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10870)
	; 23 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 10875)
	; 22 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 10877)
	; 23 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 10878)
	; 19 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10879)
	; 19 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10880)
	; 18 bytes
	ldx #253
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #242
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10892)
	; 22 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10897)
	; 21 bytes
	ldx #227
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10898)
	; 23 bytes
	delay_n 61
	delay_n (n-61)
.endif
.else
; 10899 to 10937
.if(n = 10899 || n = 10907 || n = 10909 || n = 10915 || n = 10917 || n = 10924 || n = 10925 || n = 10927 || n = 10929 || n = 10930 || n = 10932)
	; 19..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 10900)
	; 23 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10901)
	; 22 bytes
	ldx #218
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 10902)
	; 23 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 10903)
	; 19 bytes
	ldx #24 ;hides 'clc'
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10904)
	; 23 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 10905)
	; 20 bytes
	ldx #232
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10906 || n = 10908 || n = 10916 || n = 10926)
	; 20..22 bytes
	delay_n (n-3)
	ldx $A6
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
	; 19 bytes
	ldx #248
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10914 || n = 10934)
	; 22..23 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 10918)
	; 22 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 10919)
	; 21 bytes
	ldx #206
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10920)
	; 23 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 10921)
	; 19 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10922)
	; 19 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10923)
	; 18 bytes
	ldx #254
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10928)
	; 21 bytes
	ldx #223
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10931)
	; 22 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10933)
	; 22 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10935)
	; 22 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10936)
	; 19 bytes
	ldx #243
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10937)
	; 23 bytes
	delay_n 186
	delay_n (n-186)
.endif
.endif
.endif
.endif
.else
.if((n >= 10938) && (n < 11094))
.if((n >= 10938) && (n < 11016))
.if((n >= 10938) && (n < 10977))
; 10938 to 10976
.if(n = 10938 || (n > 10939 && n < 10943) || n = 10947 || n = 10951 || n = 10953 || n = 10954 || n = 10958 || n = 10959 || (n > 10960 && n < 10964) || n = 10967 || n = 10968 || n = 10970 || n = 10971 || n = 10973 || n = 10974)
	; 19..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 10939 || n = 10960 || n = 10969 || n = 10975)
	; 20..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10943)
	; 23 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 10944)
	; 23 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 10945)
	; 21 bytes
	ldx #228
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10946)
	; 22 bytes
	ldx #199
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 10948)
	; 23 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 10949)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10950)
	; 21 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 10952)
	; 20 bytes
	ldx #233
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10955)
	; 21 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-18
.elseif(n = 10956)
	; 21 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-18
.elseif(n = 10957)
	; 19 bytes
	ldx #249
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 10964)
	; 19 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10965)
	; 19 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 10966)
	; 18 bytes
	ldx #255
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-14
.elseif(n = 10972)
	; 21 bytes
	ldx #207
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #224
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10978)
	; 22 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10979 || n = 10983 || (n > 10984 && n < 10988) || n = 11003 || n = 11005 || n = 11006 || n = 11010 || n = 11011 || (n > 11012 && n < 11016))
	; 19..22 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 2 = 0) && (n = 10980 || n = 10984 || (n > 10997 && n < 11005) || n = 11012))
	; 20..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 10981)
	; 19 bytes
	ldx #244
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 21 bytes
	ldx #229
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 10994)
	; 23 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 10995)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10996)
	; 23 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 10997)
	; 19 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 10999)
	; 20 bytes
	ldx #234
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11001)
	; 19 bytes
	ldx #250
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 11007)
	; 19 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11008)
	; 19 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 11009)
	; 18 bytes
	ldx #0
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #208
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11026)
	; 19 bytes
	ldx #245
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 11029)
	; 21 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11033)
	; 23 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 11034)
	; 23 bytes
	ldx #187
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11042)
	; 23 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 11043 || (n > 11046 && n < 11050) || n = 11051 || n = 11052)
	; 20..22 bytes
	delay_n (n-2)
	nop
.elseif(n = 11044)
	; 21 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11045)
	; 19 bytes
	ldx #251
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 11046)
	; 20 bytes
	ldx #235
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11050)
	; 21 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11053)
	; 23 bytes
	delay_n (n-8)
	delay_n 8
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
	ldx #201
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11057)
	; 23 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11058 || n = 11063 || n = 11070 || n = 11072 || n = 11073)
	; 20..24 bytes
	delay_n (n-2)
	nop
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
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11069)
	; 23 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 11071)
	; 19 bytes
	ldx #246
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.endif
.else
; 11074 to 11093
.if(n = 11074 || n = 11081 || n = 11092)
	; 21..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11075)
	; 21 bytes
	ldx #226
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11076 || n = 11077 || n = 11079 || n = 11080 || n = 11086 || n = 11091)
	; 20..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11078)
	; 21 bytes
	ldx #209
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11088)
	; 21 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11089)
	; 19 bytes
	ldx #252
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 11090)
	; 22 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 11093)
	; 20 bytes
	ldx #236
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.endif
.endif
.endif
.endif
.else
.if((n >= 11094) && (n < 11172))
.if((n >= 11094) && (n < 11133))
; 11094 to 11132
.if(n = 11094 || n = 11095 || n = 11097 || n = 11098 || n = 11103 || n = 11106 || n = 11113 || n = 11118 || (n > 11119 && n < 11124) || n = 11126 || n = 11128)
	; 20..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11096 || n = 11127)
	; 22..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11099)
	; 23 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 11100)
	; 23 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11101)
	; 22 bytes
	ldx #222
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
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
	ldx #202
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11112)
	; 23 bytes
	delay_n 146
	delay_n (n-146)
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
	ldx #247
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 11117)
	; 23 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11119)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11124)
	; 21 bytes
	ldx #227
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11125)
	; 23 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 11129)
	; 23 bytes
	ldx #214
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11130)
	; 23 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 11131)
	; 21 bytes
	ldx #210
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11132)
	; 21 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-18
.endif
.else
; 11133 to 11171
.if(n = 11133)
	; 19 bytes
	ldx #253
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 11134 || n = 11135 || (n > 11136 && n < 11140) || n = 11142 || n = 11144 || n = 11146 || n = 11149 || n = 11153 || n = 11154 || n = 11159 || n = 11163 || n = 11165 || n = 11167 || n = 11168)
	; 20..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11136 || n = 11143 || n = 11164)
	; 21..22 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11140)
	; 20 bytes
	ldx #237
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11141)
	; 23 bytes
	delay_n (n-8)
	delay_n 8
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
.elseif(n = 11150)
	; 23 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 11151)
	; 22 bytes
	ldx #223
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11152)
	; 23 bytes
	ldx #189
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.elseif(n = 11161)
	; 19 bytes
	ldx #248
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 11162)
	; 23 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 11166)
	; 22 bytes
	ldx #203
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11169)
	; 22 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 11170)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11171)
	; 23 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
.if((n >= 11172) && (n < 11211))
; 11172 to 11210
.if(n = 11172 || n = 11174 || n = 11178 || n = 11182 || n = 11183 || n = 11186 || n = 11188 || n = 11189 || (n > 11190 && n < 11195) || n = 11197 || n = 11202 || n = 11203 || n = 11207 || n = 11208 || n = 11210)
	; 20..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11173)
	; 21 bytes
	ldx #228
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11175)
	; 21 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11176)
	; 21 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11177)
	; 19 bytes
	ldx #254
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 11179)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11180 || n = 11190 || n = 11209)
	; 21..22 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11181)
	; 21 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 11184)
	; 21 bytes
	ldx #211
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11185)
	; 21 bytes
	ldx #233
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11187)
	; 20 bytes
	ldx #238
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11195)
	; 23 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 11196)
	; 24 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11198)
	; 24 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11199)
	; 23 bytes
	ldx #56 ;hides 'sec'
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 11200)
	; 23 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 11201)
	; 22 bytes
	ldx #224
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11204)
	; 19 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 11205)
	; 19 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 11206)
	; 19 bytes
	ldx #249
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.endif
.else
; 11211 to 11250
.if((n > 11210 && n < 11214) || n = 11223 || n = 11226 || n = 11227 || n = 11229 || n = 11235 || n = 11236 || n = 11238 || n = 11239 || (n > 11240 && n < 11244))
	; 20..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11214)
	; 23 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11215 || n = 11216 || n = 11244)
	; 23..24 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11217 || n = 11232)
	; 23 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11218 || n = 11246)
	; 23..24 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11219)
	; 21 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11220)
	; 21 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11221)
	; 19 bytes
	ldx #255
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 11222)
	; 21 bytes
	ldx #229
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11224 || n = 11228 || n = 11240)
	; 21..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11225)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11230)
	; 23 bytes
	ldx #197
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11231)
	; 20 bytes
	ldx #234 ;hides 'nop'
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11233)
	; 21 bytes
	ldx #234
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11234)
	; 20 bytes
	ldx #239
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11237)
	; 21 bytes
	ldx #212
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11245)
	; 24 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 11247)
	; 24 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 11248)
	; 24 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 11249)
	; 19 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 11250)
	; 19 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
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
	ldx #250
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 11252 || n = 11253 || (n > 11254 && n < 11259) || n = 11266 || n = 11267 || n = 11269 || n = 11270 || (n > 11272 && n < 11276) || n = 11278 || n = 11280 || n = 11283 || n = 11285 || n = 11286 || n = 11288 || n = 11289)
	; 20..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11254 || n = 11268 || n = 11279 || n = 11284)
	; 21..24 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11259)
	; 23 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11260 || n = 11261)
	; 23 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11262)
	; 23 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11263)
	; 21 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11264)
	; 21 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 12
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11265)
	; 19 bytes
	ldx #0
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 11271)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11272)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11276)
	; 22 bytes
	ldx #205
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11277)
	; 23 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11281)
	; 20 bytes
	ldx #240
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11282)
	; 24 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11287)
	; 23 bytes
	ldx #198
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.endif
.else
; 11290 to 11328
.if(n = 11290)
	; 21 bytes
	ldx #213
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11291 || n = 11305 || n = 11306)
	; 23..24 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11292 || n = 11297 || n = 11298 || (n > 11299 && n < 11304) || n = 11309 || n = 11310 || n = 11315 || n = 11319 || n = 11322 || n = 11324 || n = 11325 || n = 11327)
	; 20..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11293 || n = 11299 || n = 11321 || n = 11326)
	; 21..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11294)
	; 19 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 11295)
	; 19 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 11296)
	; 19 bytes
	ldx #251
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 11304)
	; 23 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11307)
	; 23 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11308)
	; 23 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11311)
	; 24 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 11312)
	; 24 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11313)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11314)
	; 24 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 11316)
	; 24 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 11317)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11318)
	; 20 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-17
.elseif(n = 11320)
	; 21 bytes
	ldx #231
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11323)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11328)
	; 20 bytes
	ldx #241
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.endif
.endif
.else
.if((n >= 11329) && (n < 11368))
; 11329 to 11367
.if(n = 11329)
	; 21 bytes
	ldx #236
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11330 || (n > 11331 && n < 11336) || n = 11342 || n = 11343 || (n > 11344 && n < 11349) || n = 11354 || n = 11355 || n = 11364 || n = 11365)
	; 20..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11331)
	; 22 bytes
	ldx #206
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11336 || n = 11349)
	; 23..24 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11337)
	; 23 bytes
	ldx #218
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11338 || n = 11350)
	; 23..24 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11339)
	; 19 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 11340)
	; 19 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 11341)
	; 19 bytes
	ldx #252
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 11344 || n = 11366)
	; 21..22 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11351)
	; 22 bytes
	ldx #227
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11352)
	; 23 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11353)
	; 23 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11356)
	; 24 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11357)
	; 24 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11358)
	; 24 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 11359)
	; 24 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 11360 || n = 11362)
	; 24 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 11361)
	; 24 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 11363)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11367)
	; 20 bytes
	ldx #24 ;hides 'clc'
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.endif
.else
; 11368 to 11406
.if(n = 11368 || n = 11371 || n = 11373 || n = 11376 || n = 11377 || (n > 11378 && n < 11382) || n = 11387 || n = 11388 || (n > 11389 && n < 11394) || n = 11398 || n = 11400 || n = 11403)
	; 20..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 11369)
	; 21 bytes
	ldx #232
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11370 || n = 11372 || n = 11378 || n = 11389)
	; 21..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11374)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11375)
	; 20 bytes
	ldx #242
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11382)
	; 24 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 11383 || n = 11394)
	; 23..24 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11384)
	; 19 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 11385)
	; 19 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 11386)
	; 19 bytes
	ldx #253
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 11395)
	; 23 bytes
	ldx #211
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11396)
	; 21 bytes
	ldx #215
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11397)
	; 23 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11399)
	; 22 bytes
	ldx #56 ;hides 'sec'
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 11401)
	; 22 bytes
	ldx #228
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11402)
	; 24 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11404)
	; 24 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 11405)
	; 24 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 11406)
	; 24 bytes
	delay_n 141
	delay_n (n-141)
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
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 11409)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11410)
	; 24 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11411 || (n > 11412 && n < 11416) || n = 11420 || n = 11423 || n = 11424 || n = 11426 || n = 11427 || n = 11432 || n = 11433 || (n > 11434 && n < 11439) || n = 11443 || n = 11445)
	; 20..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11412 || n = 11421 || n = 11428 || n = 11434 || n = 11444)
	; 21..24 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11416)
	; 24 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11417)
	; 24 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11418)
	; 21 bytes
	ldx #233
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11419)
	; 23 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 11422)
	; 20 bytes
	ldx #243
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11425)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11429)
	; 19 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 11430)
	; 19 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 11431)
	; 19 bytes
	ldx #254
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 11439)
	; 22 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 11440)
	; 22 bytes
	ldx #36 ;hides 'bit zp'
	nop
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 11441)
	; 22 bytes
	ldx #208
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11442)
	; 23 bytes
	delay_n 11
	delay_n (n-11)
.endif
.else
; 11446 to 11484
.if(n = 11446)
	; 24 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 11447)
	; 23 bytes
	ldx #194
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11448)
	; 24 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 11449)
	; 21 bytes
	ldx #216
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11450)
	; 24 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 11451 || n = 11456 || n = 11457 || (n > 11458 && n < 11462) || n = 11471 || n = 11477 || n = 11478 || (n > 11479 && n < 11484))
	; 20..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 11452 || n = 11458 || n = 11468 || n = 11470 || n = 11472 || n = 11479)
	; 21..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11453)
	; 21 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11454)
	; 21 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11455)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11462)
	; 24 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11463 || n = 11464)
	; 23 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 11465)
	; 20 bytes
	ldx #234 ;hides 'nop'
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11466)
	; 24 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11467)
	; 21 bytes
	ldx #234
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11469)
	; 20 bytes
	ldx #244
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11473)
	; 21 bytes
	ldx #239
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11474)
	; 19 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 11475)
	; 19 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 11476)
	; 19 bytes
	ldx #255
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.elseif(n = 11484)
	; 23 bytes
	delay_n (n-8)
	delay_n 8
.endif
.endif
.else
.if((n >= 11485) && (n < 11524))
; 11485 to 11523
.if(n = 11485 || n = 11486 || n = 11511)
	; 23..24 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11487)
	; 23 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11488 || n = 11513)
	; 23..24 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11489 || n = 11490 || n = 11494 || n = 11495 || n = 11498 || (n > 11502 && n < 11506) || n = 11507 || n = 11518 || n = 11522 || n = 11523)
	; 20..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11491 || (n > 11507 && n < 11511))
	; 23..24 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 11492)
	; 22 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 11493)
	; 23 bytes
	ldx #221
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11496)
	; 22 bytes
	ldx #209
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11497)
	; 24 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 11499)
	; 21 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11500)
	; 21 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11501)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11502)
	; 21 bytes
	ldx #217
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11506)
	; 23 bytes
	ldx #195
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11512)
	; 24 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11514)
	; 24 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 11515)
	; 23 bytes
	ldx #202
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11516)
	; 20 bytes
	ldx #245
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11517)
	; 24 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 11519)
	; 19 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11520)
	; 19 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-16
.elseif(n = 11521)
	; 19 bytes
	ldx #0
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-15
.endif
.else
; 11524 to 11562
.if(n = 11524 || n = 11530 || n = 11550 || n = 11558)
	; 21..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11525 || n = 11526 || n = 11528 || n = 11529 || n = 11534 || n = 11535 || n = 11544 || n = 11548 || n = 11549 || n = 11552 || n = 11553 || n = 11557 || (n > 11558 && n < 11562))
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11527)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11531)
	; 23 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11532)
	; 23 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11533)
	; 23 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11536)
	; 24 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11537)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11538 || n = 11539)
	; 24 bytes
	delay_n 153
	delay_n (n-153)
.elseif((n > 11539 && n < 11543))
	; 24 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 11543)
	; 24 bytes
	ldx #199
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 11545)
	; 21 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11546)
	; 21 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11547)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11551)
	; 22 bytes
	ldx #210
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11554)
	; 23 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 11555)
	; 21 bytes
	ldx #218
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11556)
	; 24 bytes
	delay_n 36
	delay_n (n-36)
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
	ldx #246
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11564 || n = 11567 || n = 11570 || n = 11571 || n = 11573 || n = 11574 || n = 11580 || (n > 11581 && n < 11585) || n = 11594 || n = 11595 || n = 11597 || n = 11598)
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11565)
	; 21 bytes
	ldx #236
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11566 || n = 11568 || n = 11581 || n = 11596)
	; 22..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11569)
	; 21 bytes
	ldx #241
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11572)
	; 23 bytes
	ldx #203
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11575)
	; 24 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11576 || n = 11577)
	; 24 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 11578)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11579 || n = 11589 || n = 11600)
	; 23..24 bytes
	delay_n 124
	delay_n (n-124)
.elseif((n > 11584 && n < 11588))
	; 24 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 11588)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11590)
	; 24 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 11591)
	; 21 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11592)
	; 21 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11593)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11599)
	; 21 bytes
	ldx #24 ;hides 'clc'
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11601)
	; 22 bytes
	ldx #232
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.endif
.else
; 11602 to 11640
.if(n = 11602 || n = 11604 || n = 11609 || n = 11611 || n = 11613 || n = 11620 || n = 11632)
	; 22..24 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11603 || n = 11612 || n = 11615 || n = 11616 || n = 11618 || n = 11619 || n = 11621 || n = 11623 || n = 11626 || n = 11631 || (n > 11632 && n < 11636) || n = 11640)
	; 21..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11605)
	; 24 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 11606)
	; 22 bytes
	ldx #211
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11607)
	; 23 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 11608)
	; 21 bytes
	ldx #219
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11610)
	; 20 bytes
	ldx #247
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11614)
	; 21 bytes
	ldx #237
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11617)
	; 21 bytes
	ldx #242
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11622)
	; 24 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 11624)
	; 23 bytes
	ldx #197
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11625)
	; 24 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 11627)
	; 24 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 11628)
	; 24 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 11629)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11630)
	; 24 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 11636)
	; 24 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 11637)
	; 21 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11638)
	; 21 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11639)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.endif
.endif
.else
.if((n >= 11641) && (n < 11680))
; 11641 to 11679
.if(n = 11641 || n = 11643 || n = 11644 || n = 11646 || n = 11653 || n = 11655 || n = 11659 || n = 11662 || n = 11667 || n = 11669 || n = 11670 || n = 11679)
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11642 || n = 11654 || n = 11660 || n = 11664 || n = 11666 || n = 11668)
	; 22..24 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11645 || n = 11671)
	; 23..24 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 11647)
	; 24 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 11648 || n = 11650)
	; 24 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 11649)
	; 23 bytes
	ldx #224
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11651)
	; 22 bytes
	ldx #233
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11652)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 11656)
	; 24 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 11657)
	; 20 bytes
	ldx #248
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11658)
	; 25 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 11661)
	; 21 bytes
	ldx #220
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11663)
	; 21 bytes
	ldx #238
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11665)
	; 21 bytes
	ldx #243
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11672)
	; 24 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 11673 || n = 11674)
	; 24 bytes
	delay_n 153
	delay_n (n-153)
.elseif((n > 11674 && n < 11678))
	; 24 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 11678)
	; 25 bytes
	delay_n 21
	delay_n (n-21)
.endif
.else
; 11680 to 11718
.if(n = 11680)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11681)
	; 24 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 11682 || n = 11686 || n = 11687 || (n > 11688 && n < 11692) || n = 11705 || n = 11706 || (n > 11707 && n < 11712) || n = 11715)
	; 21..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11683)
	; 21 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11684)
	; 21 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11685)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11688 || n = 11707 || n = 11717)
	; 22..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11692)
	; 24 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 11693)
	; 24 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11694 || n = 11695)
	; 24 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 11696)
	; 24 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11697)
	; 24 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 11698)
	; 25 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 11699)
	; 21 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11700)
	; 24 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 11701)
	; 22 bytes
	ldx #234
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11702)
	; 20 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 11703)
	; 20 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 11704)
	; 20 bytes
	ldx #249
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11712)
	; 21 bytes
	ldx #239
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11713)
	; 21 bytes
	ldx #244
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11714)
	; 21 bytes
	ldx #221
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11716)
	; 22 bytes
	ldx #213
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.if(n = 11719)
	; 23 bytes
	ldx #217
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11720 || n = 11721 || n = 11732 || n = 11733 || (n > 11734 && n < 11739) || n = 11744 || n = 11745 || n = 11747 || n = 11752 || n = 11753 || (n > 11754 && n < 11758))
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11722 || n = 11741)
	; 24..25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11723)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11724 || n = 11726)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11725)
	; 25 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 11727)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 11728)
	; 25 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 11729)
	; 21 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11730)
	; 21 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11731)
	; 20 bytes
	ldx #(n-1)/46
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11734 || n = 11746 || n = 11754)
	; 22..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11739)
	; 24 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11740)
	; 25 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 11742)
	; 23 bytes
	ldx #199
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11743)
	; 23 bytes
	ldx #206
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11748)
	; 25 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11749)
	; 20 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 11750)
	; 20 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 11751)
	; 20 bytes
	ldx #250
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.endif
.else
; 11758 to 11796
.if(n = 11758 || n = 11763 || n = 11765 || n = 11766 || n = 11768 || n = 11769 || n = 11772 || n = 11773 || n = 11778 || n = 11779 || n = 11781 || n = 11784 || (n > 11785 && n < 11789))
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11759 || n = 11790)
	; 24..25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11760 || n = 11792)
	; 24..25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11761)
	; 21 bytes
	ldx #240
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11762 || n = 11793)
	; 24..25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11764 || n = 11770 || n = 11774 || n = 11783 || n = 11785)
	; 23..24 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11767)
	; 21 bytes
	ldx #222
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11771)
	; 22 bytes
	ldx #214
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11775)
	; 21 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11776)
	; 21 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11777)
	; 20 bytes
	ldx #0
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11780)
	; 21 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-18
.elseif(n = 11782)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11789 || n = 11794)
	; 24..25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11791)
	; 25 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 11795)
	; 25 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11796)
	; 20 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.endif
.endif
.else
.if((n >= 11797) && (n < 11836))
; 11797 to 11835
.if(n = 11797)
	; 20 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 11798)
	; 20 bytes
	ldx #251
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11799 || n = 11800 || (n > 11801 && n < 11805) || n = 11807 || n = 11811 || n = 11812 || (n > 11813 && n < 11817) || n = 11819 || n = 11822 || n = 11824 || n = 11825 || n = 11828 || n = 11830 || n = 11835)
	; 21..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11801 || n = 11813 || n = 11823 || n = 11829 || n = 11834)
	; 22..24 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11805)
	; 23 bytes
	ldx #227
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11806 || n = 11818)
	; 24..25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11808)
	; 24 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11809)
	; 21 bytes
	ldx #246
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11810)
	; 21 bytes
	ldx #241
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11817)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11820)
	; 21 bytes
	ldx #223
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11821)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11826)
	; 22 bytes
	ldx #215
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11827)
	; 23 bytes
	ldx #219
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11831)
	; 21 bytes
	ldx #24 ;hides 'clc'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11832)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11833)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.endif
.else
; 11836 to 11875
.if(n = 11836 || n = 11848 || n = 11854 || n = 11862)
	; 22..24 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n > 11836 && n < 11841) || n = 11846 || n = 11847 || n = 11849 || n = 11850 || n = 11852 || n = 11853 || n = 11858 || n = 11861 || (n > 11862 && n < 11867) || n = 11875)
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11841 || n = 11855 || n = 11869)
	; 24..25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11842 || n = 11868)
	; 25 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 11843)
	; 20 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 11844)
	; 20 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 11845)
	; 20 bytes
	ldx #252
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11851)
	; 22 bytes
	ldx #237
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11856 || n = 11870)
	; 24..25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11857)
	; 21 bytes
	ldx #247
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11859)
	; 21 bytes
	ldx #242
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11860)
	; 23 bytes
	ldx #201
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11867)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
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
	ldx #224
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11874)
	; 24 bytes
	delay_n 124
	delay_n (n-124)
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
.if(n = 11876 || n = 11887 || n = 11895 || n = 11911)
	; 22..23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11877 || n = 11878 || n = 11882 || n = 11883 || n = 11885 || n = 11886 || n = 11888 || n = 11889 || n = 11893 || n = 11894 || (n > 11895 && n < 11900) || n = 11903 || n = 11907 || n = 11910 || n = 11912 || n = 11913)
	; 21..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 11879)
	; 23 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 11880)
	; 22 bytes
	ldx #36 ;hides 'bit zp'
	nop
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 11881)
	; 22 bytes
	ldx #216
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11884)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11890)
	; 20 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 11891)
	; 20 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 11892)
	; 20 bytes
	ldx #253
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11900)
	; 24 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11901)
	; 22 bytes
	ldx #238
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11902)
	; 24 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11904)
	; 24 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11905)
	; 21 bytes
	ldx #248
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11906)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 11908)
	; 21 bytes
	ldx #243
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11909)
	; 23 bytes
	ldx #229
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11914)
	; 23 bytes
	ldx #209
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.endif
.else
; 11915 to 11953
.if(n = 11915 || n = 11916 || n = 11921 || n = 11923 || n = 11928 || (n > 11929 && n < 11933) || n = 11940 || n = 11941 || (n > 11942 && n < 11947))
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11917 || n = 11929 || n = 11942)
	; 22..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11918 || n = 11948 || n = 11949)
	; 24..25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11919)
	; 23 bytes
	ldx #202
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11920 || n = 11922)
	; 24 bytes
	delay_n 124
	delay_n (n-124)
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
	ldx #225
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11927)
	; 25 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 11933)
	; 21 bytes
	ldx #234 ;hides 'nop'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11934)
	; 22 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 11935)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11936)
	; 22 bytes
	ldx #217
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 11937)
	; 20 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 11938)
	; 20 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 11939)
	; 20 bytes
	ldx #254
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11947)
	; 24 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11950)
	; 24 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 11951)
	; 21 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11952)
	; 21 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 11953)
	; 21 bytes
	ldx #249
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.endif
.endif
.else
.if((n >= 11954) && (n < 11993))
; 11954 to 11992
.if(n = 11954 || n = 11955 || n = 11958 || n = 11959 || (n > 11960 && n < 11965) || n = 11973 || n = 11980 || n = 11981 || n = 11983 || n = 11987 || n = 11988 || n = 11990 || n = 11992)
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11956 || n = 11960 || n = 11974 || n = 11982 || n = 11989)
	; 22..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11957)
	; 21 bytes
	ldx #244
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11965)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11966)
	; 25 bytes
	delay_n 121
	delay_n (n-121)
.elseif((n > 11966 && n < 11970))
	; 24 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 11970)
	; 25 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 11971)
	; 23 bytes
	ldx #210
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 25 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 11978)
	; 23 bytes
	ldx #203
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 11979)
	; 21 bytes
	ldx #226
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 11984)
	; 20 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 11985)
	; 20 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 11986)
	; 20 bytes
	ldx #255
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 11991)
	; 22 bytes
	ldx #218
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.endif
.else
; 11993 to 12031
.if(n = 11993 || n = 11995 || n = 12002 || n = 12003 || n = 12005 || n = 12008 || n = 12010 || n = 12012 || n = 12015 || n = 12020 || n = 12030)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 11994 || n = 12004 || n = 12009 || n = 12014)
	; 23..24 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 11996)
	; 24 bytes
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
.elseif(n = 11999)
	; 21 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12000)
	; 21 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12001)
	; 21 bytes
	ldx #250
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12006)
	; 21 bytes
	ldx #245
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12007)
	; 24 bytes
	ldx #207
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 12011)
	; 22 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-19
.elseif(n = 12013)
	; 23 bytes
	ldx #231
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12016)
	; 24 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12018)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.elseif(n = 12024 || n = 12025)
	; 25 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 12026)
	; 25 bytes
	ldx #185
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 12027)
	; 25 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 12028)
	; 23 bytes
	ldx #211
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12029)
	; 25 bytes
	ldx #194
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 12031)
	; 20 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.endif
.endif
.endif
.else
.if((n >= 12032) && (n < 12110))
.if((n >= 12032) && (n < 12071))
; 12032 to 12070
.if(n = 12032)
	; 20 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-17
.elseif(n = 12033)
	; 20 bytes
	ldx #0
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-16
.elseif(n = 12034 || n = 12035 || n = 12038 || n = 12039 || n = 12041 || n = 12042 || n = 12050 || n = 12054 || n = 12056 || n = 12057 || (n > 12058 && n < 12062) || n = 12067)
	; 21..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12036 || n = 12040 || n = 12052 || n = 12058 || n = 12066)
	; 22..24 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12037)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12043)
	; 23 bytes
	ldx #223
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
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
	ldx #219
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12047)
	; 21 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12048)
	; 21 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12049)
	; 21 bytes
	ldx #251
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12051)
	; 22 bytes
	ldx #241
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12053)
	; 23 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 12055)
	; 21 bytes
	ldx #246
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12062)
	; 24 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12063)
	; 22 bytes
	ldx #24 ;hides 'clc'
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12064)
	; 25 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12065)
	; 23 bytes
	ldx #232
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
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
.if(n = 12071 || n = 12072)
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
.elseif(n = 12076 || n = 12077)
	; 25 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12078)
	; 25 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 12079)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 12080)
	; 25 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12081)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12082)
	; 25 bytes
	delay_n 96
	delay_n (n-96)
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
	ldx #228
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12086)
	; 25 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 12087 || n = 12089 || n = 12090 || (n > 12091 && n < 12095) || n = 12098 || n = 12099 || n = 12102 || n = 12103 || n = 12105 || n = 12106 || n = 12108 || n = 12109)
	; 22..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 12088)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12091 || n = 12100 || n = 12107)
	; 23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12095)
	; 21 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12096)
	; 21 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12097)
	; 21 bytes
	ldx #252
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12101)
	; 22 bytes
	ldx #220
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12104)
	; 21 bytes
	ldx #247
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.endif
.endif
.else
.if((n >= 12110) && (n < 12149))
.if((n >= 12110) && (n < 12129))
; 12110 to 12128
.if(n = 12110 || n = 12111 || n = 12119 || n = 12121)
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
	; 23 bytes
	ldx #233
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12118)
	; 25 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 12120)
	; 25 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 12122)
	; 25 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12123)
	; 24 bytes
	ldx #209
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 12124)
	; 25 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12125)
	; 25 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 12126 || n = 12127)
	; 25 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12128)
	; 25 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 12129 to 12148
.if(n = 12129)
	; 25 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12130)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12131 || n = 12132)
	; 25 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 12133 || n = 12134)
	; 25 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 12135)
	; 25 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 12136 || n = 12137)
	; 25 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12138)
	; 21 bytes
	ldx #229
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12139)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12140 || n = 12141 || n = 12146 || n = 12147)
	; 22 bytes
	delay_n (n-2)
	nop
.elseif(n = 12142 || n = 12148)
	; 23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12143)
	; 21 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12144)
	; 21 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12145)
	; 21 bytes
	ldx #253
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.endif
.endif
.else
; 12149 to 12187
.if(n = 12149 || n = 12150 || n = 12155 || n = 12157 || n = 12158 || n = 12160 || n = 12161 || n = 12171 || n = 12173 || n = 12183)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12151)
	; 22 bytes
	ldx #243
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12152)
	; 25 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 12153)
	; 21 bytes
	ldx #248
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12154 || n = 12159 || n = 12170)
	; 24 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12156)
	; 22 bytes
	ldx #221
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12162)
	; 25 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 12163)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12164)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12165)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12166)
	; 25 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 12167)
	; 22 bytes
	ldx #234 ;hides 'nop'
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12168)
	; 25 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12169)
	; 23 bytes
	ldx #234
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12172)
	; 25 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 12174)
	; 25 bytes
	delay_n 141
	delay_n (n-141)
.elseif((n > 12174 && n < 12178))
	; 25 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12178)
	; 25 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 12179)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12180 || n = 12182)
	; 25 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 12181)
	; 24 bytes
	ldx #210
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 12184)
	; 25 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12185 || n = 12186)
	; 25 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 12187)
	; 25 bytes
	delay_n 156
	delay_n (n-156)
.endif
.endif
.endif
.endif
.else
.if((n >= 12188) && (n < 12344))
.if((n >= 12188) && (n < 12266))
.if((n >= 12188) && (n < 12227))
; 12188 to 12226
.if(n = 12188 || n = 12189)
	; 25 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 12190)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12191)
	; 21 bytes
	ldx #230
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12192)
	; 21 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12193)
	; 21 bytes
	ldx #254
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12194 || n = 12195 || n = 12197 || n = 12198 || n = 12203 || n = 12204 || (n > 12205 && n < 12209) || n = 12213 || n = 12215 || n = 12216 || n = 12218 || n = 12223 || n = 12225)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12196 || n = 12205 || n = 12217)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12199)
	; 23 bytes
	ldx #214
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12200)
	; 21 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12201)
	; 21 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12202)
	; 21 bytes
	ldx #249
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12209)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12210)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12211)
	; 22 bytes
	ldx #222
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12212)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12214)
	; 23 bytes
	ldx #207
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12219)
	; 25 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 12220)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12221)
	; 23 bytes
	ldx #235
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12222 || n = 12224)
	; 25 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12226)
	; 26 bytes
	delay_n 171
	delay_n (n-171)
.endif
.else
.if((n >= 12227) && (n < 12246))
; 12227 to 12245
.if(n = 12227 || n = 12229)
	; 25 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 12228)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12230)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 12231)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 12232)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 12233)
	; 26 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12234)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12235)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 12236)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 12237)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12238)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12239)
	; 21 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12240)
	; 21 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12241)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12242)
	; 22 bytes
	ldx #232 ;hides 'inx'
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-19
.elseif(n = 12243 || n = 12245)
	; 22..23 bytes
	delay_n (n-2)
	nop
.elseif(n = 12244)
	; 21 bytes
	ldx #231
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.endif
.else
; 12246 to 12265
.if(n = 12246 || n = 12248 || n = 12252 || n = 12253 || n = 12255 || n = 12257 || n = 12258 || n = 12260 || n = 12261)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12247 || n = 12254)
	; 23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12249)
	; 21 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12250)
	; 21 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12251)
	; 21 bytes
	ldx #250
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12256)
	; 23 bytes
	ldx #215
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12259)
	; 23 bytes
	ldx #227
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12262)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 12263)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12264)
	; 22 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12265)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.endif
.endif
.endif
.else
.if((n >= 12266) && (n < 12305))
; 12266 to 12304
.if(n = 12266)
	; 22 bytes
	ldx #223
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12267 || n = 12268 || (n > 12269 && n < 12273) || n = 12275 || n = 12290 || n = 12291 || n = 12293 || n = 12294 || n = 12296 || n = 12302 || n = 12304)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12269 || n = 12276 || n = 12303)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12273)
	; 23 bytes
	ldx #208
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12274)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 12277)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
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
.elseif(n = 12282)
	; 26 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12283)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12284)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 12285)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 12286)
	; 25 bytes
	ldx #195
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 12287)
	; 21 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12288)
	; 21 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12289)
	; 21 bytes
	ldx #0
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12292)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12295)
	; 21 bytes
	ldx #24 ;hides 'clc'
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12297)
	; 21 bytes
	ldx #232
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12298)
	; 21 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12299)
	; 21 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12300)
	; 21 bytes
	ldx #251
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12301)
	; 22 bytes
	ldx #246
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.endif
.else
; 12305 to 12343
.if(n = 12305 || n = 12306 || n = 12315 || n = 12317 || n = 12322 || n = 12323 || n = 12327 || n = 12329 || n = 12334 || n = 12336)
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
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12313)
	; 23 bytes
	ldx #216
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12314 || n = 12326)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12316 || n = 12324 || n = 12328 || n = 12335)
	; 24..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12318)
	; 26 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 12319)
	; 23 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 12320)
	; 22 bytes
	ldx #36 ;hides 'bit zp'
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12321)
	; 22 bytes
	ldx #224
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12325)
	; 23 bytes
	ldx #237
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12330)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12331)
	; 26 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 12332)
	; 23 bytes
	ldx #209
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12333)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 12337)
	; 26 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 12338)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12339)
	; 25 bytes
	ldx #199
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 12340)
	; 26 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12341)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 12342)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12343)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.endif
.endif
.endif
.else
.if((n >= 12344) && (n < 12422))
.if((n >= 12344) && (n < 12383))
; 12344 to 12382
.if(n = 12344)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12345 || n = 12351 || n = 12352 || (n > 12353 && n < 12358) || n = 12369 || n = 12371 || n = 12372 || n = 12374 || n = 12378 || (n > 12379 && n < 12383))
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12346 || n = 12353 || n = 12373 || n = 12379)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12347)
	; 21 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12348)
	; 21 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12349)
	; 21 bytes
	ldx #252
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12350)
	; 21 bytes
	ldx #233
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.elseif((n > 12362 && n < 12366) || n = 12368 || n = 12375)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12366)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 12367)
	; 23 bytes
	ldx #229
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12370)
	; 23 bytes
	ldx #217
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12376)
	; 22 bytes
	ldx #225
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12377)
	; 23 bytes
	ldx #238
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.endif
.else
.if((n >= 12383) && (n < 12402))
; 12383 to 12401
.if(n = 12383)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 12384)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 12385)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12386 || n = 12393 || n = 12395 || n = 12399 || n = 12400)
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
	ldx #210
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12392)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 12394)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12396)
	; 21 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12397)
	; 21 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12398)
	; 21 bytes
	ldx #253
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12401)
	; 21 bytes
	ldx #234 ;hides 'nop'
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.endif
.else
; 12402 to 12421
.if(n = 12402 || n = 12405 || (n > 12406 && n < 12410))
	; 22..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 12403)
	; 21 bytes
	ldx #234
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12404 || n = 12406)
	; 23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12410 || n = 12415)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12411)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12412)
	; 25 bytes
	ldx #197
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 12413)
	; 24 bytes
	ldx #214
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 12414)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12416)
	; 25 bytes
	ldx #191
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12420)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 12421)
	; 23 bytes
	ldx #230
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.endif
.endif
.endif
.else
.if((n >= 12422) && (n < 12461))
; 12422 to 12460
.if(n = 12422 || n = 12424)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12423 || n = 12425 || n = 12433 || (n > 12434 && n < 12438) || n = 12448 || n = 12449 || n = 12452 || n = 12453 || n = 12455 || n = 12458 || n = 12460)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12426)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 12427)
	; 23 bytes
	ldx #218
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12428)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12429)
	; 23 bytes
	ldx #239
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12430 || n = 12432 || n = 12434 || n = 12454 || n = 12459)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12431)
	; 22 bytes
	ldx #226
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12438)
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
	; 26 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12444)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12445)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12446)
	; 21 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12447)
	; 21 bytes
	ldx #254
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12450)
	; 22 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12451)
	; 22 bytes
	ldx #249
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12456)
	; 21 bytes
	ldx #235
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12457)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 12461 to 12500
.if((n > 12460 && n < 12464) || n = 12477 || n = 12479 || n = 12483 || n = 12485 || n = 12488 || (n > 12489 && n < 12494) || n = 12497 || n = 12498)
	; 22..25 bytes
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
.elseif(n = 12467 || n = 12472 || n = 12474)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
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
	; 24 bytes
	ldx #215
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 12473)
	; 23 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-20
.elseif(n = 12475)
	; 23 bytes
	ldx #231
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12476 || n = 12478 || n = 12487)
	; 25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12480)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 12481)
	; 23 bytes
	ldx #240
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12482)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12484)
	; 23 bytes
	ldx #219
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12486)
	; 22 bytes
	ldx #227
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12489)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12494)
	; 21 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12495)
	; 21 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12496)
	; 21 bytes
	ldx #255
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12499)
	; 22 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12500)
	; 22 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
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
; 12501 to 12539
.if(n = 12501)
	; 22 bytes
	ldx #250
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12502 || n = 12503 || n = 12505 || n = 12511 || (n > 12512 && n < 12517) || n = 12531 || n = 12535 || n = 12537)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12504 || n = 12512 || n = 12530 || n = 12532 || n = 12536)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12506)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 12507 || n = 12520)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12508 || n = 12521)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12509)
	; 21 bytes
	ldx #236
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12510)
	; 26 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 12517)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12518 || n = 12522 || n = 12525)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12519)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12523)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 12524)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 12526 || n = 12528)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12527)
	; 23 bytes
	ldx #24 ;hides 'clc'
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 12529)
	; 23 bytes
	ldx #232
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12533)
	; 23 bytes
	ldx #241
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12534)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12538)
	; 25 bytes
	ldx #199
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 12539)
	; 25 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.endif
.else
; 12540 to 12578
.if(n = 12540)
	; 24 bytes
	ldx #36 ;hides 'bit zp'
	nop
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 12541)
	; 22 bytes
	ldx #228
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12542 || n = 12546 || n = 12549 || n = 12552 || n = 12553 || n = 12555 || n = 12564 || n = 12566 || n = 12567 || n = 12569 || n = 12570)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12543)
	; 21 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12544)
	; 21 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12545)
	; 21 bytes
	ldx #0
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12547)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12548 || n = 12554 || n = 12565 || n = 12571)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12550)
	; 22 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12551)
	; 22 bytes
	ldx #251
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
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
	ldx #237
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12563)
	; 26 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 12568)
	; 23 bytes
	ldx #213
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12572)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
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
.elseif(n = 12580)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12581)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12582 || n = 12595)
	; 26 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 12583)
	; 23 bytes
	ldx #233
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12584)
	; 26 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 12585)
	; 23 bytes
	ldx #242
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12586 || n = 12588)
	; 25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12587 || n = 12589)
	; 24..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12590 || n = 12594)
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
.elseif(n = 12596)
	; 22 bytes
	ldx #229
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12597)
	; 24 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.endif
.else
; 12598 to 12617
.if(n = 12598)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12599)
	; 22 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12600 || (n > 12601 && n < 12606) || n = 12607 || n = 12617)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12601)
	; 22 bytes
	ldx #252
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12606)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
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
	ldx #194
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #238
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12616)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.endif
.endif
.else
; 12618 to 12656
.if(n = 12618 || n = 12630 || n = 12638 || n = 12640 || n = 12652)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n > 12618 && n < 12623) || n = 12629 || n = 12631 || n = 12639 || n = 12641 || n = 12647 || n = 12651 || n = 12653 || n = 12654 || n = 12656)
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
	ldx #214
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12628)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 12632)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 12633)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12634)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 12635)
	; 23 bytes
	ldx #234 ;hides 'nop'
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 12636)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12637)
	; 23 bytes
	ldx #243
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
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
.elseif(n = 12645)
	; 24 bytes
	ldx #218
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 12646 || n = 12648)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 12649)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12650)
	; 22 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12655)
	; 23 bytes
	ldx #222
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.endif
.endif
.endif
.else
.if((n >= 12657) && (n < 12735))
.if((n >= 12657) && (n < 12696))
; 12657 to 12695
.if(n = 12657)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12658 || n = 12671 || n = 12692 || n = 12694)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12659 || n = 12670 || (n > 12671 && n < 12676) || n = 12688 || n = 12690 || n = 12693 || n = 12695)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12660 || n = 12679)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 12661 || n = 12680)
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
	ldx #201
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 12665)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12666)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 12667 || n = 12669)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12668)
	; 21 bytes
	ldx #239
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12676)
	; 25 bytes
	ldx #195
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 12677)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 12678)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
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
	ldx #215
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12687)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 12689)
	; 23 bytes
	ldx #244
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12691)
	; 23 bytes
	ldx #235
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.endif
.else
; 12696 to 12734
.if(n = 12696)
	; 26 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 12697 || n = 12702 || n = 12705 || n = 12708 || n = 12710 || n = 12711 || (n > 12713 && n < 12718) || n = 12723 || (n > 12724 && n < 12729))
	; 22..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 12698)
	; 22 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12699)
	; 22 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12700)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12701)
	; 22 bytes
	ldx #254
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12703 || n = 12707 || n = 12709 || n = 12724)
	; 23..24 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12704)
	; 22 bytes
	ldx #232 ;hides 'inx'
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-19
.elseif(n = 12706)
	; 22 bytes
	ldx #231
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12712)
	; 23 bytes
	ldx #223
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12713)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
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
	ldx #240
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12722)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
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
.elseif(n = 12739 || n = 12773)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12740)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12741)
	; 23 bytes
	ldx #245
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12742)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 12743 || n = 12747 || n = 12752 || n = 12753 || (n > 12754 && n < 12759) || n = 12763 || (n > 12764 && n < 12769) || n = 12771)
	; 22..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 12744 || n = 12748 || n = 12754 || n = 12762 || n = 12764 || n = 12772)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12745)
	; 23 bytes
	ldx #216
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12746)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 12749)
	; 22 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12750)
	; 22 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12751)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12759)
	; 21 bytes
	ldx #24 ;hides 'clc'
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-18
.elseif(n = 12760)
	; 23 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 12761)
	; 22 bytes
	ldx #232
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12769)
	; 23 bytes
	ldx #224
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12770)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.endif
.else
; 12774 to 12812
.if(n = 12774)
	; 21 bytes
	ldx #241
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #203
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 12791)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 12792)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12793)
	; 23 bytes
	ldx #246
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12794)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 12798)
	; 26 bytes
	ldx #191
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 12799)
	; 22 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12800)
	; 22 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12801)
	; 22 bytes
	ldx #0
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12802)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12811)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	; 22..25 bytes
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
	ldx #233
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12817)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 12819 || n = 12830 || n = 12848)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12823)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 12824)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12825)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12826)
	; 23 bytes
	ldx #225
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12827)
	; 21 bytes
	ldx #242
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 25 bytes
	ldx #214
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
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
	; 23 bytes
	ldx #247
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12846)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 12850)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 12851)
	; 22 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.endif
.else
; 12852 to 12890
.if(n = 12852)
	; 22 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12853)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12854 || n = 12855 || (n > 12856 && n < 12861) || n = 12865 || n = 12867 || n = 12873 || n = 12875 || n = 12876 || n = 12882 || (n > 12883 && n < 12888))
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12856 || n = 12866 || n = 12872 || n = 12874 || n = 12883)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #218
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	ldx #234 ;hides 'nop'
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12870)
	; 26 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 12871)
	; 22 bytes
	ldx #234
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12877)
	; 24 bytes
	ldx #222
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
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
	ldx #243
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12881)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
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
.elseif(n = 12894)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 12895)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 12896)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 12897)
	; 23 bytes
	ldx #248
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12898)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12899 || n = 12901 || n = 12905 || n = 12906 || (n > 12907 && n < 12912) || n = 12923 || n = 12924 || n = 12928)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12900 || n = 12907 || n = 12925 || n = 12929)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12902)
	; 22 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12903)
	; 22 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12904)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
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
	ldx #205
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 12917)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 12918)
	; 26 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 12919)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 12920)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12921)
	; 24 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 12922)
	; 23 bytes
	ldx #219
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12926)
	; 22 bytes
	ldx #235
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12927)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.endif
.else
; 12930 to 12968
.if((n > 12929 && n < 12933) || n = 12935 || (n > 12936 && n < 12940) || n = 12942 || n = 12944 || n = 12950 || n = 12951 || n = 12956 || n = 12957 || n = 12959 || n = 12960 || n = 12962 || n = 12963)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 12933)
	; 21 bytes
	ldx #244
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12934)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 12936 || n = 12952 || n = 12958 || n = 12964)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12940)
	; 23 bytes
	ldx #227
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12941)
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
	; 23 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 12948)
	; 23 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 12949)
	; 23 bytes
	ldx #249
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12953)
	; 22 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12954)
	; 22 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 12955)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 12961)
	; 23 bytes
	ldx #240
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
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
.if((n >= 12969) && (n < 12988))
; 12969 to 12987
.if(n = 12969)
	; 26 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 12970)
	; 26 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 12971)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12972 || n = 12982 || n = 12983 || n = 12985 || n = 12987)
	; 23..27 bytes
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
.elseif(n = 12976)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 12977)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 12978)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 12979)
	; 25 bytes
	ldx #206
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 12980)
	; 24 bytes
	ldx #36 ;hides 'bit zp'
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 12981)
	; 22 bytes
	ldx #236
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 12984)
	; 24 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12986)
	; 21 bytes
	ldx #245
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.endif
.else
; 12988 to 13007
.if(n = 12988 || n = 12990 || n = 12992 || n = 12995 || n = 13002 || n = 13003 || n = 13007)
	; 22..24 bytes
	delay_n (n-2)
	nop
.elseif(n = 12989)
	; 23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 12991)
	; 23 bytes
	ldx #24 ;hides 'clc'
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 12993)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12994)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 12996)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12997)
	; 23 bytes
	ldx #228
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 12998)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12999)
	; 23 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13000)
	; 23 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13001)
	; 23 bytes
	ldx #250
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13004)
	; 22 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 13005)
	; 22 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 13006)
	; 21 bytes
	ldx #(n-1)/51
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.endif
.endif
.else
.if((n >= 13008) && (n < 13027))
; 13008 to 13026
.if(n = 13008 || (n > 13009 && n < 13014) || n = 13017 || n = 13019)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 13009)
	; 23 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13014)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13015)
	; 23 bytes
	ldx #241
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13016)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13018)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13020)
	; 26 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 13021)
	; 25 bytes
	ldx #210
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
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
	; 25 bytes
	delay_n 124
	delay_n (n-124)
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
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 13036)
	; 22 bytes
	ldx #237
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #246
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 13040)
	; 23 bytes
	ldx #221
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13042)
	; 23 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 13048)
	; 26 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13049)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13050)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13051)
	; 23 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13052)
	; 23 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13053)
	; 23 bytes
	ldx #251
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13054)
	; 23 bytes
	ldx #229
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13055)
	; 22 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13056)
	; 22 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 6
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 13057)
	; 21 bytes
	ldx #0
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 13058 || n = 13059 || (n > 13060 && n < 13065))
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 13060)
	; 23 bytes
	delay_n (n-3)
	ldx $A6
.endif
.else
; 13066 to 13085
.if(n = 13066)
	; 25 bytes
	ldx #201
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 23 bytes
	ldx #242
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13070)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 13071 || n = 13073)
	; 24..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 13072)
	; 25 bytes
	delay_n (n-3)
	ldx $A6
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
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 13080)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 13081)
	; 25 bytes
	ldx #218
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13082)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 13083)
	; 25 bytes
	ldx #211
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
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
.elseif(n = 13088)
	; 26 bytes
	delay_n 31
	delay_n (n-31)
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
	ldx #238
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 13092)
	; 21 bytes
	ldx #247
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 13093 || n = 13094 || (n > 13095 && n < 13099) || n = 13101 || n = 13106 || n = 13107 || n = 13113 || n = 13115 || n = 13118 || n = 13125)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 13095 || n = 13108 || n = 13114)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13099)
	; 23 bytes
	ldx #222
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	; 23 bytes
	ldx #234 ;hides 'nop'
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 13104)
	; 23 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13105)
	; 23 bytes
	ldx #252
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13109)
	; 24 bytes
	ldx #226
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 13110)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 13111)
	; 23 bytes
	ldx #230
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13112)
	; 26 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13116)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	; 23 bytes
	ldx #243
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
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
; 13126 to 13164
.if(n = 13126 || n = 13134 || n = 13148 || n = 13164)
	; 23..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13127 || n = 13142 || n = 13144 || n = 13147 || (n > 13148 && n < 13153) || n = 13154 || n = 13159 || n = 13160 || n = 13162 || n = 13163)
	; 22..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13128)
	; 26 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 13129)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 13130)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 13131)
	; 25 bytes
	ldx #202
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13132)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 13133)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
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
	; 24 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 13141)
	; 25 bytes
	ldx #219
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13143)
	; 26 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 13145)
	; 21 bytes
	ldx #248
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 13146)
	; 22 bytes
	ldx #239
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 13153)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13155)
	; 23 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13156)
	; 23 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13157)
	; 23 bytes
	ldx #253
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13158)
	; 23 bytes
	ldx #223
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13161)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.endif
.else
.if((n >= 13165) && (n < 13184))
; 13165 to 13183
.if(n = 13165 || n = 13170 || n = 13172 || n = 13174 || n = 13179)
	; 24..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13166)
	; 22 bytes
	ldx #232 ;hides 'inx'
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-19
.elseif(n = 13167)
	; 24 bytes
	ldx #227
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 13168)
	; 23 bytes
	ldx #231
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13169 || n = 13171 || n = 13180)
	; 24..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13173)
	; 26 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 13175)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 13176)
	; 26 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13177)
	; 23 bytes
	ldx #244
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13178)
	; 26 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 13181)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 13182)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 13183)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.endif
.else
; 13184 to 13203
.if(n = 13184)
	; 27 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 13185)
	; 26 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 13186)
	; 26 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 13187)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 13188)
	; 26 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 13189 || n = 13194 || n = 13199 || n = 13200 || n = 13202 || n = 13203)
	; 22..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13190)
	; 26 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13191)
	; 26 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13192)
	; 26 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 13193)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 13195)
	; 26 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13196)
	; 23 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13197)
	; 23 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13198)
	; 21 bytes
	ldx #249
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 13201)
	; 22 bytes
	ldx #240
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.endif
.endif
.endif
.else
.if((n >= 13204) && (n < 13243))
; 13204 to 13242
.if(n = 13204 || n = 13212 || n = 13220 || n = 13226 || n = 13228 || n = 13234)
	; 24..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13205 || n = 13206 || n = 13210 || n = 13211 || n = 13213 || n = 13219 || n = 13221 || n = 13222 || n = 13227 || n = 13229 || n = 13233 || n = 13235 || n = 13237 || n = 13240 || n = 13242)
	; 24..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13207)
	; 23 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13208)
	; 23 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13209)
	; 23 bytes
	ldx #254
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13214)
	; 26 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13215)
	; 26 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 13216)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 13217)
	; 23 bytes
	ldx #224
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13218)
	; 26 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 13223)
	; 22 bytes
	ldx #24 ;hides 'clc'
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-19
.elseif(n = 13224)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13225)
	; 23 bytes
	ldx #232
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13230)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13231)
	; 23 bytes
	ldx #245
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13232)
	; 27 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13236)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13238)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.elseif(n = 13247)
	; 27 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 13248)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13249)
	; 23 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13250)
	; 23 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13251)
	; 21 bytes
	ldx #250
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 13252 || n = 13253 || n = 13255 || n = 13257 || n = 13258 || n = 13262 || n = 13263 || n = 13265 || n = 13275 || n = 13277 || n = 13278 || n = 13280)
	; 22..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 13254 || n = 13264 || n = 13279)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13256)
	; 22 bytes
	ldx #241
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 13259)
	; 23 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13260)
	; 23 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13261)
	; 23 bytes
	ldx #255
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
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
.elseif(n = 13269 || n = 13270)
	; 25..26 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 13271)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 13272)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 13273)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13274)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 13276)
	; 23 bytes
	ldx #225
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
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
	ldx #233
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13283)
	; 24 bytes
	ldx #229
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 13284 || n = 13286 || n = 13287 || n = 13289 || n = 13293 || n = 13296 || n = 13301 || n = 13305 || n = 13306 || (n > 13307 && n < 13311) || n = 13313 || n = 13315 || n = 13317 || n = 13318)
	; 22..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13285)
	; 23 bytes
	ldx #246
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13288 || n = 13307 || n = 13314)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13290)
	; 26 bytes
	delay_n 124
	delay_n (n-124)
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
	ldx #211
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13295)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
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
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 13300)
	; 27 bytes
	delay_n 155
	delay_n (n-155)
.elseif(n = 13302)
	; 23 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13303)
	; 23 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13304)
	; 21 bytes
	ldx #251
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 13311)
	; 22 bytes
	ldx #242
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 13312)
	; 23 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
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
	; 25 bytes
	ldx #222
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13322)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
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
	ldx #205
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13327)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 13328 || n = 13331 || n = 13333)
	; 24..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13329)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13330)
	; 26 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13332 || n = 13338)
	; 25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13334)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 13335)
	; 23 bytes
	ldx #226
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13336)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13337)
	; 22 bytes
	ldx #234 ;hides 'nop'
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13339)
	; 23 bytes
	ldx #234
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.endif
.else
; 13340 to 13359
.if(n = 13340 || n = 13342)
	; 24..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13341 || n = 13343 || n = 13346 || n = 13358 || n = 13359)
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
	; 27 bytes
	delay_n 155
	delay_n (n-155)
.elseif(n = 13354)
	; 26 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13355)
	; 23 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13356)
	; 23 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13357)
	; 21 bytes
	ldx #252
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.endif
.endif
.endif
.else
.if((n >= 13360) && (n < 13399))
; 13360 to 13398
.if(n = 13360 || n = 13369 || n = 13388)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n > 13360 && n < 13365) || n = 13368 || (n > 13369 && n < 13373) || n = 13387 || n = 13389 || n = 13395 || n = 13398)
	; 23..25 bytes
	delay_n (n-2)
	nop
.elseif(n = 13365)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13366)
	; 22 bytes
	ldx #243
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 25 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 13377)
	; 26 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 13378)
	; 26 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 13379)
	; 26 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 13380)
	; 26 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 13381)
	; 25 bytes
	ldx #223
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
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
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13386)
	; 27 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 13390)
	; 26 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 13391)
	; 25 bytes
	ldx #206
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13392)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13393)
	; 23 bytes
	ldx #248
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13394)
	; 23 bytes
	ldx #227
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13396)
	; 23 bytes
	ldx #235
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13397)
	; 24 bytes
	ldx #232 ;hides 'inx'
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-21
.endif
.else
; 13399 to 13437
.if(n = 13399)
	; 24 bytes
	ldx #231
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 13400 || n = 13401 || n = 13411 || n = 13412 || (n > 13413 && n < 13418) || n = 13423 || (n > 13424 && n < 13428) || n = 13434)
	; 22..27 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 13406 || n = 13422)
	; 25..27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13407)
	; 26 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13408)
	; 23 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13409)
	; 23 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13410)
	; 21 bytes
	ldx #253
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 13413 || n = 13424)
	; 23..24 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13418 || n = 13429)
	; 25..26 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13419)
	; 25 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 13420)
	; 25 bytes
	ldx #213
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13421)
	; 22 bytes
	ldx #244
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 13428)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
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
; 13438 to 13476
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
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13442)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13443 || n = 13448 || n = 13449 || n = 13451 || n = 13452 || n = 13454 || n = 13459 || n = 13464 || n = 13465 || (n > 13466 && n < 13471) || n = 13472)
	; 22..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13444 || n = 13450 || n = 13456 || n = 13458 || n = 13466)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13445)
	; 24 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 13446)
	; 24 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 13447)
	; 23 bytes
	ldx #249
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13453)
	; 23 bytes
	ldx #228
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13455)
	; 23 bytes
	ldx #24 ;hides 'clc'
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13457)
	; 24 bytes
	ldx #232
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 13460)
	; 26 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13461)
	; 23 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13462)
	; 23 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13463)
	; 21 bytes
	ldx #254
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 13471)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13473)
	; 25 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13474)
	; 25 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13475)
	; 25 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13476)
	; 22 bytes
	ldx #245
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
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
	ldx $A6
.elseif(n = 13482)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 13483)
	; 25 bytes
	ldx #214
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13498)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13499 || n = 13502 || n = 13503 || n = 13505 || n = 13507)
	; 24..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13500)
	; 24 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 13501)
	; 23 bytes
	ldx #250
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13504 || n = 13513)
	; 25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13506)
	; 26 bytes
	delay_n 96
	delay_n (n-96)
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
	ldx #237
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13511)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 13512)
	; 23 bytes
	ldx #229
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13514)
	; 23 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13515)
	; 23 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.endif
.endif
.endif
.else
.if((n >= 13516) && (n < 13555))
; 13516 to 13554
.if(n = 13516)
	; 21 bytes
	ldx #255
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 13517 || n = 13518 || (n > 13519 && n < 13524) || n = 13525 || n = 13530 || n = 13533 || (n > 13534 && n < 13538) || n = 13540 || n = 13545 || n = 13548 || n = 13550)
	; 22..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13519 || n = 13534)
	; 23..24 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #246
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 13544)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13546)
	; 25 bytes
	ldx #215
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13547)
	; 26 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 13549)
	; 26 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 13551)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13552)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13553)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13554)
	; 24 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.endif
.else
; 13555 to 13593
.if(n = 13555)
	; 23 bytes
	ldx #251
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13556 || n = 13558 || n = 13572 || n = 13589)
	; 23..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13557 || n = 13559 || n = 13560 || n = 13570 || n = 13571 || (n > 13572 && n < 13577) || n = 13583 || n = 13588 || (n > 13589 && n < 13594))
	; 22..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13561)
	; 25 bytes
	ldx #226
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
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
	; 27 bytes
	delay_n 155
	delay_n (n-155)
.elseif(n = 13566)
	; 26 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13567)
	; 23 bytes
	ldx #238
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13568)
	; 23 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13569)
	; 21 bytes
	ldx #0
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-17
.elseif(n = 13577)
	; 25 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13578)
	; 24 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-21
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
	ldx #247
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 13587)
	; 25 bytes
	delay_n 124
	delay_n (n-124)
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
.elseif(n = 13603 || n = 13606 || n = 13610 || n = 13611)
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13604)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 13605)
	; 26 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13607)
	; 24 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 13608)
	; 24 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 13609)
	; 23 bytes
	ldx #252
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13612)
	; 25 bytes
	delay_n (n-3)
	ldx $A6
.endif
.else
; 13613 to 13632
.if(n = 13613 || n = 13623 || n = 13626 || n = 13632)
	; 24..26 bytes
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
	; 27 bytes
	delay_n 155
	delay_n (n-155)
.elseif(n = 13619)
	; 26 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13620)
	; 26 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 13621)
	; 25 bytes
	ldx #227
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13622)
	; 26 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 13624)
	; 23 bytes
	ldx #239
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13625)
	; 26 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 13627)
	; 25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13628)
	; 23 bytes
	ldx #232 ;hides 'inx'
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-20
.elseif(n = 13629)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 13630)
	; 23 bytes
	ldx #231
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13631)
	; 24 bytes
	ldx #235
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.endif
.endif
.else
; 13633 to 13671
.if(n = 13633 || n = 13644 || n = 13666 || n = 13668)
	; 24..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13634 || n = 13636 || n = 13642 || n = 13643 || (n > 13644 && n < 13649) || n = 13653 || n = 13656 || n = 13664 || n = 13667 || n = 13669 || n = 13671)
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
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13639)
	; 25 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 13640)
	; 24 bytes
	ldx #36 ;hides 'bit zp'
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 13641)
	; 22 bytes
	ldx #248
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 13649)
	; 26 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 13650)
	; 26 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13651)
	; 25 bytes
	ldx #210
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13652)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 13654)
	; 26 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13655 || n = 13658)
	; 26..28 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 13657)
	; 26 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 13659)
	; 26 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 13660)
	; 26 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13661)
	; 24 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 13662)
	; 24 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 13663)
	; 23 bytes
	ldx #253
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13665)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13670)
	; 26 bytes
	delay_n 101
	delay_n (n-101)
.endif
.endif
.else
.if((n >= 13672) && (n < 13711))
; 13672 to 13710
.if(n = 13672)
	; 25 bytes
	ldx #217
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13673 || n = 13706)
	; 26..27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13674 || n = 13676 || n = 13679 || n = 13683 || n = 13685 || n = 13686 || n = 13688 || n = 13691 || n = 13693 || n = 13697 || n = 13698 || n = 13700 || n = 13701 || n = 13704 || n = 13709)
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
	ldx #240
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13682)
	; 26 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 13684 || n = 13690 || n = 13692 || n = 13699)
	; 24..25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13687)
	; 23 bytes
	ldx #24 ;hides 'clc'
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13689)
	; 23 bytes
	ldx #232
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13694)
	; 23 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13695)
	; 23 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13696)
	; 22 bytes
	ldx #249
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 13702)
	; 24 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 13703)
	; 25 bytes
	ldx #221
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13705)
	; 26 bytes
	delay_n 136
	delay_n (n-136)
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
.elseif(n = 13714 || n = 13718 || n = 13719 || n = 13723 || n = 13725 || n = 13727 || n = 13728 || n = 13734 || n = 13737 || n = 13739 || n = 13740 || (n > 13741 && n < 13745))
	; 24..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 13715)
	; 24 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 13716)
	; 24 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 13717)
	; 23 bytes
	ldx #254
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13720 || n = 13724 || n = 13729 || n = 13741)
	; 25..26 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13721)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13722)
	; 26 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 13726)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
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
	ldx #199
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 13733)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13735)
	; 25 bytes
	ldx #218
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13736)
	; 26 bytes
	ldx #205
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 13738)
	; 23 bytes
	ldx #241
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 24 bytes
	ldx #237
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 13748)
	; 23 bytes
	ldx #233
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13749)
	; 23 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13750)
	; 23 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
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
	ldx #250
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 13752 || n = 13753 || (n > 13754 && n < 13759) || n = 13764 || n = 13767 || n = 13772 || n = 13773 || n = 13775 || n = 13776 || n = 13778 || n = 13779 || n = 13782 || n = 13783 || n = 13785 || n = 13789)
	; 23..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13754 || n = 13768 || n = 13774 || n = 13780 || n = 13784)
	; 24..27 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 13762 || n = 13788)
	; 26..27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13763)
	; 26 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13765)
	; 25 bytes
	ldx #222
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13766)
	; 27 bytes
	delay_n 197
	delay_n (n-197)
.elseif(n = 13769)
	; 24 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 13770)
	; 24 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 13771)
	; 23 bytes
	ldx #255
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13777)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13781)
	; 25 bytes
	ldx #212
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13786)
	; 28 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13787)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.endif
.else
; 13790 to 13828
.if(n = 13790 || n = 13809 || n = 13828)
	; 24..26 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13791 || n = 13793 || n = 13797 || n = 13799 || n = 13800 || n = 13808 || (n > 13809 && n < 13814) || n = 13815 || n = 13826 || n = 13827)
	; 23..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13792)
	; 27 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 13794)
	; 27 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 13795)
	; 23 bytes
	ldx #242
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13796)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 13798)
	; 25 bytes
	ldx #219
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13801)
	; 25 bytes
	ldx #230
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13802)
	; 27 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 13803)
	; 26 bytes
	ldx #206
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 13804)
	; 23 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13805)
	; 23 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 13806)
	; 22 bytes
	ldx #251
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 13807)
	; 23 bytes
	ldx #234
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13814)
	; 26 bytes
	delay_n (n-8)
	delay_n 8
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
	delay_n 124
	delay_n (n-124)
.elseif(n = 13821)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13822)
	; 27 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13823)
	; 24 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13824)
	; 24 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 6
	bit $24
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 13825)
	; 23 bytes
	ldx #0
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.endif
.endif
.else
.if((n >= 13829) && (n < 13868))
; 13829 to 13867
.if((n > 13828 && n < 13832) || n = 13835 || (n > 13836 && n < 13841) || n = 13850 || n = 13853 || n = 13854 || (n > 13855 && n < 13859) || n = 13862 || n = 13863 || n = 13865 || n = 13867)
	; 23..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13832)
	; 27 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13833)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13834)
	; 28 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 13836 || n = 13849 || n = 13851 || n = 13855 || n = 13864)
	; 24..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13841)
	; 27 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 13842)
	; 27 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13843)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13844)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13845)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13846)
	; 25 bytes
	ldx #213
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13847)
	; 27 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 13848)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 13852)
	; 23 bytes
	ldx #243
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13859)
	; 23 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13860)
	; 23 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13861)
	; 22 bytes
	ldx #252
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 13866)
	; 23 bytes
	ldx #235
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.endif
.else
.if((n >= 13868) && (n < 13887))
; 13868 to 13886
.if(n = 13868 || n = 13870 || n = 13872)
	; 24..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13869)
	; 25 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 13875)
	; 26 bytes
	delay_n 124
	delay_n (n-124)
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
	; 27 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13888)
	; 28 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 13889)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13890)
	; 28 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 13891 || (n > 13892 && n < 13897))
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13892)
	; 25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13897)
	; 27 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13898)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 13899)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13900)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13901)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
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
	ldx #244
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13910)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 13911 || n = 13913 || n = 13917 || n = 13918 || n = 13920 || n = 13922 || n = 13923 || n = 13926 || n = 13927 || (n > 13928 && n < 13932))
	; 23..26 bytes
	delay_n (n-2)
	nop
.elseif(n = 13912 || n = 13919 || n = 13928 || n = 13940)
	; 24..28 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13914)
	; 23 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13915)
	; 23 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13916)
	; 22 bytes
	ldx #253
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 13921)
	; 24 bytes
	ldx #240
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 13924)
	; 25 bytes
	ldx #221
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13925)
	; 23 bytes
	ldx #236
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13932)
	; 27 bytes
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
	ldx #208
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 13938)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 13939)
	; 27 bytes
	ldx #202
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 13941)
	; 27 bytes
	ldx #205
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 13942)
	; 27 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13943)
	; 25 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 13944)
	; 25 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 13945)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.endif
.else
; 13946 to 13984
.if(n = 13946 || n = 13947 || n = 13949 || n = 13950 || n = 13964 || n = 13968 || n = 13972 || n = 13973 || n = 13975 || n = 13977 || n = 13978 || n = 13980)
	; 23..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 13948 || n = 13954 || n = 13974)
	; 24..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 13951)
	; 25 bytes
	ldx #225
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13952 || n = 13961)
	; 27..28 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 13953)
	; 26 bytes
	ldx #218
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 13955)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13956 || n = 13982)
	; 26..27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 13957)
	; 27 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 13958)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 13959)
	; 27 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 13960 || n = 13967)
	; 27..28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 13962)
	; 27 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 13963)
	; 28 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 13965)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 13966)
	; 23 bytes
	ldx #245
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 13969)
	; 23 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13970)
	; 23 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 13971)
	; 22 bytes
	ldx #254
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 13976)
	; 25 bytes
	ldx #215
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13979)
	; 24 bytes
	ldx #241
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 13981)
	; 25 bytes
	ldx #233
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13983)
	; 26 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13984)
	; 23 bytes
	ldx #237
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.endif
.endif
.else
.if((n >= 13985) && (n < 14024))
; 13985 to 14023
.if(n = 13985)
	; 26 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 13986 || (n > 13987 && n < 13992) || n = 14002 || n = 14003 || (n > 14004 && n < 14008) || n = 14015 || (n > 14017 && n < 14021))
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 13987)
	; 25 bytes
	ldx #222
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 13992)
	; 27 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 13993)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 13994 || n = 14011)
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
.elseif(n = 13997 || n = 14010)
	; 27..28 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13998)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 13999)
	; 25 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14000)
	; 25 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14001)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14004)
	; 25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14008)
	; 27 bytes
	ldx #203
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14009)
	; 27 bytes
	ldx #206
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14012)
	; 27 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 14013)
	; 25 bytes
	ldx #226
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14014)
	; 27 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 14016)
	; 25 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14017)
	; 26 bytes
	ldx #219
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
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
	ldx #246
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.endif
.else
; 14024 to 14062
.if(n = 14024)
	; 23 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14025)
	; 23 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14026)
	; 22 bytes
	ldx #255
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 14027 || n = 14028 || n = 14030 || n = 14032 || n = 14033 || n = 14035 || n = 14045 || (n > 14046 && n < 14050) || n = 14052 || n = 14054 || n = 14058 || n = 14059 || n = 14061 || n = 14062)
	; 23..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14029 || n = 14034 || n = 14042 || n = 14044 || n = 14046 || n = 14053 || n = 14060)
	; 24..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14031)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14036)
	; 26 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14037)
	; 24 bytes
	ldx #242
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14038)
	; 26 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14039)
	; 24 bytes
	ldx #234 ;hides 'nop'
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14040)
	; 26 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 14041)
	; 25 bytes
	ldx #216
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14043)
	; 23 bytes
	ldx #238
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14050)
	; 25 bytes
	ldx #223
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14051)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14055)
	; 25 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14056)
	; 25 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14057)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.endif
.endif
.endif
.endif
.else
.if((n >= 14063) && (n < 14219))
.if((n >= 14063) && (n < 14141))
.if((n >= 14063) && (n < 14102))
; 14063 to 14101
.if(n = 14063 || n = 14064 || n = 14073 || n = 14077 || n = 14082 || n = 14083 || (n > 14084 && n < 14089) || n = 14094 || n = 14096 || n = 14097 || n = 14099 || n = 14100)
	; 23..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14065 || n = 14089)
	; 26..27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14066)
	; 28 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14067)
	; 27 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 14068)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
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
	ldx #210
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 25 bytes
	ldx #227
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14076)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14078 || n = 14084 || n = 14098)
	; 24..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14079)
	; 23 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14080)
	; 23 bytes
	ldx #247
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14081)
	; 22 bytes
	ldx #0
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-18
.elseif(n = 14090)
	; 24 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-21
.elseif(n = 14091)
	; 26 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14092)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14093)
	; 26 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14095)
	; 24 bytes
	ldx #243
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14101)
	; 25 bytes
	ldx #235
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.endif
.else
; 14102 to 14140
.if(n = 14102)
	; 23 bytes
	ldx #239
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14103 || n = 14104 || n = 14107 || n = 14108 || n = 14110 || n = 14114 || n = 14115 || (n > 14116 && n < 14121) || n = 14139)
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14105 || n = 14109 || n = 14116 || n = 14140)
	; 25..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14106)
	; 25 bytes
	ldx #217
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14111)
	; 25 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14112)
	; 25 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14113)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14121)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14122)
	; 27 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 14123)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14124)
	; 27 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 14125)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14126 || n = 14127)
	; 27..28 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14128)
	; 28 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14129)
	; 28 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 14130)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
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
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14136)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 14137)
	; 23 bytes
	ldx #248
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14138)
	; 26 bytes
	ldx #211
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.endif
.endif
.else
.if((n >= 14141) && (n < 14180))
; 14141 to 14179
.if((n > 14140 && n < 14144) || n = 14146 || n = 14155 || (n > 14156 && n < 14160) || n = 14163 || n = 14165 || n = 14166 || n = 14170 || n = 14171 || (n > 14172 && n < 14176) || n = 14178)
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14144)
	; 25 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14145)
	; 26 bytes
	ldx #221
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14147)
	; 27 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 14148)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14149)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14150)
	; 26 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 14151)
	; 23 bytes
	ldx #24 ;hides 'clc'
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14152)
	; 27 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 14153)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14154 || n = 14156 || n = 14164 || n = 14172 || n = 14179)
	; 25..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14160)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 14161)
	; 23 bytes
	ldx #240
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14162)
	; 27 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14167)
	; 25 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14168)
	; 25 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14169)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14176)
	; 25 bytes
	ldx #225
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14177)
	; 27 bytes
	delay_n 96
	delay_n (n-96)
.endif
.else
; 14180 to 14218
.if(n = 14180 || n = 14195 || n = 14196 || n = 14198 || n = 14200 || n = 14201 || n = 14203 || n = 14213 || n = 14215 || n = 14216 || n = 14218)
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14181 || n = 14206)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14182)
	; 27 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 14183)
	; 28 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14184)
	; 28 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14185 || n = 14210)
	; 28 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14186 || n = 14187 || n = 14208)
	; 27..28 bytes
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
	; 28 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 14191)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14192)
	; 23 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14193)
	; 23 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14194)
	; 23 bytes
	ldx #249
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14197 || n = 14217)
	; 25..26 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14199)
	; 25 bytes
	ldx #229
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14202)
	; 27 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 14204)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14205)
	; 26 bytes
	ldx #212
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14207)
	; 27 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 14209)
	; 26 bytes
	ldx #222
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14211)
	; 24 bytes
	ldx #245
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14212)
	; 27 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 14214)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.endif
.endif
.endif
.else
.if((n >= 14219) && (n < 14297))
.if((n >= 14219) && (n < 14258))
; 14219 to 14257
.if(n = 14219 || n = 14222 || n = 14224 || n = 14226 || n = 14227 || (n > 14228 && n < 14232) || n = 14238 || n = 14240 || n = 14241 || n = 14243 || n = 14244 || n = 14252 || n = 14253 || (n > 14254 && n < 14258))
	; 24..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14220)
	; 23 bytes
	ldx #241
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14221)
	; 25 bytes
	ldx #237
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14223 || n = 14228 || n = 14242 || n = 14254)
	; 25..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14225)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14232)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14233)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14234)
	; 27 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 14235)
	; 26 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 14236)
	; 25 bytes
	ldx #219
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14237)
	; 27 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 14239)
	; 25 bytes
	ldx #226
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14245)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 14246)
	; 28 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14247)
	; 27 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 14248)
	; 27 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 14249)
	; 23 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14250)
	; 23 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14251)
	; 23 bytes
	ldx #250
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.endif
.else
; 14258 to 14296
.if(n = 14258 || n = 14263 || n = 14265 || n = 14271 || n = 14274 || n = 14277 || n = 14283 || (n > 14284 && n < 14289))
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14259)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14260 || n = 14289)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14261)
	; 25 bytes
	ldx #230
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14262)
	; 27 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 14264 || n = 14276 || n = 14278 || n = 14282 || n = 14284)
	; 25..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14266)
	; 28 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14267)
	; 27 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 14268)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 14269)
	; 24 bytes
	ldx #246
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14270 || n = 14296)
	; 28 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 14272)
	; 26 bytes
	ldx #213
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14273)
	; 23 bytes
	ldx #234 ;hides 'nop'
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14275)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14279)
	; 23 bytes
	ldx #242
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14280)
	; 25 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14281)
	; 23 bytes
	ldx #(n-1)/56
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14290 || n = 14292)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14291 || n = 14293)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14294)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 14295)
	; 28 bytes
	delay_n 16
	delay_n (n-16)
.endif
.endif
.else
.if((n >= 14297) && (n < 14336))
; 14297 to 14335
.if(n = 14297)
	; 28 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 14298)
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 14299)
	; 27 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 14300)
	; 26 bytes
	ldx #36 ;hides 'bit zp'
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 14301)
	; 25 bytes
	ldx #220
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14302)
	; 25 bytes
	ldx #227
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14303 || n = 14304 || n = 14309 || n = 14310 || (n > 14311 && n < 14316) || n = 14325 || n = 14329 || (n > 14330 && n < 14334))
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14305 || n = 14311 || n = 14324 || n = 14326 || n = 14330)
	; 25..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14306)
	; 23 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14307)
	; 23 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14308)
	; 23 bytes
	ldx #251
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14316)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14317 || n = 14318)
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
	; 25 bytes
	ldx #232 ;hides 'inx'
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-22
.elseif(n = 14322)
	; 28 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 14323)
	; 25 bytes
	ldx #231
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14327)
	; 24 bytes
	ldx #247
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14328)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 14334)
	; 28 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14335)
	; 25 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	.repeat 2
	 rol $26
	 ror $26
	.endrepeat
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.endif
.else
; 14336 to 14375
.if(n = 14336)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14337)
	; 23 bytes
	ldx #0
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14338)
	; 23 bytes
	ldx #243
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14339 || n = 14340 || (n > 14341 && n < 14346) || n = 14355 || n = 14366 || n = 14367 || (n > 14368 && n < 14373))
	; 24..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14341 || n = 14368)
	; 25 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14346 || n = 14373)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14347 || n = 14348 || n = 14374 || n = 14375)
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
	ldx #208
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 14358 || n = 14359)
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
	; 23 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14364)
	; 23 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14365)
	; 23 bytes
	ldx #252
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
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
; 14376 to 14414
.if(n = 14376)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14377 || n = 14409)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14378 || n = 14387 || (n > 14388 && n < 14392) || n = 14399 || (n > 14400 && n < 14405) || n = 14408)
	; 24..28 bytes
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
.elseif(n = 14381 || n = 14412)
	; 28 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 14382)
	; 28 bytes
	ldx #197
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 14383)
	; 25 bytes
	ldx #24 ;hides 'clc'
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14384)
	; 28 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 14385)
	; 24 bytes
	ldx #248
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14386 || n = 14388 || n = 14400)
	; 25..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14392)
	; 28 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 14393 || n = 14394)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 14395)
	; 28 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 14396)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 14397)
	; 23 bytes
	ldx #244
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14398)
	; 28 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 14405)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14406)
	; 26 bytes
	ldx #215
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14407)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14410)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 14411)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 14413)
	; 28 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14414)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.endif
.else
; 14415 to 14453
.if(n = 14415)
	; 28 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 14416)
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 14417)
	; 27 bytes
	ldx #212
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14418)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 14419)
	; 28 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 14420)
	; 23 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14421)
	; 23 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14422)
	; 23 bytes
	ldx #253
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14423 || n = 14424 || n = 14426 || n = 14427 || n = 14429 || n = 14430 || n = 14432 || n = 14433 || n = 14435 || n = 14436 || n = 14444 || n = 14445 || n = 14448 || n = 14449 || n = 14451)
	; 24..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14425 || n = 14434 || n = 14446 || n = 14450)
	; 25..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14428)
	; 25 bytes
	ldx #229
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14431)
	; 25 bytes
	ldx #222
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
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
	; 25 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14442)
	; 25 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14443)
	; 24 bytes
	ldx #249
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14447)
	; 25 bytes
	ldx #233
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14452)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 14453)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.endif
.endif
.else
.if((n >= 14454) && (n < 14493))
; 14454 to 14492
.if(n = 14454)
	; 26 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 14455)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14456)
	; 23 bytes
	ldx #245
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14457 || n = 14462 || n = 14463 || n = 14475 || n = 14480 || n = 14481 || (n > 14482 && n < 14487))
	; 24..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14458)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14459 || n = 14476 || n = 14482)
	; 25..28 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14460)
	; 25 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 14461)
	; 25 bytes
	ldx #241
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14464 || n = 14487)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14465)
	; 26 bytes
	ldx #226
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14466 || n = 14488 || n = 14489)
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
	; 28 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14472)
	; 28 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14473)
	; 26 bytes
	ldx #216
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14474)
	; 28 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 14477)
	; 23 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14478)
	; 23 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14479)
	; 23 bytes
	ldx #254
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14491)
	; 25 bytes
	ldx #230
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14492)
	; 28 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 14493 to 14531
.if(n = 14493 || n = 14495 || n = 14498 || n = 14502 || n = 14503 || n = 14505 || n = 14506 || n = 14511 || n = 14513 || n = 14517 || n = 14520 || n = 14521 || n = 14523 || n = 14524 || n = 14531)
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14494 || n = 14504 || n = 14510 || n = 14512 || n = 14518 || n = 14522)
	; 25..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14496)
	; 25 bytes
	ldx #223
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14497)
	; 28 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 14499)
	; 25 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14500)
	; 25 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14501)
	; 24 bytes
	ldx #250
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14507)
	; 25 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14508)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 14509)
	; 25 bytes
	ldx #234
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14514)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 14515)
	; 23 bytes
	ldx #246
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14516)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14519)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.elseif(n = 14528)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 14529)
	; 26 bytes
	ldx #227
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14530)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.endif
.endif
.endif
.else
.if((n >= 14532) && (n < 14610))
.if((n >= 14532) && (n < 14571))
; 14532 to 14570
.if(n = 14532)
	; 28 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 14533)
	; 28 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 14534)
	; 23 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14535)
	; 23 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14536)
	; 23 bytes
	ldx #255
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14537 || n = 14538 || (n > 14539 && n < 14544) || n = 14549 || n = 14556 || n = 14560 || n = 14563 || n = 14565)
	; 24..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14539 || n = 14555 || n = 14562 || n = 14564)
	; 25..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14544)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14545 || n = 14546 || n = 14569)
	; 27..28 bytes
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
.elseif(n = 14550)
	; 28 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 14551)
	; 28 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14552)
	; 24 bytes
	ldx #232 ;hides 'inx'
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	dex
	Jne *-21
.elseif(n = 14553)
	; 27 bytes
	ldx #214
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14554)
	; 25 bytes
	ldx #231
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14557)
	; 25 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14558)
	; 25 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14559)
	; 24 bytes
	ldx #251
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14561)
	; 25 bytes
	ldx #224
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.else
; 14571 to 14609
.if(n = 14571)
	; 25 bytes
	ldx #235
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14572)
	; 28 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 14573 || n = 14575 || n = 14576 || n = 14578 || n = 14579 || n = 14582 || n = 14584 || n = 14585 || n = 14588 || n = 14594 || n = 14595 || (n > 14596 && n < 14601) || n = 14609)
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14574)
	; 23 bytes
	ldx #247
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14577 || n = 14583 || n = 14596)
	; 25..26 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14580)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14581)
	; 25 bytes
	ldx #243
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14586)
	; 26 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 14587)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14589)
	; 28 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 14590)
	; 28 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14591)
	; 23 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14592)
	; 23 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-20
.elseif(n = 14593)
	; 23 bytes
	ldx #0
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14601)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14602 || n = 14603)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14604)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14605)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14606)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 14607)
	; 26 bytes
	ldx #218
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #24 ;hides 'clc'
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 14616)
	; 25 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14617)
	; 24 bytes
	ldx #252
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14618 || n = 14620 || n = 14636 || n = 14644)
	; 25..26 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14619 || (n > 14620 && n < 14624) || n = 14628 || n = 14630 || n = 14631 || n = 14635 || (n > 14636 && n < 14641) || n = 14643 || (n > 14644 && n < 14649))
	; 24..28 bytes
	delay_n (n-2)
	nop
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
	ldx #225
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14627)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14629)
	; 27 bytes
	ldx #212
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14632)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 14633)
	; 23 bytes
	ldx #248
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14634)
	; 28 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 14641)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14642)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
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
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
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
	; 26 bytes
	ldx #229
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14658 || n = 14660)
	; 27 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 14659)
	; 27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14661)
	; 28 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 14662)
	; 28 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 14663)
	; 28 bytes
	delay_n 71
	delay_n (n-71)
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
.elseif(n = 14673)
	; 25 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14674)
	; 25 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14675)
	; 24 bytes
	ldx #253
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14676 || n = 14677 || n = 14679 || n = 14681 || n = 14682 || n = 14684 || n = 14685)
	; 25..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14678 || n = 14683)
	; 26..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14680)
	; 25 bytes
	ldx #233
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
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
; 14688 to 14726
.if(n = 14688)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14689)
	; 27 bytes
	ldx #216
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14690)
	; 24 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 14691)
	; 24 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 14692)
	; 23 bytes
	ldx #249
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14693 || n = 14694 || (n > 14695 && n < 14700) || n = 14703 || n = 14704 || (n > 14705 && n < 14709) || n = 14723)
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14695 || n = 14705)
	; 25..26 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14700)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14701)
	; 25 bytes
	ldx #245
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14702)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
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
.elseif((n > 14714 && n < 14718))
	; 27 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 14718)
	; 28 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14719)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14720)
	; 28 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 14721)
	; 26 bytes
	ldx #230
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14722)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
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
	delay_n 191
	delay_n (n-191)
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
.elseif(n = 14731)
	; 25 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14732)
	; 25 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14733)
	; 24 bytes
	ldx #254
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14734 || n = 14735 || (n > 14736 && n < 14740) || n = 14745 || n = 14747 || n = 14748 || n = 14752 || n = 14753 || n = 14755 || (n > 14757 && n < 14761) || n = 14765)
	; 24..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 14736 || n = 14744 || n = 14746 || n = 14754 || n = 14764)
	; 25..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14740)
	; 27 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 4
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 14741)
	; 24 bytes
	ldx #234 ;hides 'nop'
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14742)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14743)
	; 25 bytes
	ldx #234
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14749)
	; 24 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 14750)
	; 24 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 14751)
	; 23 bytes
	ldx #250
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14756)
	; 25 bytes
	ldx #227
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14757)
	; 25 bytes
	ldx #238
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14761)
	; 25 bytes
	ldx #246
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14762)
	; 27 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14763)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.endif
.endif
.else
.if((n >= 14766) && (n < 14805))
.if((n >= 14766) && (n < 14785))
; 14766 to 14784
.if(n = 14766)
	; 26 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14767 || n = 14768 || n = 14771 || n = 14781)
	; 26..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14769)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	ldx #197
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 26 bytes
	ldx #232 ;hides 'inx'
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-23
.elseif(n = 14784)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.endif
.else
; 14785 to 14804
.if(n = 14785)
	; 26 bytes
	ldx #231
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14786)
	; 28 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 14787 || n = 14792 || n = 14793 || (n > 14794 && n < 14798))
	; 25..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14788)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14789)
	; 25 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14790)
	; 25 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14791)
	; 24 bytes
	ldx #255
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14794)
	; 26 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14798)
	; 28 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14799)
	; 28 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 14800)
	; 28 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 14801)
	; 28 bytes
	delay_n 10
	delay_n (n-10)
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
.if(n = 14805 || n = 14811 || n = 14812 || (n > 14813 && n < 14818) || n = 14823 || n = 14825 || n = 14826 || (n > 14827 && n < 14831) || n = 14838)
	; 24..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14806)
	; 25 bytes
	ldx #235
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14807)
	; 27 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 14808)
	; 24 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 14809)
	; 24 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 14810)
	; 23 bytes
	ldx #251
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14813 || n = 14827 || n = 14839)
	; 25..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14818)
	; 27 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14819)
	; 25 bytes
	ldx #239
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14820 || n = 14834)
	; 27..28 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14821)
	; 25 bytes
	ldx #228
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14822)
	; 27 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14824)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	ldx #215
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14837)
	; 28 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 14840)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
; 14844 to 14882
.if(n = 14844 || n = 14850 || n = 14851 || (n > 14852 && n < 14856) || n = 14864 || n = 14870 || n = 14871 || n = 14873 || n = 14874 || n = 14876 || n = 14877)
	; 24..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14845)
	; 28 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 14846)
	; 28 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 14847)
	; 25 bytes
	ldx #24 ;hides 'clc'
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14848)
	; 25 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 10
	.repeat 3
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14849)
	; 24 bytes
	ldx #0
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14852 || n = 14872)
	; 25..26 bytes
	delay_n (n-3)
	ldx $A6
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
	; 28 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 14860 || n = 14880)
	; 27..28 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 14861)
	; 28 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 14862)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 14863)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 14865)
	; 28 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 14866)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 14867)
	; 24 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 14868)
	; 24 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 14869)
	; 23 bytes
	ldx #252
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14875)
	; 26 bytes
	ldx #222
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14878)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 14879)
	; 27 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14881)
	; 25 bytes
	ldx #240
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14882)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.endif
.else
; 14883 to 14921
.if(n = 14883 || n = 14887 || (n > 14888 && n < 14892) || n = 14894 || n = 14899 || n = 14903 || n = 14907 || n = 14909 || n = 14915 || n = 14919)
	; 25..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14884 || n = 14888 || n = 14908 || n = 14914)
	; 26..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14885)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14886)
	; 25 bytes
	ldx #229
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14892)
	; 26 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 14893)
	; 27 bytes
	ldx #219
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
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
.elseif(n = 14902)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 14904)
	; 28 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 14905)
	; 27 bytes
	ldx #216
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14906)
	; 28 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 14910)
	; 28 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 14911)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14912)
	; 28 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 14913)
	; 26 bytes
	ldx #233
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14916)
	; 28 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 14917)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14918)
	; 28 bytes
	delay_n 167
	delay_n (n-167)
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
.else
.if((n >= 14922) && (n < 14961))
; 14922 to 14960
.if(n = 14922)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 14923 || n = 14924 || n = 14929 || n = 14930 || n = 14933 || n = 14934 || n = 14936 || n = 14937 || n = 14944 || n = 14945 || n = 14947 || n = 14948 || n = 14950 || n = 14952 || n = 14953 || n = 14955 || n = 14957)
	; 24..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 14925)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 14926)
	; 24 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 14927)
	; 24 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 14928)
	; 23 bytes
	ldx #253
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14931 || n = 14935 || n = 14949 || n = 14954)
	; 25..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14932)
	; 25 bytes
	ldx #237
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14938 || n = 14956)
	; 27..28 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14939)
	; 25 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14940)
	; 25 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 14941)
	; 25 bytes
	ldx #249
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14942)
	; 26 bytes
	ldx #223
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14943)
	; 25 bytes
	ldx #241
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14946)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 14951)
	; 25 bytes
	ldx #230
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14958)
	; 28 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14959)
	; 27 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 14960)
	; 26 bytes
	ldx #36 ;hides 'bit zp'
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-23
.endif
.else
.if((n >= 14961) && (n < 14981))
; 14961 to 14980
.if(n = 14961)
	; 27 bytes
	ldx #220
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14962 || n = 14968 || n = 14976 || n = 14979)
	; 27..29 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 14973)
	; 28 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 14974)
	; 27 bytes
	ldx #217
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14975)
	; 25 bytes
	ldx #234 ;hides 'nop'
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14977)
	; 26 bytes
	ldx #234
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 14978)
	; 27 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14983)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 14984)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 14985)
	; 24 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 14986)
	; 24 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 14987)
	; 23 bytes
	ldx #254
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 14988 || n = 14989 || (n > 14990 && n < 14995) || n = 14997)
	; 24..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 14990 || n = 14998)
	; 25..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 14995)
	; 25 bytes
	ldx #238
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 14996)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 14999)
	; 25 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15000)
	; 25 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
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
	; 25 bytes
	ldx #250
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15002 || n = 15003 || n = 15009 || (n > 15010 && n < 15014) || n = 15018 || (n > 15019 && n < 15023) || n = 15027 || (n > 15029 && n < 15033))
	; 25..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15004 || n = 15008 || n = 15010 || n = 15017 || n = 15019)
	; 26..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15005)
	; 25 bytes
	ldx #242
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15006)
	; 28 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 15007)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 15014)
	; 25 bytes
	ldx #232 ;hides 'inx'
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	dex
	Jne *-22
.elseif(n = 15015)
	; 28 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 15016)
	; 25 bytes
	ldx #231
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
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
.elseif(n = 15026)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15028)
	; 26 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 15029)
	; 27 bytes
	ldx #221
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
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
	; 28 bytes
	delay_n 167
	delay_n (n-167)
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
	; 26 bytes
	ldx #235
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15042 || n = 15047 || n = 15048 || (n > 15049 && n < 15053) || n = 15062 || n = 15063 || n = 15065 || n = 15066 || n = 15069 || n = 15070 || (n > 15071 && n < 15076) || n = 15078)
	; 24..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15043)
	; 27 bytes
	ldx #218
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15044)
	; 24 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 15045)
	; 24 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 15046)
	; 23 bytes
	ldx #255
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 15049 || n = 15064 || n = 15071)
	; 25..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15053)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	ldx #239
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15059)
	; 25 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15060)
	; 25 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15061)
	; 25 bytes
	ldx #251
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15067)
	; 25 bytes
	ldx #243
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15068)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 15076)
	; 26 bytes
	ldx #225
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #24 ;hides 'clc'
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15080 || n = 15117)
	; 27..28 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15081)
	; 25 bytes
	ldx #232
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15082 || n = 15084 || n = 15108)
	; 25..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15083 || n = 15085 || n = 15086 || n = 15090 || n = 15100 || n = 15101 || n = 15106 || n = 15107 || (n > 15108 && n < 15112) || n = 15114)
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
	; 28 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 15096)
	; 29 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 15097)
	; 27 bytes
	ldx #222
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15098)
	; 28 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 15099)
	; 28 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 15102)
	; 28 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15103)
	; 24 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15104)
	; 24 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-21
.elseif(n = 15105)
	; 23 bytes
	ldx #0
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-19
.elseif(n = 15112)
	; 27 bytes
	ldx #219
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	; 25 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15120)
	; 25 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15121)
	; 25 bytes
	ldx #240
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15122 || n = 15123 || n = 15125 || n = 15130 || n = 15131 || (n > 15132 && n < 15136))
	; 25..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15124 || n = 15132)
	; 26..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15126)
	; 28 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15127)
	; 28 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15128)
	; 28 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15129)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 15136)
	; 28 bytes
	delay_n 31
	delay_n (n-31)
.endif
.else
; 15137 to 15156
.if(n = 15137)
	; 28 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15138)
	; 28 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15139)
	; 28 bytes
	delay_n 10
	delay_n (n-10)
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
	ldx #226
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15144 || n = 15145 || n = 15148 || n = 15150)
	; 26..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15146)
	; 25 bytes
	ldx #233
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15147)
	; 28 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15149)
	; 27 bytes
	delay_n (n-3)
	ldx $A6
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
	; 28 bytes
	delay_n 167
	delay_n (n-167)
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
.elseif(n = 15159 || n = 15164 || n = 15171 || n = 15174)
	; 27..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15160)
	; 29 bytes
	delay_n 31
	delay_n (n-31)
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
	; 27 bytes
	ldx #223
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
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
	; 26 bytes
	ldx #237
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15170)
	; 27 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 15172)
	; 28 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 15173)
	; 28 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 15175)
	; 29 bytes
	delay_n 46
	delay_n (n-46)
.endif
.else
; 15176 to 15195
.if(n = 15176)
	; 28 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 15177)
	; 28 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 15178)
	; 28 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 15179)
	; 25 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15180)
	; 25 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15181)
	; 25 bytes
	ldx #253
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15182 || n = 15183 || n = 15185 || n = 15186 || n = 15192 || n = 15194)
	; 25..27 bytes
	delay_n (n-2)
	nop
.elseif(n = 15184)
	; 25 bytes
	ldx #241
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15187 || n = 15193)
	; 26..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15188)
	; 25 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15189)
	; 25 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15190)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 15191)
	; 25 bytes
	ldx #245
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15195)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.endif
.endif
.else
.if((n >= 15196) && (n < 15215))
; 15196 to 15214
.if(n = 15196 || n = 15203 || n = 15208 || n = 15212 || n = 15213)
	; 26..29 bytes
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
.elseif(n = 15204)
	; 29 bytes
	delay_n 99
	delay_n (n-99)
.elseif(n = 15205)
	; 29 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 15206)
	; 28 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15207)
	; 28 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 15209)
	; 25 bytes
	ldx #234 ;hides 'nop'
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15210)
	; 26 bytes
	ldx #227
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15211)
	; 25 bytes
	ldx #234
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15214)
	; 27 bytes
	delay_n (n-3)
	ldx $A6
.endif
.else
; 15215 to 15234
.if(n = 15215 || n = 15218 || n = 15219 || n = 15224 || n = 15234)
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
	; 29 bytes
	delay_n 12
	delay_n (n-12)
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
.elseif(n = 15228 || n = 15229)
	; 27..28 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 15230)
	; 29 bytes
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
	; 26 bytes
	ldx #238
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.endif
.endif
.endif
.else
.if((n >= 15235) && (n < 15274))
; 15235 to 15273
.if(n = 15235 || n = 15238 || n = 15242 || n = 15243 || n = 15252 || n = 15253 || (n > 15254 && n < 15258) || n = 15268 || n = 15273)
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
.elseif(n = 15239)
	; 25 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15240)
	; 25 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15241)
	; 25 bytes
	ldx #254
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15244 || n = 15248 || n = 15254 || n = 15269)
	; 26..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15245)
	; 26 bytes
	ldx #232 ;hides 'inx'
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	dex
	Jne *-23
.elseif(n = 15246 || n = 15270)
	; 28..29 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15247)
	; 25 bytes
	ldx #242
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15249)
	; 25 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15250)
	; 25 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15251)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 15258)
	; 28 bytes
	ldx #209
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.elseif(n = 15264)
	; 29 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 15265)
	; 29 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 15266)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	ldx #235
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15277)
	; 26 bytes
	ldx #228
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15278 || n = 15280 || n = 15282 || n = 15283)
	; 26..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15279)
	; 27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15281)
	; 28 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 15284)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15285)
	; 29 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15286)
	; 28 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 15287)
	; 29 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15288)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15289 || n = 15291)
	; 28..29 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 15290)
	; 29 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 15292)
	; 29 bytes
	delay_n 41
	delay_n (n-41)
.endif
.else
; 15293 to 15312
.if(n = 15293 || n = 15298 || n = 15302 || n = 15303 || (n > 15304 && n < 15308))
	; 26..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15294 || n = 15296)
	; 28..29 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 15295)
	; 29 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 15297)
	; 26 bytes
	ldx #239
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15299)
	; 25 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15300)
	; 25 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15301)
	; 25 bytes
	ldx #255
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15304)
	; 27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15308)
	; 29 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 15309)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15310)
	; 25 bytes
	ldx #243
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15311)
	; 25 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15312)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.if(n = 15313 || n = 15314 || (n > 15315 && n < 15319) || n = 15321 || n = 15329 || n = 15332 || n = 15334 || n = 15339 || n = 15343 || n = 15345 || n = 15346 || (n > 15347 && n < 15351))
	; 25..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15315)
	; 25 bytes
	ldx #247
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15319)
	; 27 bytes
	ldx #222
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.elseif(n = 15330)
	; 27 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 15331)
	; 28 bytes
	ldx #210
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 15333)
	; 29 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15335)
	; 29 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 15336)
	; 29 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 15337)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15338)
	; 29 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15340 || n = 15347)
	; 28..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15341)
	; 25 bytes
	ldx #236
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15342)
	; 29 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15344)
	; 26 bytes
	ldx #229
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.endif
.else
; 15352 to 15390
.if(n = 15352)
	; 29 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15353 || n = 15385)
	; 28..29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15354 || n = 15370 || n = 15383)
	; 28..30 bytes
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
	; 25 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15360)
	; 25 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15361)
	; 25 bytes
	ldx #0
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15362 || n = 15363 || (n > 15364 && n < 15368) || n = 15374 || n = 15375 || n = 15378 || n = 15379 || n = 15381 || n = 15382 || n = 15390)
	; 25..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 15364 || n = 15376 || n = 15380)
	; 26..27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15368)
	; 29 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15369)
	; 27 bytes
	ldx #226
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15371)
	; 25 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15372)
	; 25 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15373)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 15377)
	; 25 bytes
	ldx #248
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15384)
	; 28 bytes
	delay_n 11
	delay_n (n-11)
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
	ldx #223
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15389)
	; 29 bytes
	delay_n 16
	delay_n (n-16)
.endif
.endif
.else
.if((n >= 15391) && (n < 15430))
; 15391 to 15429
.if(n = 15391 || n = 15409 || n = 15414 || n = 15428)
	; 27..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15392 || n = 15402 || n = 15405 || n = 15408 || n = 15410 || n = 15412 || n = 15413 || n = 15415 || n = 15420 || n = 15427 || n = 15429)
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
	; 27 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 15400)
	; 27 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 4
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 15401)
	; 28 bytes
	ldx #220
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 15403)
	; 29 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15404)
	; 28 bytes
	ldx #211
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 15406)
	; 25 bytes
	ldx #237
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15407)
	; 29 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15411)
	; 26 bytes
	ldx #230
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 29 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 15424)
	; 29 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 15425)
	; 26 bytes
	ldx #241
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15426)
	; 30 bytes
	delay_n 116
	delay_n (n-116)
.endif
.else
; 15430 to 15468
.if(n = 15430 || n = 15431 || n = 15435 || n = 15438 || n = 15440 || n = 15441 || n = 15447 || n = 15459 || n = 15461)
	; 26..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15432)
	; 25 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15433)
	; 25 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15434)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 15436)
	; 25 bytes
	ldx #245
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15437 || n = 15442 || n = 15460)
	; 26..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15439)
	; 25 bytes
	ldx #249
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15443)
	; 26 bytes
	ldx #234 ;hides 'nop'
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15444)
	; 28 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15445)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15446 || n = 15448)
	; 28..29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15449)
	; 29 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 15450 || n = 15452)
	; 29..30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15451)
	; 29 bytes
	ldx #206
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #224
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
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
.elseif(n = 15465)
	; 29 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15466 || n = 15468)
	; 29..30 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15467)
	; 29 bytes
	ldx #209
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
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
.elseif(n = 15470)
	; 27 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 15471)
	; 25 bytes
	ldx #238
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15472 || n = 15473 || n = 15475 || n = 15480 || n = 15482 || n = 15491 || n = 15496 || n = 15497 || n = 15503)
	; 25..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 15474 || n = 15481 || n = 15492 || n = 15498 || n = 15502 || n = 15504)
	; 26..28 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15476)
	; 26 bytes
	ldx #232 ;hides 'inx'
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-23
.elseif(n = 15477)
	; 28 bytes
	ldx #212
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 15478)
	; 26 bytes
	ldx #231
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15479)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15483 || n = 15507)
	; 28..29 bytes
	delay_n 12
	delay_n (n-12)
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
	; 26 bytes
	ldx #242
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15490)
	; 29 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15493)
	; 25 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15494)
	; 25 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15495)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 15499)
	; 25 bytes
	ldx #246
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15500)
	; 26 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 15501)
	; 25 bytes
	ldx #250
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15505)
	; 27 bytes
	ldx #228
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15506)
	; 28 bytes
	delay_n 11
	delay_n (n-11)
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
.elseif(n = 15510)
	; 29 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 15511)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15512)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
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
	ldx #225
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15529 || n = 15539 || n = 15546)
	; 27..29 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #239
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15543)
	; 25 bytes
	ldx #24 ;hides 'clc'
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15544)
	; 29 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 15545)
	; 26 bytes
	ldx #232
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.endif
.endif
.else
.if((n >= 15547) && (n < 15586))
; 15547 to 15585
.if(n = 15547 || n = 15549 || n = 15552 || n = 15557 || n = 15558 || n = 15560 || (n > 15563 && n < 15568) || n = 15569 || n = 15575 || n = 15579 || n = 15584)
	; 25..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15548 || n = 15559)
	; 26..28 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15550)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15551)
	; 29 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15553)
	; 26 bytes
	ldx #243
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15554)
	; 25 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15555)
	; 25 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15556)
	; 24 bytes
	ldx #(n-1)/61
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 15561)
	; 26 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 15562)
	; 25 bytes
	ldx #247
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15563)
	; 25 bytes
	ldx #251
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
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
	; 27 bytes
	ldx #229
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15576)
	; 29 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15577)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
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
.elseif(n = 15589 || n = 15594 || n = 15597 || n = 15599 || n = 15603 || (n > 15604 && n < 15609) || n = 15614 || n = 15618 || n = 15619 || n = 15621 || n = 15622)
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
	ldx #226
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15596)
	; 29 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15598 || n = 15604 || n = 15620)
	; 26..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15600)
	; 29 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 15601)
	; 25 bytes
	ldx #240
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 28 bytes
	ldx #223
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 15612)
	; 26 bytes
	ldx #233
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15613)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15615)
	; 25 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15616)
	; 25 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 4
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15617)
	; 24 bytes
	ldx #0
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-20
.elseif(n = 15623)
	; 26 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 15624)
	; 26 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 15625)
	; 25 bytes
	ldx #248
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
; 15626 to 15664
.if(n = 15626 || n = 15627 || (n > 15628 && n < 15632) || n = 15634 || n = 15639 || n = 15645 || n = 15649 || n = 15654 || n = 15659)
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15628 || n = 15644)
	; 27..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15632)
	; 29 bytes
	ldx #203
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 15633)
	; 29 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15635)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15636 || n = 15640)
	; 29..30 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15637)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15638)
	; 29 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15641)
	; 27 bytes
	ldx #230
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15642)
	; 29 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 15643)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15646 || n = 15650)
	; 29..30 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15647)
	; 29 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15648)
	; 29 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15651 || n = 15655)
	; 29..30 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15652)
	; 29 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15653)
	; 29 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15656)
	; 29 bytes
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
.elseif(n = 15660)
	; 29 bytes
	delay_n 124
	delay_n (n-124)
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
	ldx #227
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.endif
.else
; 15665 to 15703
.if(n = 15665)
	; 30 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 15666)
	; 25 bytes
	ldx #241
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15667 || n = 15669 || n = 15680 || n = 15682 || n = 15684 || n = 15691 || n = 15695)
	; 27..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15668 || (n > 15669 && n < 15673) || n = 15683 || n = 15689 || n = 15690 || n = 15693 || n = 15694)
	; 26..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 15673)
	; 29 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15674)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15675)
	; 30 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 15676)
	; 29 bytes
	ldx #209
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 15677)
	; 25 bytes
	ldx #234 ;hides 'nop'
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15678 || n = 15700)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15679)
	; 26 bytes
	ldx #234
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15681)
	; 26 bytes
	ldx #245
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15685)
	; 26 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 15686)
	; 25 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15687)
	; 25 bytes
	ldx #253
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15688)
	; 25 bytes
	ldx #249
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15692)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15696)
	; 28 bytes
	ldx #215
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.if((n >= 15704) && (n < 15723))
; 15704 to 15722
.if(n = 15704)
	; 30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15705 || n = 15711 || n = 15715 || n = 15720)
	; 28..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15706)
	; 30 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15707)
	; 27 bytes
	ldx #232 ;hides 'inx'
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	dex
	Jne *-24
.elseif(n = 15708 || n = 15716)
	; 29..30 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 15709)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15710)
	; 29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15712)
	; 29 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15713 || n = 15721)
	; 29..30 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15714)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15717)
	; 29 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 15718)
	; 29 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15719)
	; 30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15722)
	; 29 bytes
	delay_n 166
	delay_n (n-166)
.endif
.else
; 15723 to 15742
.if(n = 15723)
	; 29 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15724)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15725 || n = 15740 || n = 15741)
	; 28..29 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 15726)
	; 30 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15727)
	; 29 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 15728)
	; 29 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 15729)
	; 30 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 15730)
	; 29 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 15731)
	; 25 bytes
	ldx #242
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15732)
	; 29 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 15733 || (n > 15734 && n < 15738))
	; 26..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 15734)
	; 27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15738)
	; 29 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 15739)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15742)
	; 29 bytes
	delay_n 186
	delay_n (n-186)
.endif
.endif
.else
.if((n >= 15743) && (n < 15762))
; 15743 to 15761
.if(n = 15743)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15744)
	; 30 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15745)
	; 26 bytes
	ldx #246
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15746)
	; 26 bytes
	ldx #235
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15747)
	; 26 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 15748)
	; 26 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 15749)
	; 25 bytes
	ldx #254
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15750)
	; 25 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15751)
	; 25 bytes
	ldx #250
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15752 || n = 15753 || (n > 15754 && n < 15759))
	; 26..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15754)
	; 27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15759)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15760 || n = 15761)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 15762 to 15781
.if(n = 15762)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15763)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15764)
	; 30 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 15765 || n = 15767 || n = 15771 || n = 15779 || n = 15781)
	; 28..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15766 || n = 15780)
	; 29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15768)
	; 28 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 15769)
	; 28 bytes
	ldx #216
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 15770)
	; 29 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 15772)
	; 30 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15773)
	; 29 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15774)
	; 30 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 15775)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15776)
	; 30 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15777)
	; 27 bytes
	ldx #232
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15778)
	; 29 bytes
	delay_n 161
	delay_n (n-161)
.endif
.endif
.endif
.endif
.else
.if((n >= 15782) && (n < 15860))
.if((n >= 15782) && (n < 15821))
.if((n >= 15782) && (n < 15801))
; 15782 to 15800
.if(n = 15782)
	; 30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15783)
	; 29 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 15784)
	; 29 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 15785 || n = 15795 || n = 15798 || n = 15800)
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15786)
	; 29 bytes
	ldx #205
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 15787)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15788)
	; 29 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 15789)
	; 30 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15790)
	; 29 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 15791)
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
.elseif(n = 15794)
	; 30 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 15796)
	; 25 bytes
	ldx #243
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15797)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15799)
	; 27 bytes
	delay_n (n-3)
	ldx $A6
.endif
.else
; 15801 to 15820
.if(n = 15801 || n = 15804 || n = 15815 || n = 15816 || (n > 15817 && n < 15821))
	; 26..28 bytes
	delay_n (n-2)
	nop
.elseif(n = 15802)
	; 27 bytes
	ldx #229
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15803)
	; 29 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 15805 || n = 15817)
	; 27..29 bytes
	delay_n (n-3)
	ldx $A6
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
	; 26 bytes
	ldx #247
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15810)
	; 26 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 15811)
	; 25 bytes
	ldx #255
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15812)
	; 25 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15813)
	; 25 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15814)
	; 25 bytes
	ldx #251
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.endif
.endif
.else
.if((n >= 15821) && (n < 15840))
; 15821 to 15839
.if(n = 15821)
	; 28 bytes
	ldx #226
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 15822)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15823)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15824)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15825)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15826)
	; 29 bytes
	ldx #211
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 15827)
	; 30 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 15828 || n = 15836 || n = 15838)
	; 28..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15829)
	; 30 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15830)
	; 30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15831 || n = 15832)
	; 30 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15833)
	; 30 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15834)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15835)
	; 30 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15837)
	; 29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15839)
	; 28 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-25
.endif
.else
; 15840 to 15859
.if(n = 15840)
	; 28 bytes
	ldx #36 ;hides 'bit zp'
	nop
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 15841)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15842)
	; 28 bytes
	ldx #217
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 15843 || n = 15844 || n = 15846 || n = 15847 || n = 15849)
	; 28..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15845)
	; 27 bytes
	ldx #233
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15848)
	; 29 bytes
	delay_n (n-3)
	ldx $A6
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
	; 29 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 15856 || n = 15857)
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
	ldx #244
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15862)
	; 30 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 15863 || (n > 15864 && n < 15869) || n = 15874 || n = 15878 || n = 15879 || n = 15881 || n = 15882 || (n > 15883 && n < 15887) || n = 15893)
	; 26..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15864 || n = 15883)
	; 27..28 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15869)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15870)
	; 30 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15871)
	; 26 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15872)
	; 26 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 15873)
	; 25 bytes
	ldx #0
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15875)
	; 25 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15876)
	; 25 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15877)
	; 25 bytes
	ldx #252
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15880)
	; 26 bytes
	ldx #237
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15887)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15888)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 15889)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15890)
	; 30 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 15891)
	; 28 bytes
	ldx #227
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 15892)
	; 30 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 15894 || n = 15896 || n = 15898)
	; 30 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 15895)
	; 30 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 15897)
	; 30 bytes
	delay_n 146
	delay_n (n-146)
.endif
.else
.if((n >= 15899) && (n < 15918))
; 15899 to 15917
.if(n = 15899 || n = 15903)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15900)
	; 30 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 15901)
	; 29 bytes
	ldx #212
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 15902)
	; 30 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 15904)
	; 30 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 15905)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15906)
	; 30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 15907)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15908 || n = 15910 || n = 15914 || n = 15916)
	; 29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15909 || n = 15917)
	; 28..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15911)
	; 27 bytes
	ldx #234 ;hides 'nop'
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 15912)
	; 28 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 15913)
	; 27 bytes
	ldx #234
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15915)
	; 28 bytes
	ldx #218
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.endif
.else
; 15918 to 15937
.if(n = 15918)
	; 30 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 15919 || n = 15928 || (n > 15929 && n < 15934))
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15920)
	; 29 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 15921 || n = 15923)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15922)
	; 30 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 15924)
	; 30 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 15925)
	; 30 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 15926)
	; 25 bytes
	ldx #245
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15927)
	; 30 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 15929)
	; 27 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15934)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15935)
	; 27 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 15936)
	; 27 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 15937)
	; 26 bytes
	ldx #249
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
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
.if(n = 15938)
	; 25 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15939)
	; 25 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 15940)
	; 25 bytes
	ldx #253
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15941 || n = 15942 || (n > 15943 && n < 15947) || n = 15949 || n = 15951 || n = 15953 || n = 15954)
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 15943 || n = 15950)
	; 27..28 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15947)
	; 26 bytes
	ldx #238
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 15948)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15952)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
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
	ldx #202
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 15960)
	; 30 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 15961)
	; 28 bytes
	ldx #228
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 15962)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 15963 || n = 15975)
	; 28..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15964 || n = 15968)
	; 30 bytes
	delay_n 91
	delay_n (n-91)
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
.elseif(n = 15969)
	; 30 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 15970)
	; 30 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 15972)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15973)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15974)
	; 30 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 15976)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.endif
.endif
.else
; 15977 to 16015
.if(n = 15977 || n = 15978 || n = 15980 || n = 15983 || n = 15990 || n = 15993 || (n > 15994 && n < 15999) || n = 16004 || n = 16005 || n = 16008 || n = 16011)
	; 26..29 bytes
	delay_n (n-2)
	nop
.elseif(n = 15979 || n = 15984 || n = 15994 || n = 16006 || n = 16010 || n = 16012)
	; 27..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 15981)
	; 27 bytes
	ldx #235
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 15982)
	; 30 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 15985)
	; 29 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 15986)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 15987)
	; 29 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 15988)
	; 28 bytes
	ldx #219
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 15989)
	; 30 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 15991)
	; 25 bytes
	ldx #246
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 15992)
	; 30 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 15999)
	; 27 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16000)
	; 27 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16001)
	; 25 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 16002)
	; 25 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 16003)
	; 25 bytes
	ldx #254
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 16007)
	; 26 bytes
	ldx #24 ;hides 'clc'
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 16009)
	; 27 bytes
	ldx #232
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16013)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16014)
	; 26 bytes
	ldx #239
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16015)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.else
.if((n >= 16016) && (n < 16055))
; 16016 to 16054
.if(n = 16016 || (n > 16017 && n < 16021) || n = 16033 || n = 16041 || n = 16043 || n = 16045 || n = 16051 || n = 16053)
	; 27..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 16017 || n = 16042 || n = 16050 || n = 16052)
	; 28..29 bytes
	delay_n (n-3)
	ldx $A6
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
	delay_n 11
	delay_n (n-11)
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
	; 28 bytes
	ldx #229
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16032 || n = 16044)
	; 30 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 16035)
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
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16040)
	; 30 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 16046)
	; 30 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16047)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16048)
	; 30 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 16049)
	; 27 bytes
	ldx #236
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16054)
	; 30 bytes
	delay_n 51
	delay_n (n-51)
.endif
.else
; 16055 to 16093
.if(n = 16055 || n = 16058 || (n > 16059 && n < 16063) || n = 16067 || n = 16068 || (n > 16069 && n < 16074) || n = 16080 || n = 16082 || n = 16083 || (n > 16084 && n < 16089))
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 16056)
	; 25 bytes
	ldx #247
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 16057)
	; 29 bytes
	ldx #223
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16059 || n = 16069 || n = 16084)
	; 27..28 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16063)
	; 27 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16064)
	; 25 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 16065)
	; 25 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 16066)
	; 25 bytes
	ldx #255
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 16074)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16075 || n = 16076)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16077)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16078)
	; 27 bytes
	ldx #233
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16079)
	; 30 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 16081)
	; 26 bytes
	ldx #240
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16089)
	; 30 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 16090 || n = 16092)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16091)
	; 30 bytes
	delay_n 151
	delay_n (n-151)
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
	ldx #209
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16095 || n = 16097)
	; 30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16096)
	; 30 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 16098 || n = 16099)
	; 30 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16100)
	; 30 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 16101)
	; 28 bytes
	ldx #230
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16102)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16103 || n = 16107 || n = 16109 || n = 16110 || n = 16119 || n = 16120 || n = 16122 || n = 16123 || n = 16125 || n = 16126 || n = 16130 || n = 16131)
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 16104)
	; 30 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 16105)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16106)
	; 30 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 16108 || n = 16124 || n = 16132)
	; 27..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16111)
	; 30 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 16112)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16113)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 16114)
	; 30 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 16115)
	; 29 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 16116)
	; 30 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 16117)
	; 27 bytes
	ldx #237
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16118)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16121)
	; 25 bytes
	ldx #248
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 16127)
	; 25 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16128)
	; 25 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-22
.elseif(n = 16129)
	; 25 bytes
	ldx #0
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.endif
.else
; 16133 to 16171
.if((n > 16132 && n < 16137) || n = 16143 || n = 16149 || n = 16150 || (n > 16151 && n < 16155))
	; 27..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 16137)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16138 || n = 16139 || n = 16158)
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
	ldx #234 ;hides 'nop'
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16146)
	; 30 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16147)
	; 27 bytes
	ldx #234
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16148)
	; 26 bytes
	ldx #241
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16151)
	; 28 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 16161 || n = 16162)
	; 30 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 16163 || n = 16165)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16164)
	; 30 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 16166 || n = 16167)
	; 30 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 16168 || n = 16170)
	; 30 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 16169)
	; 28 bytes
	ldx #232 ;hides 'inx'
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-25
.elseif(n = 16171)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.endif
.endif
.else
.if((n >= 16172) && (n < 16211))
; 16172 to 16210
.if(n = 16172)
	; 30 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16173 || n = 16175 || n = 16176 || n = 16187 || n = 16188 || n = 16190 || n = 16194 || n = 16195 || n = 16197 || n = 16199 || n = 16203 || n = 16208 || n = 16209)
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 16174 || n = 16189 || n = 16196)
	; 27..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16177)
	; 30 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 16178)
	; 30 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 16179)
	; 30 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 16180)
	; 29 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 16181 || n = 16182)
	; 30 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 16183)
	; 30 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 16184)
	; 26 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 16185)
	; 26 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 16186)
	; 25 bytes
	ldx #249
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 16191)
	; 27 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16192)
	; 27 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16193)
	; 26 bytes
	ldx #253
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16198)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16200)
	; 30 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 16201)
	; 29 bytes
	ldx #216
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16202)
	; 30 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 16204)
	; 30 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16205)
	; 30 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 16206)
	; 28 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 16207)
	; 28 bytes
	ldx #222
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16210)
	; 30 bytes
	delay_n 81
	delay_n (n-81)
.endif
.else
; 16211 to 16250
.if(n = 16211 || (n > 16216 && n < 16222) || n = 16243)
	; 27..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 16212)
	; 30 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 16213 || n = 16214)
	; 30 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 16215)
	; 26 bytes
	ldx #242
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16216)
	; 27 bytes
	ldx #235
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16222)
	; 30 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 16223 || n = 16233)
	; 30 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 16224 || n = 16225)
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
.elseif((n > 16227 && n < 16231))
	; 30 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 16231 || n = 16232)
	; 30 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 16234 || n = 16235)
	; 30 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16236)
	; 30 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 16237)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16238)
	; 30 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 16239)
	; 27 bytes
	ldx #24 ;hides 'clc'
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16240 || n = 16242 || n = 16244)
	; 29..30 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16241)
	; 28 bytes
	ldx #232
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16245)
	; 29 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 16246 || n = 16247)
	; 30 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 16248)
	; 29 bytes
	ldx #211
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16249)
	; 26 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 16250)
	; 26 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
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
	ldx #250
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 16252 || n = 16253 || n = 16255 || n = 16258 || n = 16259 || n = 16261 || n = 16262 || n = 16264 || n = 16266 || n = 16278 || n = 16284 || n = 16286 || n = 16287 || n = 16289)
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 16254 || n = 16263 || n = 16283 || n = 16288)
	; 27..30 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16256)
	; 27 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 4
	bit $24
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16257)
	; 26 bytes
	ldx #254
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16260)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16265)
	; 30 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 16267)
	; 30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16268)
	; 30 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16269)
	; 30 bytes
	delay_n 124
	delay_n (n-124)
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
	; 29 bytes
	ldx #226
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
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
	ldx #217
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16277)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16279)
	; 29 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 16280)
	; 28 bytes
	ldx #223
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16281)
	; 29 bytes
	ldx #220
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16282)
	; 26 bytes
	ldx #243
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16285)
	; 27 bytes
	ldx #236
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.endif
.else
; 16290 to 16328
.if(n = 16290)
	; 30 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 16291 || n = 16305 || (n > 16306 && n < 16310) || n = 16313 || n = 16317 || n = 16318 || n = 16320 || n = 16322 || n = 16323 || n = 16325)
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 16292)
	; 30 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 16293 || n = 16327)
	; 29..30 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16294 || n = 16328)
	; 29..30 bytes
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
.elseif((n > 16297 && n < 16301))
	; 30 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 16301)
	; 30 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 16302)
	; 30 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 16303)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16304)
	; 30 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 16306 || n = 16319 || n = 16324)
	; 27..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16310)
	; 29 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 16311)
	; 28 bytes
	ldx #233
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16312)
	; 30 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 16314)
	; 26 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 16315)
	; 26 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 16316)
	; 25 bytes
	ldx #251
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 16321)
	; 26 bytes
	ldx #255
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16326)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
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
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16332 || n = 16365)
	; 30..31 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16334 || n = 16352 || n = 16357)
	; 28..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16337)
	; 30 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 16338)
	; 31 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 16339 || n = 16340)
	; 30..31 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 16341)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 16342)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16343 || n = 16361 || n = 16366)
	; 30..31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16345)
	; 29 bytes
	ldx #227
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
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
	ldx #244
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16350)
	; 31 bytes
	delay_n 99
	delay_n (n-99)
.elseif(n = 16353)
	; 28 bytes
	ldx #224
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16354)
	; 27 bytes
	ldx #237
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16362)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16363)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 16364)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16367)
	; 30 bytes
	delay_n 51
	delay_n (n-51)
.endif
.else
; 16368 to 16406
.if(n = 16368)
	; 31 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 16369)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16370)
	; 31 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 16371 || n = 16373 || n = 16374 || n = 16382 || n = 16383 || n = 16386 || n = 16387 || n = 16390 || n = 16391 || n = 16394 || n = 16395 || n = 16399 || n = 16404 || n = 16406)
	; 26..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16372 || n = 16384 || n = 16388 || n = 16403 || n = 16405)
	; 27..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16375)
	; 29 bytes
	delay_n 124
	delay_n (n-124)
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
	; 26 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 16380)
	; 26 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 16381)
	; 25 bytes
	ldx #252
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 16385)
	; 26 bytes
	ldx #0
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16389)
	; 27 bytes
	ldx #241
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16392 || n = 16396)
	; 29..30 bytes
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
	; 31 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 16400)
	; 27 bytes
	ldx #232 ;hides 'inx'
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-24
.elseif(n = 16401)
	; 31 bytes
	delay_n 85
	delay_n (n-85)
.elseif(n = 16402)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.endif
.endif
.endif
.else
.if((n >= 16407) && (n < 16485))
.if((n >= 16407) && (n < 16446))
; 16407 to 16445
.if(n = 16407)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16408 || n = 16409 || n = 16418 || (n > 16419 && n < 16423) || n = 16425 || n = 16427 || n = 16428 || n = 16430 || n = 16431 || n = 16436 || n = 16437 || n = 16439 || n = 16441)
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
	; 31 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 16416)
	; 26 bytes
	ldx #245
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16417)
	; 29 bytes
	ldx #228
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16419 || n = 16438)
	; 28..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16423)
	; 27 bytes
	ldx #238
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16424)
	; 30 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16426)
	; 28 bytes
	ldx #225
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16429)
	; 29 bytes
	ldx #222
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16432)
	; 30 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 16433)
	; 27 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16434)
	; 27 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16435)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16440)
	; 29 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 16442)
	; 30 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 16443)
	; 31 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16444)
	; 26 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 16445)
	; 26 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
.endif
.else
; 16446 to 16484
.if(n = 16446)
	; 25 bytes
	ldx #253
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 16447 || n = 16448 || (n > 16449 && n < 16454) || n = 16455 || n = 16459 || n = 16461 || n = 16463 || n = 16464 || n = 16475 || n = 16477 || n = 16478 || n = 16481 || n = 16484)
	; 26..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16449 || n = 16460 || n = 16474 || n = 16476)
	; 27..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16454)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16456)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16457)
	; 27 bytes
	ldx #242
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16458)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16462)
	; 30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16465)
	; 31 bytes
	delay_n 116
	delay_n (n-116)
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
	ldx #24 ;hides 'clc'
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16472)
	; 30 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16473)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16479)
	; 29 bytes
	ldx #214
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16480)
	; 31 bytes
	delay_n 99
	delay_n (n-99)
.elseif(n = 16482)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16483)
	; 26 bytes
	ldx #246
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.endif
.endif
.else
.if((n >= 16485) && (n < 16524))
; 16485 to 16523
.if(n = 16485 || (n > 16486 && n < 16491) || n = 16494 || (n > 16495 && n < 16499) || n = 16502 || n = 16503 || n = 16506 || n = 16512 || n = 16513 || (n > 16514 && n < 16519) || n = 16520)
	; 26..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 16486 || n = 16495 || n = 16504 || n = 16514)
	; 27..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16491 || n = 16519)
	; 29..30 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16492)
	; 27 bytes
	ldx #239
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16493)
	; 30 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16499)
	; 27 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16500)
	; 27 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16501)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16505)
	; 29 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 16507)
	; 30 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 16508)
	; 31 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 16509)
	; 26 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 16510)
	; 26 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 16511)
	; 25 bytes
	ldx #254
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 16521)
	; 28 bytes
	ldx #236
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
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
.if(n = 16524 || n = 16527 || (n > 16528 && n < 16532) || n = 16539 || n = 16546 || n = 16548 || n = 16549 || n = 16551 || n = 16552 || n = 16554 || n = 16555 || n = 16558)
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16525)
	; 27 bytes
	ldx #243
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16526)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16528 || n = 16547 || n = 16553 || n = 16559)
	; 28..31 bytes
	delay_n (n-3)
	ldx $A6
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
	; 30 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 16541)
	; 31 bytes
	delay_n 156
	delay_n (n-156)
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
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16545)
	; 31 bytes
	delay_n 99
	delay_n (n-99)
.elseif(n = 16550)
	; 26 bytes
	ldx #247
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16556)
	; 29 bytes
	ldx #215
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16557)
	; 30 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16560)
	; 30 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16561)
	; 27 bytes
	ldx #240
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.if(n = 16563 || n = 16568 || n = 16569 || n = 16571 || n = 16573 || n = 16577 || n = 16578 || (n > 16579 && n < 16584) || n = 16585 || n = 16589 || n = 16590 || n = 16595 || n = 16597 || n = 16598 || n = 16600)
	; 26..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16564 || n = 16579 || n = 16594 || n = 16596)
	; 27..30 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16565)
	; 27 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16566)
	; 27 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16567)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16570)
	; 29 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 16572)
	; 28 bytes
	ldx #227
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16574)
	; 26 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 16575)
	; 26 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
.elseif(n = 16576)
	; 25 bytes
	ldx #255
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 16584)
	; 29 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16586)
	; 29 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16587)
	; 29 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16588)
	; 29 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16591)
	; 28 bytes
	ldx #237
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16592)
	; 30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16593)
	; 27 bytes
	ldx #244
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16599)
	; 30 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 16601)
	; 31 bytes
	ldx #200
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.elseif(n = 16604 || n = 16609 || n = 16614 || n = 16619 || (n > 16620 && n < 16625) || n = 16634 || n = 16635 || n = 16637 || n = 16638)
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16605)
	; 31 bytes
	delay_n 29
	delay_n (n-29)
.elseif(n = 16606)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	; 31 bytes
	delay_n 99
	delay_n (n-99)
.elseif(n = 16611)
	; 31 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 16612)
	; 30 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16613)
	; 27 bytes
	ldx #234 ;hides 'nop'
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16615)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16616 || n = 16618 || n = 16620 || n = 16636)
	; 28..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16617)
	; 26 bytes
	ldx #248
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16625)
	; 30 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16626)
	; 31 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 16627)
	; 30 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 16628)
	; 30 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16629)
	; 30 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16630)
	; 27 bytes
	ldx #241
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16631)
	; 27 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16632)
	; 27 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16633)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16639)
	; 26 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16640)
	; 26 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-23
.endif
.endif
.else
.if((n >= 16641) && (n < 16680))
; 16641 to 16679
.if(n = 16641)
	; 25 bytes
	ldx #0
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-21
.elseif(n = 16642 || n = 16643 || (n > 16644 && n < 16649) || n = 16650 || n = 16654 || n = 16655 || n = 16659 || n = 16663 || n = 16665 || n = 16666 || n = 16669 || n = 16679)
	; 26..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16644 || n = 16664)
	; 27..29 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 16653 || n = 16656)
	; 29..31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16657)
	; 30 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16658)
	; 31 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 16660 || n = 16662)
	; 30..31 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 16661)
	; 27 bytes
	ldx #245
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
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
	ldx #211
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 16671)
	; 31 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 16672)
	; 30 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16673)
	; 31 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 16674)
	; 30 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 16675)
	; 31 bytes
	delay_n 99
	delay_n (n-99)
.elseif(n = 16676)
	; 31 bytes
	delay_n 126
	delay_n (n-126)
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
.elseif(n = 16681)
	; 31 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 16682)
	; 27 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16683)
	; 27 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16684)
	; 26 bytes
	ldx #249
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16685 || n = 16688 || n = 16690 || n = 16691 || n = 16700 || n = 16701 || n = 16704 || n = 16714)
	; 28..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16686)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16687 || n = 16689 || n = 16702 || n = 16706 || n = 16713)
	; 28..31 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 16696 || n = 16711)
	; 30..31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16697)
	; 27 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16698)
	; 27 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16699)
	; 27 bytes
	ldx #242
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16703)
	; 28 bytes
	ldx #24 ;hides 'clc'
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 16705)
	; 29 bytes
	ldx #232
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16707)
	; 30 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 16708)
	; 31 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 16710)
	; 29 bytes
	ldx #217
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16712)
	; 30 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 16715)
	; 31 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 16716)
	; 31 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 16718)
	; 28 bytes
	ldx #229
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.endif
.endif
.endif
.else
.if((n >= 16719) && (n < 16797))
.if((n >= 16719) && (n < 16758))
.if((n >= 16719) && (n < 16738))
; 16719 to 16737
.if(n = 16719)
	; 29 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 16720 || n = 16724 || n = 16728 || n = 16734 || n = 16735)
	; 29..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16721 || n = 16732)
	; 29..30 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16722)
	; 30 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 16723)
	; 31 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16725)
	; 29 bytes
	ldx #226
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16726)
	; 29 bytes
	ldx #223
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16727)
	; 30 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 16729)
	; 27 bytes
	ldx #246
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16730)
	; 31 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16731)
	; 28 bytes
	ldx #239
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16733)
	; 29 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 16736)
	; 31 bytes
	delay_n 186
	delay_n (n-186)
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
.elseif(n = 16739 || n = 16744 || n = 16752 || n = 16753 || n = 16755 || n = 16756)
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16740)
	; 31 bytes
	delay_n 99
	delay_n (n-99)
.elseif(n = 16741)
	; 30 bytes
	delay_n 124
	delay_n (n-124)
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
.elseif(n = 16746)
	; 31 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 16747)
	; 30 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 16748)
	; 31 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 16749)
	; 27 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16750)
	; 27 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16751)
	; 26 bytes
	ldx #250
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16754)
	; 28 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16757)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.endif
.endif
.else
; 16758 to 16796
.if(n = 16758 || n = 16759 || n = 16761 || n = 16766 || n = 16767 || n = 16769 || n = 16770 || (n > 16771 && n < 16776) || n = 16779 || n = 16781 || n = 16784 || n = 16789 || n = 16793 || n = 16795)
	; 28..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16760 || n = 16771 || n = 16790 || n = 16794)
	; 29..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16762)
	; 30 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 16763)
	; 27 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16764)
	; 27 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16765)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16768)
	; 27 bytes
	ldx #243
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16776)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16777)
	; 29 bytes
	ldx #233
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16778)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16780)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16782)
	; 30 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 16783)
	; 31 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 16785)
	; 31 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 16786)
	; 31 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 16787)
	; 29 bytes
	ldx #218
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16788)
	; 31 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 16791)
	; 28 bytes
	ldx #230
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16792)
	; 30 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 16796)
	; 29 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.endif
.endif
.else
.if((n >= 16797) && (n < 16836))
; 16797 to 16835
.if(n = 16797)
	; 27 bytes
	ldx #247
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16798 || n = 16799 || n = 16803 || n = 16805 || n = 16806 || n = 16809 || n = 16814 || n = 16819 || n = 16820 || (n > 16821 && n < 16826) || n = 16832 || n = 16833 || n = 16835)
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16800 || n = 16804 || n = 16821 || n = 16834)
	; 28..30 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16801)
	; 28 bytes
	ldx #240
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16802)
	; 30 bytes
	delay_n 161
	delay_n (n-161)
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
.elseif(n = 16811 || n = 16812)
	; 30..31 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 16813)
	; 31 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 16815)
	; 30 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 16816)
	; 27 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16817)
	; 27 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16818)
	; 26 bytes
	ldx #251
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16826)
	; 30 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16827)
	; 30 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 16828)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16829)
	; 27 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16830)
	; 27 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16831)
	; 27 bytes
	ldx #(n-1)/66
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.endif
.else
; 16836 to 16875
.if(n = 16836 || n = 16839 || (n > 16840 && n < 16845) || n = 16846 || n = 16851 || n = 16853 || n = 16856 || n = 16861 || n = 16866 || n = 16867 || n = 16869 || n = 16870)
	; 28..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 16837)
	; 27 bytes
	ldx #244
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16838)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16840 || n = 16850 || n = 16868 || n = 16874)
	; 29..30 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16845)
	; 31 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 16847)
	; 28 bytes
	ldx #234 ;hides 'nop'
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16848)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 16849)
	; 29 bytes
	ldx #234
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16852)
	; 31 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 16854)
	; 31 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 16855)
	; 31 bytes
	delay_n 171
	delay_n (n-171)
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
	; 28 bytes
	ldx #232 ;hides 'inx'
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	dex
	Jne *-25
.elseif(n = 16863)
	; 30 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 16864)
	; 28 bytes
	ldx #231
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16865)
	; 27 bytes
	ldx #248
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16871)
	; 28 bytes
	ldx #241
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16872)
	; 31 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 16873)
	; 29 bytes
	ldx #228
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16875)
	; 30 bytes
	delay_n 124
	delay_n (n-124)
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
	ldx #225
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16877)
	; 31 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 16878 || n = 16880 || n = 16886 || n = 16887 || (n > 16888 && n < 16893) || n = 16898 || n = 16901 || (n > 16902 && n < 16906) || n = 16908 || (n > 16909 && n < 16914))
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16879 || n = 16888 || n = 16900 || n = 16902 || n = 16909)
	; 28..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16881)
	; 31 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 16882)
	; 31 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 16883)
	; 27 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16884)
	; 27 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16885)
	; 26 bytes
	ldx #252
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 16893)
	; 30 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16894)
	; 31 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 16895)
	; 27 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16896)
	; 27 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16897)
	; 27 bytes
	ldx #0
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16899)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16906)
	; 27 bytes
	ldx #245
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16907)
	; 30 bytes
	ldx #214
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 29 bytes
	ldx #235
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16922)
	; 31 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 16923)
	; 30 bytes
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
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 16931)
	; 28 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 16932)
	; 28 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 16933)
	; 27 bytes
	ldx #249
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.endif
.else
; 16934 to 16953
.if(n = 16934 || n = 16939 || n = 16943 || n = 16945 || n = 16953)
	; 28..30 bytes
	delay_n (n-2)
	nop
.elseif(n = 16935)
	; 27 bytes
	ldx #24 ;hides 'clc'
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16936 || n = 16938 || n = 16940 || n = 16944)
	; 29..30 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 16937)
	; 28 bytes
	ldx #232
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16941)
	; 28 bytes
	ldx #242
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 16942)
	; 30 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 16946)
	; 31 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 16947)
	; 29 bytes
	ldx #229
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16948)
	; 32 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 16949)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 16950)
	; 27 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16951)
	; 27 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 16952)
	; 26 bytes
	ldx #253
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx $A6
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
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 16971)
	; 31 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 16975)
	; 27 bytes
	ldx #246
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	ldx #215
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
; 16993 to 17031
.if(n = 16993)
	; 29 bytes
	ldx #236
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 16994)
	; 31 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 16995 || n = 16997 || n = 17002 || n = 17003 || (n > 17004 && n < 17008) || (n > 17011 && n < 17016) || n = 17020 || n = 17021 || (n > 17022 && n < 17026) || n = 17028)
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 16996)
	; 31 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 16998)
	; 31 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 16999)
	; 28 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17000)
	; 28 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17001)
	; 27 bytes
	ldx #250
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17004 || n = 17022)
	; 28..29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17008)
	; 31 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 17009)
	; 30 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 17010)
	; 28 bytes
	ldx #233
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17011)
	; 28 bytes
	ldx #243
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17016)
	; 31 bytes
	ldx #205
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 17017)
	; 27 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17018)
	; 27 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17019)
	; 26 bytes
	ldx #254
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 17026)
	; 29 bytes
	ldx #227
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.elseif(n = 17034)
	; 32 bytes
	delay_n 12
	delay_n (n-12)
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
	delay_n 106
	delay_n (n-106)
.elseif(n = 17040)
	; 31 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 17041)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17042)
	; 32 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 17044)
	; 27 bytes
	ldx #247
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17047)
	; 29 bytes
	delay_n (n-3)
	ldx $A6
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
	; 31 bytes
	delay_n 124
	delay_n (n-124)
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
	; 31 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 17064)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17065)
	; 29 bytes
	ldx #237
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17066)
	; 31 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17067)
	; 28 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17068)
	; 28 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17069)
	; 27 bytes
	ldx #251
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17070)
	; 29 bytes
	delay_n (n-2)
	nop
.endif
.endif
.else
; 17071 to 17109
.if(n = 17071 || (n > 17072 && n < 17076) || n = 17087 || n = 17088 || (n > 17089 && n < 17093) || n = 17097 || n = 17103 || n = 17105 || n = 17109)
	; 27..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17072 || n = 17089 || n = 17104)
	; 28..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17076)
	; 30 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 17077 || n = 17094)
	; 30..31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17078)
	; 31 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 17079 || n = 17096)
	; 30..31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17080)
	; 31 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 17081)
	; 27 bytes
	ldx #234 ;hides 'nop'
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17082)
	; 30 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 17083)
	; 28 bytes
	ldx #234
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17084)
	; 27 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17085)
	; 27 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17086)
	; 26 bytes
	ldx #255
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 17093)
	; 29 bytes
	ldx #232 ;hides 'inx'
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-26
.elseif(n = 17095)
	; 29 bytes
	ldx #222
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17098)
	; 30 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17099)
	; 31 bytes
	ldx #206
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 17100)
	; 31 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17101)
	; 29 bytes
	ldx #228
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 32 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 17112)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17113)
	; 27 bytes
	ldx #248
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17114 || n = 17115 || (n > 17116 && n < 17121))
	; 28..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17116)
	; 29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17121)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17122 || n = 17123)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17124 || n = 17125)
	; 31 bytes
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
	; 28 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17136)
	; 28 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17137)
	; 27 bytes
	ldx #252
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17138 || n = 17139 || n = 17141 || n = 17142 || n = 17146)
	; 28..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17140)
	; 29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17143)
	; 30 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 17144)
	; 30 bytes
	ldx #217
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.if(n = 17149 || n = 17165)
	; 30..31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17150)
	; 31 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17151)
	; 27 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17152)
	; 27 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17153)
	; 26 bytes
	ldx #0
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-22
.elseif(n = 17154 || n = 17155 || (n > 17156 && n < 17161) || n = 17166 || n = 17171 || n = 17174 || n = 17178 || n = 17184 || n = 17185 || n = 17187)
	; 27..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17156 || n = 17175 || n = 17179 || n = 17186)
	; 28..31 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 17167)
	; 29 bytes
	ldx #24 ;hides 'clc'
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17168)
	; 31 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 17169)
	; 29 bytes
	ldx #232
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17170)
	; 31 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17172)
	; 29 bytes
	ldx #223
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17173)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 17176)
	; 29 bytes
	ldx #229
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17177)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 17180)
	; 27 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17181)
	; 27 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17182)
	; 27 bytes
	ldx #249
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17183)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.if(n = 17188 || n = 17189 || n = 17196 || n = 17206)
	; 29..32 bytes
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
.elseif(n = 17201)
	; 31 bytes
	ldx #215
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 17202)
	; 31 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 17203)
	; 28 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17204)
	; 28 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17205)
	; 27 bytes
	ldx #253
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.endif
.else
; 17207 to 17226
.if(n = 17207 || n = 17211 || n = 17213 || n = 17223 || n = 17225 || n = 17226)
	; 28..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17208 || n = 17224)
	; 29..30 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17209)
	; 29 bytes
	ldx #239
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17210)
	; 30 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 17212)
	; 31 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 17214)
	; 31 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 17215)
	; 31 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 17216)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
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
	; 28 bytes
	ldx #246
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17222)
	; 31 bytes
	delay_n 136
	delay_n (n-136)
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
	ldx #236
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17230)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17231 || n = 17233 || n = 17235 || n = 17240 || n = 17245 || n = 17252 || n = 17253 || n = 17255 || n = 17256 || (n > 17257 && n < 17261))
	; 28..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17232 || n = 17246 || n = 17257)
	; 29..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17234)
	; 31 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 17236)
	; 31 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 17237)
	; 31 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17238)
	; 30 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 17239)
	; 31 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 17241 || n = 17263)
	; 31..32 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17242)
	; 31 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 17243)
	; 29 bytes
	ldx #233
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
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
	; 27 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17250)
	; 27 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17251)
	; 27 bytes
	ldx #250
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17254)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	ldx #208
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.endif
.endif
.else
.if((n >= 17266) && (n < 17305))
; 17266 to 17304
.if(n = 17266 || n = 17285)
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
.elseif(n = 17269 || n = 17298)
	; 31..32 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 17270)
	; 32 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17271)
	; 28 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17272)
	; 28 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17273)
	; 27 bytes
	ldx #254
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17274 || n = 17275 || (n > 17276 && n < 17281) || n = 17283 || n = 17286 || n = 17293 || (n > 17294 && n < 17298) || n = 17304)
	; 28..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17276 || n = 17294)
	; 29..30 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17281)
	; 29 bytes
	ldx #240
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17282)
	; 31 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 17284)
	; 31 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17287 || n = 17290)
	; 32 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17288)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17289)
	; 31 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 17291)
	; 28 bytes
	ldx #247
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17292)
	; 32 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 17299)
	; 31 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 17300)
	; 32 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17301)
	; 31 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 17302)
	; 28 bytes
	ldx #237
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17303)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.endif
.else
; 17305 to 17343
.if(n = 17305 || n = 17323 || n = 17328)
	; 29..30 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n > 17305 && n < 17309) || n = 17311 || n = 17316 || n = 17321 || n = 17322 || n = 17327 || (n > 17328 && n < 17332) || n = 17342 || n = 17343)
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
	; 29 bytes
	ldx #234 ;hides 'nop'
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 17317)
	; 29 bytes
	ldx #234
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17318)
	; 27 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17319)
	; 27 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17320)
	; 27 bytes
	ldx #251
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17324)
	; 28 bytes
	ldx #232 ;hides 'inx'
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	dex
	Jne *-25
.elseif(n = 17325)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17326)
	; 29 bytes
	ldx #231
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 17336)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 17339)
	; 28 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17340)
	; 28 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17341)
	; 27 bytes
	ldx #255
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.endif
.endif
.endif
.else
.if((n >= 17344) && (n < 17422))
.if((n >= 17344) && (n < 17383))
; 17344 to 17382
.if(n = 17344 || n = 17356 || n = 17364 || n = 17378)
	; 29..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n > 17344 && n < 17348) || n = 17355 || n = 17357 || n = 17363 || (n > 17364 && n < 17368) || n = 17377 || n = 17379 || n = 17380)
	; 29..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17348)
	; 31 bytes
	ldx #209
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 17349)
	; 31 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 17350)
	; 32 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17351)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17352)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 17353)
	; 29 bytes
	ldx #241
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17354)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17358 || n = 17359)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 17360)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17361)
	; 28 bytes
	ldx #248
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17362)
	; 32 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 17368 || n = 17369)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17370)
	; 32 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 17371)
	; 32 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 17372)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 17373 || n = 17374)
	; 31 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 17375)
	; 28 bytes
	ldx #238
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17376)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 17381)
	; 30 bytes
	ldx #220
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 17382)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.endif
.else
; 17383 to 17421
.if(n = 17383 || n = 17390 || n = 17391 || (n > 17392 && n < 17396) || n = 17398 || n = 17400 || n = 17402 || n = 17405 || n = 17410 || n = 17411 || (n > 17412 && n < 17416) || n = 17418)
	; 28..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17384 || n = 17392 || n = 17404 || n = 17412)
	; 29..32 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17385 || n = 17406 || n = 17419)
	; 31..32 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17386)
	; 32 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 17387)
	; 27 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17388)
	; 27 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17389)
	; 27 bytes
	ldx #252
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17396)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17397 || n = 17417)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17399)
	; 28 bytes
	ldx #24 ;hides 'clc'
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17401)
	; 29 bytes
	ldx #232
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17403)
	; 29 bytes
	ldx #226
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17407)
	; 28 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17408)
	; 28 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	bit $24
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17409)
	; 27 bytes
	ldx #0
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17416)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 17420)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 17421)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.else
.if((n >= 17422) && (n < 17461))
; 17422 to 17460
.if(n = 17422)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17423 || n = 17427 || n = 17432 || n = 17433 || (n > 17434 && n < 17439) || n = 17450 || (n > 17451 && n < 17455) || n = 17459 || n = 17460)
	; 28..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17424)
	; 32 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17425)
	; 29 bytes
	ldx #242
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17426)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17428 || n = 17434 || n = 17451)
	; 30..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17429)
	; 29 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17430)
	; 29 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17431)
	; 28 bytes
	ldx #249
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17439)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 17440)
	; 32 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 17441)
	; 31 bytes
	ldx #218
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif((n > 17441 && n < 17445) || n = 17449)
	; 31 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 17445)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
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
	ldx #239
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17455)
	; 32 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 17456)
	; 27 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17457)
	; 27 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17458)
	; 27 bytes
	ldx #253
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.endif
.else
; 17461 to 17500
.if(n = 17461 || n = 17470 || n = 17479 || n = 17483)
	; 29..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n > 17461 && n < 17465) || n = 17469 || (n > 17470 && n < 17475) || n = 17478 || n = 17482 || n = 17484)
	; 28..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17465)
	; 29 bytes
	ldx #236
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17466 || n = 17475)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17467)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17468 || n = 17477)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17476)
	; 29 bytes
	ldx #233
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17480)
	; 29 bytes
	ldx #227
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17481)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
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
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17496)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 17497)
	; 29 bytes
	ldx #243
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17499)
	; 29 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17500)
	; 29 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
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
	; 28 bytes
	ldx #250
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17502 || n = 17503 || (n > 17504 && n < 17508) || n = 17522 || n = 17523 || n = 17528 || n = 17529 || (n > 17530 && n < 17535))
	; 28..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17504 || n = 17524 || n = 17530)
	; 29..30 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif((n > 17510 && n < 17514))
	; 31 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 17514)
	; 31 bytes
	ldx #211
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 17515)
	; 32 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 17516)
	; 32 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 17519)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 17520)
	; 30 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 17521)
	; 28 bytes
	ldx #240
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17525)
	; 27 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17526)
	; 27 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17527)
	; 27 bytes
	ldx #254
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17535)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17536 || n = 17537)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17538)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17539)
	; 29 bytes
	ldx #237
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.endif
.else
; 17540 to 17578
.if(n = 17540 || (n > 17541 && n < 17546) || n = 17553 || n = 17556 || n = 17559 || n = 17561 || n = 17572 || n = 17573 || (n > 17574 && n < 17578))
	; 28..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17541 || n = 17552 || n = 17554 || n = 17560 || n = 17574)
	; 29..31 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #234 ;hides 'nop'
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17550)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17551)
	; 29 bytes
	ldx #234
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17555)
	; 30 bytes
	ldx #232 ;hides 'inx'
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	dex
	Jne *-27
.elseif(n = 17557)
	; 29 bytes
	ldx #228
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.elseif(n = 17565)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 17566 || n = 17568)
	; 32 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 17567)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17569)
	; 29 bytes
	ldx #244
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17570)
	; 29 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17571)
	; 28 bytes
	ldx #251
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17578)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.elseif((n > 17579 && n < 17583) || n = 17591)
	; 31 bytes
	delay_n 124
	delay_n (n-124)
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
.elseif(n = 17586 || n = 17588)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 17587)
	; 32 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17589)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17590)
	; 32 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17593)
	; 32 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 17594)
	; 27 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17595)
	; 27 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17596)
	; 27 bytes
	ldx #255
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17597 || n = 17598 || (n > 17599 && n < 17604) || n = 17610 || n = 17611 || (n > 17612 && n < 17617))
	; 28..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17599 || n = 17612)
	; 29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17604 || n = 17617)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17605 || n = 17606)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17607)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 17608)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17609)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.endif
.else
; 17618 to 17656
.if(n = 17618)
	; 30 bytes
	ldx #223
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 17619)
	; 31 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17620 || n = 17628 || n = 17630 || n = 17635 || n = 17636 || n = 17638 || n = 17642 || n = 17643 || (n > 17644 && n < 17648))
	; 29..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17621)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17622)
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
	ldx #235
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17627)
	; 32 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 17629 || n = 17637 || n = 17644)
	; 30..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17631)
	; 29 bytes
	ldx #24 ;hides 'clc'
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17632)
	; 32 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17633)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 17634)
	; 29 bytes
	ldx #229
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17639)
	; 29 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17640)
	; 29 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17641)
	; 28 bytes
	ldx #252
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17648)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif((n > 17648 && n < 17652))
	; 31 bytes
	delay_n 124
	delay_n (n-124)
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
.elseif(n = 17655)
	; 32 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17656)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
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
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17659)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 17660)
	; 32 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 17661 || n = 17666 || n = 17667 || (n > 17668 && n < 17673) || n = 17681 || n = 17682 || (n > 17683 && n < 17687) || n = 17689)
	; 28..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17662)
	; 31 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 17663)
	; 27 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17664)
	; 27 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-24
.elseif(n = 17665)
	; 27 bytes
	ldx #0
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17668 || n = 17683)
	; 29 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17673 || n = 17688)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17674 || n = 17675 || n = 17690)
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
	; 28 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17679)
	; 28 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17680)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17687)
	; 29 bytes
	ldx #239
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
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
	ldx #224
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 17698)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17699 || n = 17703 || n = 17705 || n = 17712)
	; 30..31 bytes
	delay_n (n-2)
	nop
.elseif(n = 17700)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17701)
	; 29 bytes
	ldx #236
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17702)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17704 || n = 17714)
	; 30..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17706)
	; 32 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 17707)
	; 31 bytes
	ldx #227
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 17708)
	; 32 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17709)
	; 29 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17710)
	; 29 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17711)
	; 28 bytes
	ldx #253
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17713)
	; 29 bytes
	ldx #246
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.endif
.else
; 17715 to 17734
.if(n = 17715 || n = 17717 || n = 17728)
	; 30..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17716)
	; 31 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n > 17717 && n < 17721) || n = 17733)
	; 31 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 17721)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 17722)
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
	; 32 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17730)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
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
.elseif(n = 17738 || n = 17742 || (n > 17743 && n < 17747) || n = 17752 || n = 17753 || (n > 17754 && n < 17759) || n = 17763 || n = 17765 || n = 17773)
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17739 || n = 17741)
	; 32 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 17740)
	; 28 bytes
	ldx #243
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17743 || n = 17754 || n = 17764)
	; 29..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17747 || n = 17760)
	; 32 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17748 || n = 17759)
	; 31..32 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17749)
	; 28 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17750)
	; 28 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17751)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17761)
	; 29 bytes
	ldx #240
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17762)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 17766)
	; 32 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 17767)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17768)
	; 32 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 17769 || n = 17771)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 17770)
	; 32 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 17772)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.endif
.else
; 17774 to 17812
.if(n = 17774)
	; 32 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 17775)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 17776)
	; 29 bytes
	ldx #237
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17777)
	; 32 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17778 || n = 17782 || n = 17783 || n = 17787 || n = 17790 || (n > 17791 && n < 17795) || n = 17808)
	; 29..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17779)
	; 29 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17780)
	; 29 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17781)
	; 28 bytes
	ldx #254
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17784 || n = 17791)
	; 30..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17785)
	; 29 bytes
	ldx #247
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17786)
	; 29 bytes
	ldx #232 ;hides 'inx'
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	dex
	Jne *-26
.elseif(n = 17788)
	; 29 bytes
	ldx #231
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17789 || n = 17803 || n = 17804)
	; 31..32 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 17795)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 17796)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 17797)
	; 32 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 17798)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17799 || n = 17801)
	; 32 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 17800)
	; 32 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17802)
	; 32 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 17805)
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
.elseif((n > 17808 && n < 17812))
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
	ldx #244
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17814)
	; 32 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 17815 || (n > 17816 && n < 17820) || n = 17823 || n = 17824 || (n > 17825 && n < 17830) || n = 17837 || n = 17839 || n = 17840)
	; 28..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 17816 || n = 17825 || n = 17838)
	; 29..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17820)
	; 28 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17821)
	; 28 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17822)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	; 29 bytes
	ldx #241
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17836)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 17841)
	; 31 bytes
	ldx #223
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 17842)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17843)
	; 32 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 17844 || n = 17845)
	; 32 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 17846)
	; 31 bytes
	ldx #215
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 17847)
	; 32 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17848)
	; 32 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17849)
	; 29 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17850)
	; 29 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17851)
	; 28 bytes
	ldx #255
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.endif
.else
; 17852 to 17890
.if(n = 17852 || n = 17853 || n = 17856 || n = 17859 || n = 17867 || n = 17869 || n = 17870 || n = 17880 || n = 17884 || n = 17885 || n = 17888 || n = 17890)
	; 29..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17854 || n = 17860 || n = 17866 || n = 17868 || n = 17889)
	; 30..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17855)
	; 30 bytes
	ldx #226
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 17857)
	; 29 bytes
	ldx #248
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17858)
	; 32 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 17861)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 17862)
	; 32 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 17863)
	; 29 bytes
	ldx #24 ;hides 'clc'
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17864 || n = 17874 || n = 17875)
	; 31..32 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 17865)
	; 29 bytes
	ldx #232
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17871)
	; 32 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 17872)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 17873)
	; 32 bytes
	delay_n 51
	delay_n (n-51)
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
	ldx #245
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.if(n = 17891)
	; 28 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17892)
	; 28 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17893)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 17894 || n = 17895 || (n > 17896 && n < 17901) || n = 17906 || n = 17910 || n = 17911 || n = 17913 || n = 17915 || n = 17916 || n = 17922 || n = 17923 || n = 17925)
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17896 || n = 17912 || n = 17924)
	; 29..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17901)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17902)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	; 29 bytes
	ldx #242
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17914)
	; 32 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 17917)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 17918)
	; 32 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 17919)
	; 29 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 17920)
	; 29 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 10
	.repeat 4
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17921)
	; 28 bytes
	ldx #0
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17926)
	; 29 bytes
	ldx #239
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17927)
	; 29 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17928)
	; 29 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 17929)
	; 29 bytes
	ldx #249
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.endif
.else
; 17930 to 17968
.if(n = 17930 || n = 17931 || n = 17933 || n = 17935 || n = 17936 || n = 17939 || n = 17944 || n = 17946 || n = 17951 || n = 17955 || n = 17956 || n = 17960 || n = 17961 || n = 17965 || n = 17966 || n = 17968)
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 17932 || n = 17940 || n = 17945 || n = 17967)
	; 29..32 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 17934)
	; 30 bytes
	ldx #227
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 17937)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 17938)
	; 32 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 17941)
	; 31 bytes
	ldx #230
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 17942)
	; 29 bytes
	ldx #233
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17943)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
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
	; 31 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 17959)
	; 28 bytes
	ldx #246
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 17962)
	; 28 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17963)
	; 28 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 17964)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	; 29 bytes
	ldx #243
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 17984)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 17986 || n = 18004)
	; 31 bytes
	delay_n (n-3)
	ldx $A6
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
	; 29 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18000)
	; 29 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18001)
	; 29 bytes
	ldx #240
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #217
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18013)
	; 30 bytes
	ldx #228
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18014)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 18015 || n = 18021 || n = 18023 || n = 18026 || n = 18027 || n = 18036 || n = 18037 || (n > 18038 && n < 18043))
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18016 || n = 18020 || n = 18022 || n = 18038)
	; 29..32 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18017)
	; 29 bytes
	ldx #234 ;hides 'nop'
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18018)
	; 32 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18019)
	; 29 bytes
	ldx #234
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #247
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 18033)
	; 28 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 18034)
	; 28 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 18035)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.elseif(n = 18048 || n = 18052 || n = 18053 || n = 18058 || n = 18059 || (n > 18060 && n < 18064) || n = 18074 || n = 18075 || n = 18077 || n = 18078 || n = 18080 || n = 18082)
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
	; 29 bytes
	ldx #244
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18060 || n = 18079)
	; 31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18064)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
	ldx #203
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 29 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18072)
	; 29 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18073)
	; 29 bytes
	ldx #251
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18076)
	; 29 bytes
	ldx #241
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18081)
	; 31 bytes
	ldx #226
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18083)
	; 32 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 18084)
	; 32 bytes
	delay_n 191
	delay_n (n-191)
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
.elseif(n = 18087 || n = 18088)
	; 31..32 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 18089)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18090)
	; 32 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 18091 || n = 18093 || n = 18094 || n = 18098 || n = 18100 || n = 18102 || n = 18103 || n = 18107 || n = 18108 || (n > 18109 && n < 18114) || n = 18119 || n = 18124)
	; 28..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18092)
	; 30 bytes
	ldx #229
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18095)
	; 30 bytes
	ldx #24 ;hides 'clc'
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 18096)
	; 29 bytes
	ldx #235
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18097)
	; 31 bytes
	ldx #232
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18099 || n = 18109)
	; 29..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18101)
	; 32 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 18104)
	; 28 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 18105)
	; 28 bytes
	ldx #248
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 18106)
	; 27 bytes
	ldx #(n-1)/71
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 18114)
	; 31 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18115 || n = 18116)
	; 31..32 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18117)
	; 31 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 18118)
	; 31 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18120)
	; 32 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 18121)
	; 32 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 18122)
	; 31 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 18123)
	; 32 bytes
	ldx #221
	nop
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-28
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
; 18126 to 18164
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
.elseif(n = 18129 || n = 18133 || n = 18135 || n = 18139 || n = 18146 || n = 18147 || n = 18149 || n = 18153 || n = 18155 || n = 18156 || n = 18163 || n = 18164)
	; 30..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18130)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 18131)
	; 29 bytes
	ldx #245
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18132)
	; 32 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18134 || n = 18148 || n = 18154)
	; 31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18136)
	; 32 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18137)
	; 32 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18138)
	; 32 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 18140)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 18141)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18142)
	; 32 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18143)
	; 29 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18144)
	; 29 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18145)
	; 29 bytes
	ldx #252
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18150)
	; 32 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 18151)
	; 29 bytes
	ldx #242
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18152)
	; 32 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18157)
	; 32 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 18158 || n = 18159)
	; 31..32 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 18160)
	; 32 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18161)
	; 31 bytes
	ldx #227
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18162)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
.endif
.else
.if((n >= 18165) && (n < 18184))
; 18165 to 18183
.if(n = 18165)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18166)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 18167 || n = 18169 || n = 18179 || (n > 18180 && n < 18184))
	; 28..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 18168 || n = 18174 || n = 18180)
	; 29..32 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18170)
	; 33 bytes
	delay_n 64
	delay_n (n-64)
.elseif(n = 18171)
	; 30 bytes
	ldx #230
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18172)
	; 32 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 18173)
	; 29 bytes
	ldx #236
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18175)
	; 28 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18176)
	; 28 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-25
.elseif(n = 18177)
	; 27 bytes
	ldx #0
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-23
.elseif(n = 18178)
	; 28 bytes
	ldx #249
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.endif
.else
; 18184 to 18203
.if(n = 18184 || n = 18195 || n = 18200)
	; 31..33 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 18193 || n = 18194)
	; 32..33 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18196)
	; 32 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 18197)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18198)
	; 32 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 18199)
	; 33 bytes
	delay_n 22
	delay_n (n-22)
.elseif(n = 18201)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 18202)
	; 32 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18203)
	; 32 bytes
	delay_n 26
	delay_n (n-26)
.endif
.endif
.endif
.else
.if((n >= 18204) && (n < 18243))
; 18204 to 18242
.if(n = 18204)
	; 33 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18205)
	; 29 bytes
	ldx #246
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18206)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 18207 || n = 18209 || n = 18210 || n = 18218 || n = 18219 || n = 18221 || n = 18225 || n = 18228 || n = 18230 || n = 18231 || n = 18235 || n = 18239 || n = 18240)
	; 30..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18208 || n = 18220 || n = 18229)
	; 31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18211)
	; 32 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 18212)
	; 32 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18213 || n = 18214)
	; 32..33 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18215)
	; 29 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18216)
	; 29 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18217)
	; 29 bytes
	ldx #253
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18222)
	; 32 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 18223 || n = 18224)
	; 32..33 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18226)
	; 29 bytes
	ldx #243
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18227)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 18232)
	; 32 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 18233)
	; 32 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 18234)
	; 33 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 18236)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18237)
	; 32 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 18238)
	; 32 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 18241)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18242)
	; 32 bytes
	delay_n 136
	delay_n (n-136)
.endif
.else
; 18243 to 18281
.if(n = 18243 || n = 18245 || n = 18252 || n = 18253 || (n > 18254 && n < 18258) || n = 18265 || n = 18270 || n = 18274 || n = 18275 || n = 18281)
	; 29..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18244 || n = 18254 || n = 18264)
	; 30..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18246)
	; 33 bytes
	ldx #205
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 18247)
	; 32 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18248)
	; 30 bytes
	ldx #232 ;hides 'inx'
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-27
.elseif(n = 18249)
	; 29 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18250)
	; 29 bytes
	ldx #237
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18251)
	; 28 bytes
	ldx #250
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 18258)
	; 32 bytes
	delay_n 81
	delay_n (n-81)
.elseif(n = 18259)
	; 32 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 18260)
	; 32 bytes
	ldx #36 ;hides 'bit zp'
	nop
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-29
.elseif(n = 18261)
	; 31 bytes
	ldx #220
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18262)
	; 32 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 18263)
	; 32 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 18266 || n = 18267)
	; 32..33 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 18268)
	; 32 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18269)
	; 33 bytes
	delay_n 92
	delay_n (n-92)
.elseif(n = 18271)
	; 33 bytes
	ldx #210
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 18272)
	; 32 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 18273)
	; 32 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18276)
	; 32 bytes
	ldx #215
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 18277)
	; 32 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 18278)
	; 32 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18279)
	; 29 bytes
	ldx #247
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18280)
	; 32 bytes
	delay_n 174
	delay_n (n-174)
.endif
.endif
.endif
.else
.if((n >= 18282) && (n < 18360))
.if((n >= 18282) && (n < 18321))
; 18282 to 18320
.if(n = 18282 || n = 18292 || n = 18304 || n = 18310 || n = 18320)
	; 31..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n > 18282 && n < 18286) || n = 18290 || n = 18291 || (n > 18292 && n < 18296) || n = 18303 || n = 18305 || n = 18306 || n = 18309 || n = 18315 || n = 18319)
	; 30..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18286)
	; 33 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 18287)
	; 29 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18288)
	; 29 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18289)
	; 29 bytes
	ldx #254
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18296)
	; 33 bytes
	delay_n 190
	delay_n (n-190)
.elseif(n = 18297)
	; 32 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 18298)
	; 32 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 18299 || n = 18300)
	; 32 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 18301)
	; 29 bytes
	ldx #244
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18302)
	; 32 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18307)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.elseif(n = 18316 || n = 18318)
	; 32..33 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18317)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.endif
.else
; 18321 to 18359
.if(n = 18321)
	; 31 bytes
	ldx #229
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18322)
	; 29 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18323)
	; 29 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18324)
	; 28 bytes
	ldx #251
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 18325 || n = 18326 || n = 18328 || n = 18332 || n = 18340 || n = 18345 || n = 18346 || n = 18350 || n = 18355 || n = 18357)
	; 29..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18327)
	; 29 bytes
	ldx #238
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18329)
	; 30 bytes
	ldx #232
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18330 || n = 18347 || n = 18356)
	; 31..33 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 18341)
	; 33 bytes
	delay_n 124
	delay_n (n-124)
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
	ldx #221
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18348 || n = 18349)
	; 32..33 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 18351)
	; 32 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 18352)
	; 33 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18353)
	; 29 bytes
	ldx #248
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18354)
	; 33 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 18358)
	; 32 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 18359)
	; 29 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.endif
.endif
.else
.if((n >= 18360) && (n < 18399))
; 18360 to 18398
.if(n = 18360)
	; 29 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18361)
	; 29 bytes
	ldx #255
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18362 || n = 18363 || (n > 18364 && n < 18368) || n = 18370 || n = 18378 || (n > 18379 && n < 18384) || n = 18389 || n = 18390 || n = 18392 || n = 18394 || n = 18398)
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18364 || n = 18379 || n = 18391)
	; 31..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18368)
	; 32 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 18369)
	; 32 bytes
	ldx #224
	nop
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 18371 || n = 18386)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
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
.elseif(n = 18375)
	; 32 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 18376)
	; 29 bytes
	ldx #245
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.elseif(n = 18387)
	; 33 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 18388)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18393)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18395)
	; 29 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18396)
	; 29 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18397)
	; 28 bytes
	ldx #252
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.endif
.else
; 18399 to 18437
.if(n = 18399 || (n > 18400 && n < 18404) || n = 18406 || n = 18410 || n = 18412 || n = 18428 || n = 18429 || n = 18434 || n = 18435 || n = 18437)
	; 29..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 18400 || n = 18407 || n = 18411 || n = 18430 || n = 18436)
	; 30..32 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18404)
	; 29 bytes
	ldx #239
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18405)
	; 32 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18408)
	; 30 bytes
	ldx #233
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18409)
	; 31 bytes
	ldx #236
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
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
	ldx #207
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 18425)
	; 30 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 18426)
	; 30 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 18427)
	; 29 bytes
	ldx #249
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18431)
	; 29 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18432)
	; 29 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18433)
	; 29 bytes
	ldx #0
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.endif
.endif
.endif
.endif
.else
.if((n >= 18438) && (n < 18594))
.if((n >= 18438) && (n < 18516))
.if((n >= 18438) && (n < 18477))
; 18438 to 18476
.if(n = 18438 || n = 18439 || n = 18453 || (n > 18454 && n < 18459) || n = 18464 || n = 18471 || n = 18472 || (n > 18473 && n < 18477))
	; 29..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18440)
	; 33 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 18441 || n = 18459)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18442)
	; 33 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 18443)
	; 33 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18444 || n = 18462)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 18445)
	; 33 bytes
	ldx #212
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 18446)
	; 32 bytes
	ldx #217
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 18447)
	; 33 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18448)
	; 32 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 18449 || n = 18454 || n = 18473)
	; 30..34 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18450)
	; 33 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 18451)
	; 29 bytes
	ldx #246
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18452)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 18460)
	; 33 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 18461)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18463)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18465)
	; 33 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18466)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 18467)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18468)
	; 29 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18469)
	; 29 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18470)
	; 28 bytes
	ldx #253
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.endif
.else
; 18477 to 18515
.if(n = 18477 || n = 18483 || n = 18486 || n = 18489 || n = 18491 || n = 18492 || n = 18494 || n = 18502 || n = 18503 || (n > 18504 && n < 18508) || n = 18512 || n = 18514)
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18478 || n = 18509)
	; 32..33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18479)
	; 31 bytes
	ldx #232 ;hides 'inx'
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	dex
	Jne *-28
.elseif(n = 18480)
	; 32 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18481)
	; 29 bytes
	ldx #240
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18482 || n = 18493 || n = 18497)
	; 32..33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18484 || n = 18488 || n = 18490 || n = 18504 || n = 18513)
	; 31..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18485)
	; 29 bytes
	ldx #234 ;hides 'nop'
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18487)
	; 30 bytes
	ldx #234
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.elseif(n = 18499)
	; 30 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 18500)
	; 30 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 18501)
	; 29 bytes
	ldx #250
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18508)
	; 33 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 18510)
	; 31 bytes
	ldx #223
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.elseif(n = 18518 || n = 18525)
	; 33..34 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 18519 || n = 18521)
	; 32..33 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 18520)
	; 33 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18522)
	; 34 bytes
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
.elseif(n = 18526)
	; 29 bytes
	ldx #247
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18527)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 18528 || (n > 18529 && n < 18533) || n = 18535 || n = 18539 || n = 18540 || n = 18544 || n = 18545 || (n > 18546 && n < 18550) || n = 18552)
	; 29..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18529 || n = 18546)
	; 30..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18533)
	; 32 bytes
	ldx #226
	nop
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 18534 || n = 18551)
	; 32..33 bytes
	delay_n (n-8)
	delay_n 8
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
.elseif(n = 18541)
	; 29 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18542)
	; 29 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18543)
	; 28 bytes
	ldx #254
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 18550)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.endif
.else
; 18555 to 18593
.if(n = 18555 || n = 18587)
	; 32..33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18556)
	; 33 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 18557 || n = 18560 || (n > 18561 && n < 18565) || n = 18567 || n = 18568 || n = 18570 || n = 18572 || n = 18576 || n = 18577 || (n > 18578 && n < 18583))
	; 30..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18558)
	; 29 bytes
	ldx #241
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18559)
	; 31 bytes
	ldx #24 ;hides 'clc'
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 18561 || n = 18569 || n = 18578)
	; 31..32 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18565)
	; 31 bytes
	ldx #238
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18566)
	; 30 bytes
	ldx #235
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18571)
	; 33 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18573)
	; 30 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 18574)
	; 30 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 18575)
	; 29 bytes
	ldx #251
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18583 || n = 18590)
	; 33..34 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 18584)
	; 33 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 18585)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18586)
	; 33 bytes
	delay_n 116
	delay_n (n-116)
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
	; 33 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 18593)
	; 31 bytes
	ldx #224
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.endif
.endif
.endif
.else
.if((n >= 18594) && (n < 18672))
.if((n >= 18594) && (n < 18633))
; 18594 to 18632
.if(n = 18594)
	; 32 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 18595 || n = 18597 || n = 18598 || n = 18603 || (n > 18604 && n < 18609) || n = 18610 || n = 18617 || n = 18618 || n = 18620 || n = 18622 || n = 18623 || n = 18625 || n = 18630)
	; 29..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18596 || n = 18604 || n = 18619)
	; 30..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18599)
	; 33 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 18600)
	; 34 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18601)
	; 29 bytes
	ldx #248
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18602)
	; 33 bytes
	ldx #209
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 18609)
	; 33 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 18611 || n = 18626)
	; 32..33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18612 || n = 18627)
	; 32..33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 18613 || n = 18628)
	; 32..33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18614)
	; 29 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18615)
	; 29 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18616)
	; 28 bytes
	ldx #255
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 18621)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
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
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18632)
	; 33 bytes
	delay_n 16
	delay_n (n-16)
.endif
.else
.if((n >= 18633) && (n < 18652))
; 18633 to 18651
.if(n = 18633 || n = 18637 || n = 18639 || n = 18640 || n = 18650 || n = 18651)
	; 30..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 18634 || n = 18638)
	; 31..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18635)
	; 29 bytes
	ldx #242
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18636)
	; 33 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 18641)
	; 31 bytes
	ldx #233
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18642)
	; 33 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18643)
	; 31 bytes
	ldx #239
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18644)
	; 33 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18645)
	; 30 bytes
	ldx #236
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18646)
	; 33 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 18647)
	; 30 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 18648)
	; 30 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 18649)
	; 29 bytes
	ldx #252
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.endif
.else
; 18652 to 18671
.if(n = 18652)
	; 31 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n > 18652 && n < 18656) || n = 18658 || n = 18668)
	; 31..34 bytes
	delay_n (n-2)
	nop
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
	delay_n 11
	delay_n (n-11)
.elseif(n = 18661)
	; 33 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 18662)
	; 33 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18663)
	; 33 bytes
	ldx #(n-1)/86
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 18664)
	; 33 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 18665 || n = 18667)
	; 32..33 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 18666)
	; 33 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18669)
	; 33 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 18670)
	; 34 bytes
	delay_n 127
	delay_n (n-127)
.elseif(n = 18671)
	; 34 bytes
	delay_n 36
	delay_n (n-36)
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
	; 29 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18675)
	; 29 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18676)
	; 29 bytes
	ldx #249
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18677 || n = 18678 || (n > 18679 && n < 18684) || n = 18690 || n = 18691 || (n > 18692 && n < 18697) || n = 18698 || n = 18699 || n = 18703 || n = 18708)
	; 29..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18679 || n = 18692)
	; 30..31 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18684)
	; 33 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18685 || n = 18686)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18687)
	; 29 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18688)
	; 29 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18689)
	; 28 bytes
	ldx #0
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-24
.elseif(n = 18697)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18700)
	; 32 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 18701)
	; 32 bytes
	delay_n 12
	delay_n (n-12)
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
	ldx #215
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 18707)
	; 33 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 18709)
	; 33 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 18710)
	; 30 bytes
	ldx #232 ;hides 'inx'
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-27
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
	ldx #243
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18713 || n = 18715)
	; 31..32 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18714 || (n > 18715 && n < 18719) || (n > 18724 && n < 18730))
	; 30..32 bytes
	delay_n (n-2)
	nop
.elseif(n = 18719)
	; 31 bytes
	ldx #234 ;hides 'nop'
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 18720)
	; 33 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18721)
	; 30 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 18722)
	; 30 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 18723)
	; 29 bytes
	ldx #253
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18724)
	; 30 bytes
	ldx #237
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18730)
	; 33 bytes
	delay_n 41
	delay_n (n-41)
.endif
.else
; 18731 to 18750
.if(n = 18731)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18732)
	; 33 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 18733)
	; 33 bytes
	ldx #223
	bit $24
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 18734)
	; 33 bytes
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
.elseif(n = 18738 || n = 18740)
	; 32..33 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 18739)
	; 33 bytes
	delay_n 196
	delay_n (n-196)
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
	; 34 bytes
	delay_n 186
	delay_n (n-186)
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
.elseif(n = 18749)
	; 29 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18750)
	; 29 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
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
	ldx #250
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18752 || n = 18753 || (n > 18754 && n < 18759) || n = 18761 || n = 18764 || n = 18775 || n = 18777 || n = 18778 || n = 18781 || n = 18784 || n = 18788)
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18754 || n = 18762 || n = 18776)
	; 31..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18759)
	; 31 bytes
	ldx #226
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.elseif(n = 18766)
	; 34 bytes
	delay_n 131
	delay_n (n-131)
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
.elseif(n = 18771)
	; 34 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 18772)
	; 33 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 18773)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18774)
	; 34 bytes
	delay_n 85
	delay_n (n-85)
.elseif(n = 18779)
	; 32 bytes
	ldx #229
	nop
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 18780)
	; 33 bytes
	ldx #211
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 18782)
	; 33 bytes
	delay_n 166
	delay_n (n-166)
.elseif(n = 18783)
	; 33 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 18785)
	; 33 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 18786)
	; 32 bytes
	ldx #221
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 18787)
	; 33 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 18789)
	; 29 bytes
	ldx #244
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.endif
.else
; 18790 to 18828
.if(n = 18790)
	; 33 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18791 || n = 18794 || n = 18798 || n = 18799 || n = 18802 || n = 18805 || (n > 18806 && n < 18810) || n = 18814 || n = 18819 || n = 18827 || n = 18828)
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18792 || n = 18800 || n = 18804 || n = 18806)
	; 31..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18793)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18795)
	; 30 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 18796)
	; 30 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 18797)
	; 29 bytes
	ldx #254
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18801)
	; 31 bytes
	ldx #235
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18803)
	; 30 bytes
	ldx #238
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18810)
	; 33 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 18811 || n = 18813)
	; 32..33 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 18812)
	; 33 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 18815)
	; 33 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 18816)
	; 34 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 18817)
	; 33 bytes
	ldx #224
	bit $24
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 18818)
	; 34 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 18820)
	; 33 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 18821)
	; 34 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 18822)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 18823)
	; 34 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 18824)
	; 29 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18825)
	; 29 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18826)
	; 29 bytes
	ldx #251
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.endif
.endif
.else
.if((n >= 18829) && (n < 18868))
.if((n >= 18829) && (n < 18848))
; 18829 to 18847
.if(n = 18829 || n = 18845)
	; 31..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n > 18829 && n < 18834) || n = 18839 || n = 18844 || n = 18846)
	; 31..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18834)
	; 32 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-29
.elseif(n = 18835)
	; 33 bytes
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
.elseif(n = 18841)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18842)
	; 31 bytes
	ldx #227
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18843)
	; 34 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 18847)
	; 33 bytes
	delay_n 124
	delay_n (n-124)
.endif
.else
; 18848 to 18867
.if(n = 18848)
	; 34 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 18849)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18850)
	; 33 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 18851 || n = 18853 || n = 18854)
	; 31..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 18852)
	; 32 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 18858)
	; 34 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 18859)
	; 34 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18860)
	; 33 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 18861)
	; 32 bytes
	ldx #230
	nop
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 18862)
	; 34 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18863)
	; 33 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 18864)
	; 34 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18865)
	; 33 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 18866)
	; 29 bytes
	ldx #245
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18867)
	; 34 bytes
	delay_n 41
	delay_n (n-41)
.endif
.endif
.else
; 18868 to 18906
.if(n = 18868 || n = 18872 || n = 18873 || n = 18875 || n = 18876 || n = 18879 || n = 18883 || n = 18884 || (n > 18885 && n < 18890) || n = 18902 || n = 18903 || n = 18905 || n = 18906)
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18869)
	; 30 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 18870)
	; 30 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 18871)
	; 29 bytes
	ldx #255
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18874)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18877)
	; 31 bytes
	ldx #242
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18878 || n = 18894)
	; 33..34 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18880)
	; 33 bytes
	ldx #217
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 18881)
	; 31 bytes
	ldx #236
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18882)
	; 30 bytes
	ldx #239
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18885 || n = 18904)
	; 31..32 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18890)
	; 34 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 18891 || n = 18892)
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
.elseif(n = 18896 || n = 18897)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 18898)
	; 34 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 18899)
	; 29 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18900)
	; 29 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18901)
	; 29 bytes
	ldx #252
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.endif
.endif
.endif
.else
.if((n >= 18907) && (n < 18985))
.if((n >= 18907) && (n < 18946))
; 18907 to 18945
.if(n = 18907 || n = 18908 || n = 18914 || n = 18915 || n = 18922 || n = 18926 || n = 18927 || (n > 18928 && n < 18932) || n = 18934 || n = 18939)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 18909)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18910 || n = 18911)
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
.elseif(n = 18916 || n = 18917)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18918)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 18919)
	; 33 bytes
	ldx #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-30
.elseif(n = 18920)
	; 32 bytes
	ldx #36 ;hides 'bit zp'
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-29
.elseif(n = 18921)
	; 33 bytes
	ldx #(n-1)/86
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 18923)
	; 31 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 18924)
	; 31 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 18925)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18928)
	; 32 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18932)
	; 34 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 18933)
	; 34 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 18935)
	; 34 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 18936 || n = 18937)
	; 34 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 18938)
	; 34 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 18940)
	; 34 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 18941)
	; 32 bytes
	ldx #232 ;hides 'inx'
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-29
.elseif(n = 18942)
	; 34 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 18943)
	; 29 bytes
	ldx #246
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18944)
	; 30 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	bit $24
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 18945)
	; 29 bytes
	ldx #0
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-25
.endif
.else
; 18946 to 18984
.if(n = 18946 || n = 18948 || n = 18958 || n = 18964 || n = 18979)
	; 31..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 18947 || (n > 18948 && n < 18953) || n = 18957 || n = 18959 || n = 18960 || n = 18963 || n = 18965 || n = 18966 || n = 18968 || n = 18977 || n = 18978 || (n > 18979 && n < 18984))
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 18953)
	; 30 bytes
	ldx #234 ;hides 'nop'
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18954)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 18955)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 18956)
	; 32 bytes
	ldx #223
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 18961)
	; 30 bytes
	ldx #240
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 18962)
	; 34 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 18967)
	; 33 bytes
	ldx #218
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 29 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18975)
	; 29 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 18976)
	; 29 bytes
	ldx #253
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 18984)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.endif
.endif
.else
.if((n >= 18985) && (n < 19024))
; 18985 to 19023
.if(n = 18985)
	; 33 bytes
	ldx #226
	bit $24
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 18986 || n = 19018)
	; 33..35 bytes
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
.elseif(n = 18989 || n = 18991)
	; 34 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 18990 || n = 18995)
	; 33 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 18992)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18993)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 18994 || n = 18996)
	; 34 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 18997)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 18998)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 18999)
	; 31 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19000)
	; 31 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19001)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19002 || n = 19003 || n = 19005 || n = 19010 || n = 19012 || n = 19013 || n = 19022)
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 19004 || n = 19011 || n = 19023)
	; 31..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19006)
	; 32 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-29
.elseif(n = 19007)
	; 33 bytes
	ldx #(n-1)/86
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19008)
	; 31 bytes
	ldx #229
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19009)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19014 || n = 19016)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 19015 || n = 19017)
	; 34 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 19019 || n = 19021)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 19020)
	; 29 bytes
	ldx #247
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.endif
.else
; 19024 to 19062
.if((n > 19023 && n < 19028) || n = 19035 || n = 19037 || n = 19038 || n = 19042 || (n > 19043 && n < 19047) || n = 19052 || n = 19053 || (n > 19054 && n < 19059))
	; 30..33 bytes
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
.elseif(n = 19030 || n = 19060 || n = 19061)
	; 33 bytes
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
	; 31 bytes
	ldx #244
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19034)
	; 34 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19036)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19039 || n = 19043 || n = 19054)
	; 31..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19040)
	; 30 bytes
	ldx #241
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19041)
	; 31 bytes
	ldx #238
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19047)
	; 33 bytes
	ldx #214
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19049)
	; 29 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 19050)
	; 29 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 19051)
	; 29 bytes
	ldx #254
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.endif
.endif
.endif
.endif
.else
.if((n >= 19063) && (n < 19219))
.if((n >= 19063) && (n < 19141))
.if((n >= 19063) && (n < 19102))
; 19063 to 19101
.if(n = 19063)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19064)
	; 34 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 19065 || n = 19078 || n = 19079 || (n > 19080 && n < 19084) || n = 19093 || n = 19095 || n = 19099 || n = 19101)
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 19066)
	; 34 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19067 || n = 19085 || n = 19098)
	; 33..34 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 19068)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 19069)
	; 33 bytes
	ldx #227
	bit $24
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19070 || n = 19072)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19071)
	; 34 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 19073)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 19074)
	; 34 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 19075)
	; 31 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19076)
	; 31 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19077)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19080 || n = 19094 || n = 19100)
	; 31..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19084)
	; 34 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19086)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19087)
	; 34 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19088)
	; 34 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19089)
	; 34 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 19090 || n = 19092)
	; 34 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 19091)
	; 31 bytes
	ldx #230
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19096)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 19097)
	; 29 bytes
	ldx #248
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.endif
.else
; 19102 to 19140
.if((n > 19101 && n < 19105) || n = 19113 || n = 19115 || n = 19121 || n = 19123 || n = 19127 || n = 19128 || (n > 19129 && n < 19134) || n = 19140)
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 19105 || n = 19134)
	; 33 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19106)
	; 34 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 19107)
	; 32 bytes
	ldx #233
	nop
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19108 || n = 19137)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19109 || n = 19138)
	; 33 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19110)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 19111)
	; 31 bytes
	ldx #245
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19112)
	; 34 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 19114 || n = 19120 || n = 19122 || n = 19129)
	; 31..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19116)
	; 34 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19117)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19118)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19119)
	; 30 bytes
	ldx #242
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19124)
	; 29 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 19125)
	; 29 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 19126)
	; 29 bytes
	ldx #255
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 19135 || n = 19136)
	; 33 bytes
	delay_n 10
	delay_n (n-10)
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
	ldx #220
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19142)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19143 || n = 19170)
	; 34 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19144 || n = 19164)
	; 33..34 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 19145 || n = 19147)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19146)
	; 34 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 19148)
	; 34 bytes
	delay_n 51
	delay_n (n-51)
.elseif(n = 19149 || n = 19150)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 19151)
	; 31 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19152)
	; 31 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19153)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19154 || n = 19155 || (n > 19156 && n < 19161) || n = 19169 || n = 19175 || n = 19176 || n = 19178 || n = 19179)
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19156 || n = 19177)
	; 31..32 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 19165)
	; 34 bytes
	delay_n 12
	delay_n (n-12)
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
.elseif(n = 19172)
	; 30 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 19173)
	; 30 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 19174)
	; 29 bytes
	ldx #249
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.endif
.else
; 19180 to 19218
.if(n = 19180 || n = 19181 || n = 19191 || n = 19193 || n = 19202 || n = 19203 || (n > 19204 && n < 19209) || n = 19215)
	; 30..34 bytes
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
.elseif(n = 19184 || n = 19210)
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
	; 31 bytes
	ldx #234 ;hides 'nop'
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19188)
	; 34 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19189)
	; 31 bytes
	ldx #246
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19190 || n = 19192 || n = 19204 || n = 19214)
	; 31..34 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #243
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19199)
	; 29 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19200)
	; 29 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-26
.elseif(n = 19201)
	; 29 bytes
	ldx #0
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 19211)
	; 32 bytes
	ldx #226
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19216)
	; 34 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 19217)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
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
.if(n = 19219 || n = 19230 || n = 19231 || (n > 19232 && n < 19237) || n = 19239 || n = 19244 || n = 19252 || n = 19253 || n = 19255 || n = 19256)
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19220 || n = 19246 || n = 19247)
	; 34 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19221 || n = 19248)
	; 33 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 19222)
	; 34 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19223)
	; 34 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 19224)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 19225)
	; 33 bytes
	ldx #216
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19226)
	; 34 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19227)
	; 31 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19228)
	; 31 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19229)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19232 || n = 19254)
	; 31..32 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19237)
	; 33 bytes
	ldx #229
	bit $24
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19238)
	; 34 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19240)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19241)
	; 34 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19242)
	; 34 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 19243)
	; 34 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 19245)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 19249)
	; 30 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 19250)
	; 30 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 19251)
	; 29 bytes
	ldx #250
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 19257)
	; 31 bytes
	ldx #232
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.endif
.else
; 19258 to 19296
.if(n = 19258 || n = 19259 || n = 19269 || n = 19273 || n = 19283 || n = 19285 || n = 19286 || n = 19294)
	; 32..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19260 || n = 19270 || n = 19274 || n = 19280 || n = 19282 || n = 19284)
	; 32..34 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 19265)
	; 33 bytes
	ldx #(n-1)/86
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19266)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19267)
	; 31 bytes
	ldx #247
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19268)
	; 34 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19271)
	; 32 bytes
	ldx #235
	nop
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19272)
	; 33 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-30
.elseif(n = 19275)
	; 34 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 19276)
	; 34 bytes
	delay_n 76
	delay_n (n-76)
.elseif(n = 19277)
	; 30 bytes
	ldx #244
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19278)
	; 34 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 19279)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19281)
	; 31 bytes
	ldx #241
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19287)
	; 34 bytes
	delay_n 86
	delay_n (n-86)
.elseif(n = 19288)
	; 34 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19290)
	; 34 bytes
	delay_n 116
	delay_n (n-116)
.elseif(n = 19291 || n = 19292)
	; 34 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19293)
	; 34 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 19295)
	; 34 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 19296)
	; 32 bytes
	ldx #227
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
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
.elseif(n = 19298 || n = 19306 || n = 19307 || (n > 19308 && n < 19312) || n = 19316 || n = 19319 || n = 19329 || n = 19330 || (n > 19331 && n < 19336))
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19299 || n = 19308 || n = 19318 || n = 19331)
	; 31..35 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19300)
	; 34 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 19301 || n = 19302)
	; 34 bytes
	delay_n 101
	delay_n (n-101)
.elseif(n = 19303)
	; 31 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19304)
	; 31 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19305)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19312)
	; 34 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 19313)
	; 34 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19314)
	; 33 bytes
	ldx #217
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19315)
	; 33 bytes
	ldx #222
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	; 33 bytes
	ldx #230
	bit $24
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19322)
	; 34 bytes
	delay_n 121
	delay_n (n-121)
.elseif((n > 19322 && n < 19326))
	; 33 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 19326)
	; 30 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 19327)
	; 30 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 19328)
	; 29 bytes
	ldx #251
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
	ldx #233
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19341)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 19342 || n = 19344 || n = 19346 || n = 19347 || n = 19349 || n = 19355)
	; 32..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 19343 || n = 19348)
	; 33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19345)
	; 31 bytes
	ldx #248
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19350)
	; 34 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 19351)
	; 33 bytes
	ldx #(n-1)/86
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19352)
	; 34 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19353)
	; 32 bytes
	ldx #236
	nop
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19354)
	; 34 bytes
	delay_n 26
	delay_n (n-26)
.endif
.else
; 19356 to 19375
.if(n = 19356)
	; 30 bytes
	ldx #245
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19357)
	; 34 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 19358 || (n > 19361 && n < 19366))
	; 31..33 bytes
	delay_n (n-2)
	nop
.elseif(n = 19359)
	; 32 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19360)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19361)
	; 31 bytes
	ldx #242
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.elseif(n = 19371 || n = 19372)
	; 34 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19373)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 19374)
	; 34 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19375)
	; 33 bytes
	delay_n 124
	delay_n (n-124)
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
.if(n = 19376 || n = 19377)
	; 34 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 19378)
	; 35 bytes
	delay_n 50
	delay_n (n-50)
.elseif(n = 19379)
	; 31 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19380)
	; 31 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19381)
	; 30 bytes
	ldx #(n-1)/76
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19382 || n = 19383 || (n > 19384 && n < 19389) || n = 19406 || n = 19407 || (n > 19408 && n < 19413))
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 19384 || n = 19408)
	; 31..32 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19389)
	; 34 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 19390 || n = 19392)
	; 34 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19391)
	; 34 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19393)
	; 34 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19394)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif((n > 19394 && n < 19398))
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
.elseif(n = 19400)
	; 35 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19401)
	; 34 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 19402)
	; 33 bytes
	ldx #223
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19403)
	; 30 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 19404)
	; 30 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 19405)
	; 29 bytes
	ldx #252
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.elseif(n = 19417 || n = 19420 || n = 19453)
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
.elseif(n = 19421)
	; 31 bytes
	ldx #234 ;hides 'nop'
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19422)
	; 31 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19423)
	; 31 bytes
	ldx #234
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19424 || n = 19425 || n = 19427 || n = 19428 || n = 19430 || n = 19437 || n = 19439 || n = 19440 || n = 19443 || n = 19445 || n = 19446)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19426 || n = 19438 || n = 19444)
	; 32..33 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #246
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19436)
	; 34 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19441)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19442)
	; 34 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19447)
	; 34 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 19448)
	; 33 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-30
.elseif(n = 19449)
	; 34 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 19450 || n = 19452)
	; 33..34 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 19451)
	; 34 bytes
	delay_n 46
	delay_n (n-46)
.endif
.endif
.else
.if((n >= 19454) && (n < 19493))
; 19454 to 19492
.if(n = 19454)
	; 34 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 19455)
	; 31 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19456)
	; 31 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 10
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19457)
	; 30 bytes
	ldx #0
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19458 || n = 19459 || (n > 19460 && n < 19464) || n = 19468 || n = 19470 || n = 19473 || n = 19478 || n = 19483 || n = 19484 || (n > 19485 && n < 19489) || n = 19491)
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19460 || n = 19485)
	; 31..32 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19464)
	; 34 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 19465 || n = 19490)
	; 33..34 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19466)
	; 32 bytes
	ldx #229
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19467 || n = 19492)
	; 33..34 bytes
	delay_n 10
	delay_n (n-10)
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
	ldx #(n-1)/91
	nop
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
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
.elseif(n = 19480)
	; 30 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 19481)
	; 30 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 19482)
	; 29 bytes
	ldx #253
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 19489)
	; 33 bytes
	ldx #224
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.endif
.else
; 19493 to 19531
.if(n = 19493)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19494 || n = 19497)
	; 33..35 bytes
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
.elseif(n = 19498)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19499)
	; 31 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19500)
	; 31 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19501)
	; 31 bytes
	ldx #250
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19502 || n = 19503 || n = 19505 || n = 19507 || n = 19508 || n = 19510 || n = 19512 || n = 19515 || n = 19516 || (n > 19517 && n < 19521) || n = 19523 || n = 19524 || n = 19526 || n = 19527 || n = 19530)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19504 || n = 19509 || n = 19517 || n = 19525)
	; 32..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19506)
	; 31 bytes
	ldx #235
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
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
	ldx #247
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19521)
	; 31 bytes
	ldx #244
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19522)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19528)
	; 34 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19529)
	; 33 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 19531)
	; 34 bytes
	delay_n 126
	delay_n (n-126)
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
	ldx #222
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-30
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
.elseif(n = 19544 || n = 19546)
	; 34..35 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19545)
	; 35 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19547)
	; 35 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19548)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19549)
	; 35 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 19550)
	; 35 bytes
	delay_n 36
	delay_n (n-36)
.endif
.else
; 19551 to 19570
.if(n = 19551)
	; 32 bytes
	ldx #230
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19552)
	; 35 bytes
	delay_n 196
	delay_n (n-196)
.elseif(n = 19553 || n = 19555 || n = 19560 || n = 19561 || (n > 19562 && n < 19567))
	; 30..34 bytes
	delay_n (n-2)
	nop
.elseif(n = 19554 || n = 19562)
	; 31..34 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19556)
	; 34 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19557)
	; 30 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 19558)
	; 30 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 19559)
	; 29 bytes
	ldx #254
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
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
.if(n = 19571 || n = 19574)
	; 33..35 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19572)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.elseif(n = 19573)
	; 33 bytes
	ldx #233
	bit $24
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19575)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19576)
	; 33 bytes
	ldx #225
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19577)
	; 31 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19578)
	; 31 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19579)
	; 31 bytes
	ldx #251
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19580 || n = 19581 || n = 19583 || n = 19584 || n = 19587 || n = 19591 || n = 19594 || n = 19595 || n = 19597 || n = 19598 || n = 19605 || n = 19607 || n = 19608)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19582 || n = 19592 || n = 19596 || n = 19602 || n = 19604 || n = 19606)
	; 32..34 bytes
	delay_n (n-3)
	ldx $A6
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
	ldx #236
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19590)
	; 34 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19593)
	; 30 bytes
	ldx #248
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19599)
	; 32 bytes
	ldx #239
	nop
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19600)
	; 34 bytes
	delay_n 41
	delay_n (n-41)
.elseif(n = 19601)
	; 31 bytes
	ldx #245
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19603)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19609)
	; 33 bytes
	ldx #(n-1)/86
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
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
.elseif(n = 19611 || n = 19612 || n = 19617 || n = 19622 || n = 19627)
	; 34..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19613)
	; 34 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 19614)
	; 35 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19615)
	; 34 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 19616)
	; 35 bytes
	delay_n 57
	delay_n (n-57)
.elseif(n = 19618)
	; 34 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 19619)
	; 35 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19620)
	; 34 bytes
	delay_n 61
	delay_n (n-61)
.elseif(n = 19621)
	; 35 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 19623)
	; 34 bytes
	delay_n 141
	delay_n (n-141)
.elseif(n = 19624)
	; 35 bytes
	ldx #211
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-31
.elseif(n = 19625)
	; 34 bytes
	delay_n 66
	delay_n (n-66)
.elseif(n = 19626)
	; 35 bytes
	delay_n 191
	delay_n (n-191)
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
.elseif(n = 19632 || n = 19637 || n = 19638 || (n > 19639 && n < 19644))
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19633)
	; 34 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19634)
	; 30 bytes
	ldx #<$2C ;hides 'bit $01A2'
	ldx #1
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 19635)
	; 30 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 19636)
	; 29 bytes
	ldx #255
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 19639)
	; 31 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 19651)
	; 35 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19652)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19653)
	; 34 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19654 || n = 19658 || n = 19659 || n = 19661 || n = 19664 || n = 19665 || n = 19673 || n = 19674 || (n > 19675 && n < 19680) || n = 19683 || n = 19685 || n = 19686)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19655)
	; 31 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19656)
	; 31 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19657)
	; 31 bytes
	ldx #252
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19660 || n = 19666 || n = 19675 || n = 19687)
	; 32..35 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19662)
	; 34 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19663)
	; 33 bytes
	ldx #226
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19667)
	; 34 bytes
	delay_n 31
	delay_n (n-31)
.elseif(n = 19668)
	; 34 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 19669)
	; 34 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-31
.elseif(n = 19670)
	; 31 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19671)
	; 31 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19672)
	; 30 bytes
	ldx #249
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19680)
	; 34 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 19681)
	; 31 bytes
	ldx #246
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19682)
	; 34 bytes
	delay_n 46
	delay_n (n-46)
.elseif(n = 19684)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.endif
.endif
.endif
.endif
.else
.if((n >= 19688) && (n < 19844))
.if((n >= 19688) && (n < 19766))
.if((n >= 19688) && (n < 19727))
; 19688 to 19726
.if(n = 19688 || n = 19689 || n = 19691 || n = 19699 || n = 19704 || n = 19709 || n = 19714 || n = 19715 || n = 19717 || n = 19718 || n = 19720)
	; 30..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19690)
	; 34 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 19692)
	; 34 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 19693)
	; 35 bytes
	delay_n 21
	delay_n (n-21)
.elseif(n = 19694 || n = 19723)
	; 33..35 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19695)
	; 33 bytes
	ldx #(n-1)/86
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19696 || n = 19725)
	; 33..35 bytes
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
.elseif(n = 19701 || n = 19702)
	; 34..35 bytes
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
.elseif(n = 19706 || n = 19707)
	; 34..35 bytes
	delay_n 71
	delay_n (n-71)
.elseif(n = 19708)
	; 35 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19710)
	; 34 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19711)
	; 30 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19712)
	; 30 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	nop
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-27
.elseif(n = 19713)
	; 29 bytes
	ldx #0
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-25
.elseif(n = 19716 || n = 19722)
	; 31..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19719)
	; 31 bytes
	ldx #24 ;hides 'clc'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19721)
	; 32 bytes
	ldx #232
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19724)
	; 33 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19726)
	; 34 bytes
	delay_n 167
	delay_n (n-167)
.endif
.else
.if((n >= 19727) && (n < 19746))
; 19727 to 19745
.if(n = 19727)
	; 34 bytes
	delay_n 91
	delay_n (n-91)
.elseif(n = 19728)
	; 35 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19729)
	; 34 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19730)
	; 34 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19731 || n = 19736 || n = 19737 || n = 19739 || n = 19743)
	; 32..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19732)
	; 34 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19733)
	; 31 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19734)
	; 31 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19735)
	; 31 bytes
	ldx #253
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19738)
	; 33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19740)
	; 34 bytes
	delay_n 181
	delay_n (n-181)
.elseif(n = 19741)
	; 33 bytes
	ldx #235
	bit $24
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19742)
	; 34 bytes
	delay_n 106
	delay_n (n-106)
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
.if(n = 19746 || n = 19752 || n = 19753 || n = 19756 || n = 19757)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19747)
	; 34 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 19748)
	; 34 bytes
	ldx #(n-1)/91
	nop
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-30
.elseif(n = 19749)
	; 31 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19750)
	; 31 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19751)
	; 30 bytes
	ldx #250
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19754 || n = 19758 || n = 19764)
	; 32..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19755)
	; 31 bytes
	ldx #238
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19759)
	; 33 bytes
	ldx #222
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19760)
	; 33 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 19761)
	; 31 bytes
	ldx #247
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19762)
	; 34 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 19763)
	; 32 bytes
	ldx #241
	nop
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19765)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.endif
.endif
.endif
.else
.if((n >= 19766) && (n < 19805))
.if((n >= 19766) && (n < 19785))
; 19766 to 19784
.if(n = 19766 || n = 19767 || (n > 19768 && n < 19772) || n = 19776 || n = 19783)
	; 32..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19768)
	; 33 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 19780 || n = 19782)
	; 34..35 bytes
	delay_n 146
	delay_n (n-146)
.elseif(n = 19781)
	; 33 bytes
	ldx #(n-1)/86
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19784)
	; 34 bytes
	delay_n 71
	delay_n (n-71)
.endif
.else
; 19785 to 19804
.if(n = 19785 || n = 19786 || n = 19791)
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
.elseif(n = 19790 || n = 19792)
	; 34..35 bytes
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
.elseif(n = 19795 || n = 19797)
	; 34..35 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 19796)
	; 34 bytes
	delay_n 124
	delay_n (n-124)
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
	ldx #36 ;hides 'bit zp'
	nop
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-31
.elseif(n = 19801)
	; 34 bytes
	ldx #225
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-30
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
	ldx #233
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19807)
	; 34 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19808 || n = 19810 || n = 19814 || n = 19815 || n = 19817 || n = 19818 || n = 19821 || n = 19826 || n = 19831 || n = 19832 || (n > 19833 && n < 19837) || n = 19840 || n = 19842 || n = 19843)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19809)
	; 34 bytes
	delay_n 96
	delay_n (n-96)
.elseif(n = 19811)
	; 31 bytes
	ldx #<$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19812)
	; 31 bytes
	bit <$2C ;hides 'bit $02A2'
	ldx #2
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19813)
	; 31 bytes
	ldx #254
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19816 || n = 19833)
	; 32..33 bytes
	delay_n (n-3)
	ldx $A6
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
	; 33 bytes
	ldx #236
	bit $24
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19827)
	; 34 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19828)
	; 31 bytes
	ldx #<$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19829)
	; 31 bytes
	bit <$2C ;hides 'bit $05A2'
	ldx #5
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19830)
	; 30 bytes
	ldx #251
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19837)
	; 33 bytes
	ldx #228
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19838)
	; 31 bytes
	ldx #239
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19839)
	; 34 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 19841)
	; 31 bytes
	ldx #248
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.endif
.endif
.endif
.else
.if((n >= 19844) && (n < 19922))
.if((n >= 19844) && (n < 19883))
.if((n >= 19844) && (n < 19863))
; 19844 to 19862
.if(n = 19844 || n = 19849)
	; 33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19845)
	; 32 bytes
	ldx #242
	nop
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19846)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19847 || n = 19848 || (n > 19849 && n < 19854) || n = 19861)
	; 32..35 bytes
	delay_n (n-2)
	nop
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
	; 33 bytes
	ldx #232 ;hides 'inx'
	nop
	bit $24
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	dex
	Jne *-30
.elseif(n = 19866)
	; 34 bytes
	delay_n 153
	delay_n (n-153)
.elseif(n = 19867)
	; 33 bytes
	ldx #(n-1)/86
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19868)
	; 35 bytes
	delay_n 155
	delay_n (n-155)
.elseif(n = 19869)
	; 34 bytes
	delay_n 156
	delay_n (n-156)
.elseif(n = 19870)
	; 35 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19871 || n = 19876 || n = 19881)
	; 35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19872)
	; 35 bytes
	delay_n 121
	delay_n (n-121)
.elseif(n = 19873)
	; 35 bytes
	ldx #216
	bit $24
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-31
.elseif(n = 19874)
	; 34 bytes
	delay_n 161
	delay_n (n-161)
.elseif(n = 19875)
	; 34 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 19877)
	; 35 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 19878 || n = 19879)
	; 34..35 bytes
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
.if(n = 19883 || n = 19884)
	; 34..35 bytes
	delay_n 171
	delay_n (n-171)
.elseif(n = 19885)
	; 35 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 19886 || n = 19892 || n = 19893 || (n > 19894 && n < 19899) || n = 19901 || n = 19905 || n = 19906 || n = 19910 || n = 19911 || (n > 19912 && n < 19917) || n = 19918)
	; 31..36 bytes
	delay_n (n-2)
	nop
.elseif(n = 19887)
	; 34 bytes
	delay_n 174
	delay_n (n-174)
.elseif(n = 19888)
	; 35 bytes
	delay_n 176
	delay_n (n-176)
.elseif(n = 19889)
	; 31 bytes
	ldx #234 ;hides 'nop'
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19890)
	; 31 bytes
	bit <$2C ;hides 'bit $01A2'
	ldx #1
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19891)
	; 31 bytes
	ldx #255
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19894 || n = 19912)
	; 32..33 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19899)
	; 34 bytes
	delay_n 186
	delay_n (n-186)
.elseif(n = 19900)
	; 35 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19902)
	; 35 bytes
	delay_n 151
	delay_n (n-151)
.elseif(n = 19903)
	; 35 bytes
	ldx #214
	delay_n 10
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-31
.elseif(n = 19904)
	; 34 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19907)
	; 31 bytes
	ldx #<$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19908)
	; 31 bytes
	bit <$2C ;hides 'bit $04A2'
	ldx #4
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19909)
	; 30 bytes
	ldx #252
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19917)
	; 34 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19919)
	; 32 bytes
	ldx #<$2C ;hides 'bit $07A2'
	ldx #7
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-29
.elseif(n = 19920)
	; 32 bytes
	bit <$2C ;hides 'bit $07A2'
	ldx #7
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-29
.elseif(n = 19921)
	; 31 bytes
	ldx #240
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.endif
.endif
.else
.if((n >= 19922) && (n < 19961))
; 19922 to 19960
.if(n = 19922 || n = 19923 || n = 19925 || n = 19926 || n = 19928 || n = 19929 || (n > 19930 && n < 19935) || n = 19939 || n = 19955 || n = 19957)
	; 32..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19924)
	; 33 bytes
	ldx #229
	delay_n 10
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19927)
	; 31 bytes
	ldx #(n-1)/81
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19930 || n = 19940)
	; 33..35 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19935)
	; 35 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19936 || n = 19944)
	; 35..36 bytes
	delay_n 106
	delay_n (n-106)
.elseif(n = 19937)
	; 33 bytes
	ldx #224
	.repeat 7
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19938)
	; 35 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 19941 || n = 19949)
	; 35..36 bytes
	delay_n 111
	delay_n (n-111)
.elseif(n = 19942)
	; 35 bytes
	delay_n 191
	delay_n (n-191)
.elseif(n = 19943)
	; 36 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19945)
	; 35 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19946)
	; 35 bytes
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
	; 33 bytes
	ldx #24 ;hides 'clc'
	bit $24
	delay_n 10
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-30
.elseif(n = 19952 || n = 19954)
	; 34..35 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 19953)
	; 33 bytes
	ldx #(n-1)/86
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-29
.elseif(n = 19956)
	; 35 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 19958)
	; 36 bytes
	delay_n 31
	delay_n (n-31)
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
; 19961 to 20000
.if(n = 19961)
	; 35 bytes
	delay_n 131
	delay_n (n-131)
.elseif(n = 19962)
	; 35 bytes
	delay_n 124
	delay_n (n-124)
.elseif(n = 19963)
	; 36 bytes
	delay_n 36
	delay_n (n-36)
.elseif(n = 19964)
	; 36 bytes
	delay_n 126
	delay_n (n-126)
.elseif(n = 19965)
	; 35 bytes
	delay_n 56
	delay_n (n-56)
.elseif(n = 19966)
	; 35 bytes
	delay_n 136
	delay_n (n-136)
.elseif(n = 19967)
	; 31 bytes
	ldx #<$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19968)
	; 31 bytes
	bit <$2C ;hides 'bit $00A2'
	ldx #0
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19969)
	; 31 bytes
	ldx #0
	bit $24
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-27
.elseif(n = 19970 || n = 19971 || (n > 19972 && n < 19976) || n = 19978 || n = 19980 || n = 19982 || n = 19989 || n = 19990 || (n > 19991 && n < 19996) || n = 19997)
	; 31..35 bytes
	delay_n (n-2)
	nop
.elseif(n = 19972 || n = 19979 || n = 19991)
	; 32..34 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 19976)
	; 32 bytes
	ldx #235
	nop
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-28
.elseif(n = 19977)
	; 34 bytes
	ldx #227
	nop
	bit $24
	.repeat 2
	 rol $36,X
	 ror $36,X
	.endrepeat
	.repeat 9
	 rol $36,X
	.endrepeat
	dex
	Jne *-30
.elseif(n = 19981)
	; 35 bytes
	delay_n 151
	delay_n (n-151)
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
	; 31 bytes
	ldx #<$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19987)
	; 31 bytes
	bit <$2C ;hides 'bit $03A2'
	ldx #3
	delay_n 10
	.repeat 5
	 rol $36,X
	 ror $36,X
	.endrepeat
	inx
	Jne *-28
.elseif(n = 19988)
	; 30 bytes
	ldx #253
	nop
	.repeat 6
	 rol $36,X
	 ror $36,X
	.endrepeat
	dex
	Jne *-26
.elseif(n = 19996)
	; 34 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19998)
	; 34 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19999)
	; 32 bytes
	ldx #<$2C ;hides 'bit $06A2'
	ldx #6
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-29
.elseif(n = 20000)
	; 32 bytes
	bit <$2C ;hides 'bit $06A2'
	ldx #6
	nop
	bit $24
	delay_n 12
	.repeat 9
	 rol $36,X
	.endrepeat
	inx
	Jne *-29
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
