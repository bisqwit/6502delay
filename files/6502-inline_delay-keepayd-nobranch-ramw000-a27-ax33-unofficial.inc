; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber X, Z+N, C, and V
; Will preserve A, Y, S, D, and I (interrupt-disable flag)
;
; WARNING: May write garbage into $00-$FF.
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

; Requires a disposable zeropage variable @zptemp.
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
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 9
; - 4 bytes or less when n <= 17
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 291
; - 8 bytes or less when n <= 293
; - 9 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.3, max=4
; - When 11 <= n <= 20: min=3, avg=4.0, max=5
; - When 21 <= n <= 30: min=5, avg=5.9, max=7
; - When 31 <= n <= 50: min=7, avg=7.0, max=7
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.1, max=9
; - When 301 <= n <= 500: min=8, avg=9.0, max=9
; - When 501 <= n <= 750: min=7, avg=9.0, max=9
; - When 751 <= n <= 1000: min=7, avg=9.0, max=9
; - When 1001 <= n <= 2000: min=7, avg=9.0, max=9
; - When 2001 <= n <= 3000: min=7, avg=9.0, max=9
; - When 3001 <= n <= 5000: min=7, avg=9.0, max=9
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
; 1 to 1250
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	cmp $C5
.elseif(n = 4 || n = 8 || n = 16 || n = 17 || n = 31 || n = 32 || n = 292 || n = 293 || n = 301 || n = 558 || n = 815 || n = 1072)
	; 2..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 2 bytes
	inc @zptemp
.elseif(n = 6)
	; 2 bytes
	inc $F6,X
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 9 || n = 11 || n = 19 || (n > 20 && n < 24))
	; 3..6 bytes
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
.elseif(n = 13)
	; 4 bytes
	php
	inc $F6,X
	plp
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 18 || n = 24 || n = 25)
	; 5..7 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 20 || n = 27 || n = 29 || n = 30)
	; 5..6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 26 || n = 28)
	; 6 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 32 && n < 36))
	; 7 bytes
	tax
	lda #n-33
	jsr delay_a_27_clocks
	txa
.elseif((n > 35 && n < 292))
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
.elseif((n > 293 && n < 299))
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 299)
	; 7 bytes
	pha
	lax #1
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 300 || (n > 301 && n < 556) || n = 557 || (n > 558 && n < 813) || n = 814 || (n > 815 && n < 1070) || n = 1071 || (n > 1072 && n < 1251))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 556)
	; 7 bytes
	pha
	lax #2
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 813)
	; 7 bytes
	pha
	lax #3
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1070)
	; 7 bytes
	pha
	lax #4
	jsr delay_256a_x_33_clocks
	pla
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 1327) || n = 1328 || (n > 1329 && n < 1584) || n = 1585 || (n > 1586 && n < 1841) || n = 1842 || (n > 1843 && n < 2098) || n = 2099 || (n > 2100 && n < 2355) || n = 2356 || (n > 2357 && n < 2501))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1327)
	; 7 bytes
	pha
	lax #5
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1329 || n = 1586 || n = 1843 || n = 2100 || n = 2357)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1584)
	; 7 bytes
	pha
	lax #6
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1841)
	; 7 bytes
	pha
	lax #7
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2098)
	; 7 bytes
	pha
	lax #8
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2355)
	; 7 bytes
	pha
	lax #9
	jsr delay_256a_x_33_clocks
	pla
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 2612) || n = 2613 || (n > 2614 && n < 2869) || n = 2870 || (n > 2871 && n < 3126) || n = 3127 || (n > 3128 && n < 3383) || n = 3384 || (n > 3385 && n < 3640) || n = 3641 || (n > 3642 && n < 3897) || n = 3898 || (n > 3899 && n < 4154) || n = 4155 || (n > 4156 && n < 4411) || n = 4412 || (n > 4413 && n < 4668) || n = 4669 || (n > 4670 && n < 4925) || n = 4926 || (n > 4927 && n < 5001))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2612)
	; 7 bytes
	pha
	lax #10
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2614 || n = 2871 || n = 3128 || n = 3385 || n = 3642 || n = 3899 || n = 4156 || n = 4413 || n = 4670 || n = 4927)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2869)
	; 7 bytes
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3126)
	; 7 bytes
	pha
	lax #12
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3383)
	; 7 bytes
	pha
	lax #13
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3640)
	; 7 bytes
	pha
	lax #14
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3897)
	; 7 bytes
	pha
	lax #15
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 4154)
	; 7 bytes
	pha
	lax #16
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 4411)
	; 7 bytes
	pha
	lax #17
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 4668)
	; 7 bytes
	pha
	lax #18
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 4925)
	; 7 bytes
	pha
	lax #19
	jsr delay_256a_x_33_clocks
	pla
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5182) || n = 5183 || (n > 5184 && n < 5439) || n = 5440 || (n > 5441 && n < 5696) || n = 5697 || (n > 5698 && n < 5953) || n = 5954 || (n > 5955 && n < 6210) || n = 6211 || (n > 6212 && n < 6467) || n = 6468 || (n > 6469 && n < 6724) || n = 6725 || (n > 6726 && n < 6981) || n = 6982 || (n > 6983 && n < 7238) || n = 7239 || (n > 7240 && n < 7495) || n = 7496 || (n > 7497 && n < 7752) || n = 7753 || (n > 7754 && n < 8009) || n = 8010 || (n > 8011 && n < 8266) || n = 8267 || (n > 8268 && n < 8523) || n = 8524 || (n > 8525 && n < 8780) || n = 8781 || (n > 8782 && n < 9037) || n = 9038 || (n > 9039 && n < 9294) || n = 9295 || (n > 9296 && n < 9551) || n = 9552 || (n > 9553 && n < 9808) || n = 9809 || (n > 9810 && n < 10001))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5182)
	; 7 bytes
	pha
	lax #20
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5184 || n = 5441 || n = 5698 || n = 5955 || n = 6212 || n = 6469 || n = 6726 || n = 6983 || n = 7240 || n = 7497 || n = 7754 || n = 8011 || n = 8268 || n = 8525 || n = 8782 || n = 9039 || n = 9296 || n = 9553 || n = 9810)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5439)
	; 7 bytes
	pha
	lax #21
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5696)
	; 7 bytes
	pha
	lax #22
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5953)
	; 7 bytes
	pha
	lax #23
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6210)
	; 7 bytes
	pha
	lax #24
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6467)
	; 7 bytes
	pha
	lax #25
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6724)
	; 7 bytes
	pha
	lax #26
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6981)
	; 7 bytes
	pha
	lax #27
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 7238)
	; 7 bytes
	pha
	lax #28
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 7495)
	; 7 bytes
	pha
	lax #29
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 7752)
	; 7 bytes
	pha
	lax #30
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8009)
	; 7 bytes
	pha
	lax #31
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8266)
	; 7 bytes
	pha
	lax #32
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8523)
	; 7 bytes
	pha
	lax #33
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8780)
	; 7 bytes
	pha
	lax #34
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 9037)
	; 7 bytes
	pha
	lax #35
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 9294)
	; 7 bytes
	pha
	lax #36
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 9551)
	; 7 bytes
	pha
	lax #37
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 9808)
	; 7 bytes
	pha
	lax #38
	jsr delay_256a_x_33_clocks
	pla
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10065) || n = 10066 || (n > 10067 && n < 10322) || n = 10323 || (n > 10324 && n < 10579) || n = 10580 || (n > 10581 && n < 10836) || n = 10837 || (n > 10838 && n < 11093) || n = 11094 || (n > 11095 && n < 11350) || n = 11351 || (n > 11352 && n < 11607) || n = 11608 || (n > 11609 && n < 11864) || n = 11865 || (n > 11866 && n < 12121) || n = 12122 || (n > 12123 && n < 12378) || n = 12379 || (n > 12380 && n < 12635) || n = 12636 || (n > 12637 && n < 12892) || n = 12893 || (n > 12894 && n < 13149) || n = 13150 || (n > 13151 && n < 13406) || n = 13407 || (n > 13408 && n < 13663) || n = 13664 || (n > 13665 && n < 13920) || n = 13921 || (n > 13922 && n < 14177) || n = 14178 || (n > 14179 && n < 14434) || n = 14435 || (n > 14436 && n < 14691) || n = 14692 || (n > 14693 && n < 14948) || n = 14949 || (n > 14950 && n < 15001))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10065)
	; 7 bytes
	pha
	lax #39
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10067 || n = 10324 || n = 10581 || n = 10838 || n = 11095 || n = 11352 || n = 11609 || n = 11866 || n = 12123 || n = 12380 || n = 12637 || n = 12894 || n = 13151 || n = 13408 || n = 13665 || n = 13922 || n = 14179 || n = 14436 || n = 14693 || n = 14950)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 10322)
	; 7 bytes
	pha
	lax #40
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10579)
	; 7 bytes
	pha
	lax #41
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10836)
	; 7 bytes
	pha
	lax #42
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11093)
	; 7 bytes
	pha
	lax #43
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11350)
	; 7 bytes
	pha
	lax #44
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11607)
	; 7 bytes
	pha
	lax #45
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11864)
	; 7 bytes
	pha
	lax #46
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12121)
	; 7 bytes
	pha
	lax #47
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12378)
	; 7 bytes
	pha
	lax #48
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12635)
	; 7 bytes
	pha
	lax #49
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12892)
	; 7 bytes
	pha
	lax #50
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13149)
	; 7 bytes
	pha
	lax #51
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13406)
	; 7 bytes
	pha
	lax #52
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13663)
	; 7 bytes
	pha
	lax #53
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13920)
	; 7 bytes
	pha
	lax #54
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 14177)
	; 7 bytes
	pha
	lax #55
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 14434)
	; 7 bytes
	pha
	lax #56
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 14691)
	; 7 bytes
	pha
	lax #57
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 14948)
	; 7 bytes
	pha
	lax #58
	jsr delay_256a_x_33_clocks
	pla
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15205) || n = 15206 || (n > 15207 && n < 15462) || n = 15463 || (n > 15464 && n < 15719) || n = 15720 || (n > 15721 && n < 15976) || n = 15977 || (n > 15978 && n < 16233) || n = 16234 || (n > 16235 && n < 16490) || n = 16491 || (n > 16492 && n < 16747) || n = 16748 || (n > 16749 && n < 17004) || n = 17005 || (n > 17006 && n < 17261) || n = 17262 || (n > 17263 && n < 17518) || n = 17519 || (n > 17520 && n < 17775) || n = 17776 || (n > 17777 && n < 18032) || n = 18033 || (n > 18034 && n < 18289) || n = 18290 || (n > 18291 && n < 18546) || n = 18547 || (n > 18548 && n < 18803) || n = 18804 || (n > 18805 && n < 19060) || n = 19061 || (n > 19062 && n < 19317) || n = 19318 || (n > 19319 && n < 19574) || n = 19575 || (n > 19576 && n < 19831) || n = 19832 || (n > 19833 && n < 20001))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15205)
	; 7 bytes
	pha
	lax #59
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15207 || n = 15464 || n = 15721 || n = 15978 || n = 16235 || n = 16492 || n = 16749 || n = 17006 || n = 17263 || n = 17520 || n = 17777 || n = 18034 || n = 18291 || n = 18548 || n = 18805 || n = 19062 || n = 19319 || n = 19576 || n = 19833)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 15462)
	; 7 bytes
	pha
	lax #60
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15719)
	; 7 bytes
	pha
	lax #61
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15976)
	; 7 bytes
	pha
	lax #62
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 16233)
	; 7 bytes
	pha
	lax #63
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 16490)
	; 7 bytes
	pha
	lax #64
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 16747)
	; 7 bytes
	pha
	lax #65
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17004)
	; 7 bytes
	pha
	lax #66
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17261)
	; 7 bytes
	pha
	lax #67
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17518)
	; 7 bytes
	pha
	lax #68
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17775)
	; 7 bytes
	pha
	lax #69
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18032)
	; 7 bytes
	pha
	lax #70
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18289)
	; 7 bytes
	pha
	lax #71
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18546)
	; 7 bytes
	pha
	lax #72
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18803)
	; 7 bytes
	pha
	lax #73
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 19060)
	; 7 bytes
	pha
	lax #74
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 19317)
	; 7 bytes
	pha
	lax #75
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 19574)
	; 7 bytes
	pha
	lax #76
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 19831)
	; 7 bytes
	pha
	lax #77
	jsr delay_256a_x_33_clocks
	pla
.endif
.endif
.endif

.endif
	rest
.endmacro
