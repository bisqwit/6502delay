; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber X, Z+N, and V
; Will preserve A, Y, S, C, D, and I (interrupt-disable flag)
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
; - 7 bytes or less when n <= 32
; - 8 bytes or less when n <= 37
; - 9 bytes or less when n <= 298
; -10 bytes or less when n <= 300
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.3, max=4
; - When 11 <= n <= 20: min=3, avg=4.0, max=5
; - When 21 <= n <= 30: min=5, avg=5.9, max=7
; - When 31 <= n <= 50: min=7, avg=8.6, max=9
; - When 51 <= n <= 75: min=9, avg=9.0, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=9, avg=9.0, max=9
; - When 201 <= n <= 300: min=9, avg=9.0, max=10
; - When 301 <= n <= 500: min=9, avg=11.0, max=11
; - When 501 <= n <= 750: min=9, avg=11.0, max=11
; - When 751 <= n <= 1000: min=9, avg=11.0, max=11
; - When 1001 <= n <= 2000: min=9, avg=11.0, max=11
; - When 2001 <= n <= 3000: min=9, avg=11.0, max=11
; - When 3001 <= n <= 5000: min=9, avg=11.0, max=11
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
; 1 to 1250
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	bit $24
.elseif(n = 4 || n = 8 || n = 16 || n = 17 || n = 31 || n = 32 || n = 34 || (n > 298 && n < 303) || n = 308 || n = 565 || n = 822 || n = 1079)
	; 2..11 bytes
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
.elseif(n = 9 || n = 11 || n = 19 || (n > 20 && n < 24) || n = 33 || n = 35 || n = 37)
	; 3..8 bytes
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
.elseif(n = 20 || n = 27 || n = 29 || n = 30 || n = 39)
	; 5..9 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 26 || n = 28 || n = 38)
	; 6..9 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 36)
	; 8 bytes
	inc $F6,X
	delay_n (n-6)
.elseif((n > 39 && n < 43))
	; 9 bytes
	php
	tax
	lda #n-40
	jsr delay_a_27_clocks
	txa
	plp
.elseif((n > 42 && n < 299))
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
	pla
	plp
.elseif((n > 302 && n < 306))
	; 11 bytes
	php
	pha
	php
	lda #n-50
	jsr delay_a_27_clocks
	plp
	pla
	plp
.elseif(n = 306)
	; 9 bytes
	php
	pha
	lax #1
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 307 || (n > 308 && n < 563) || n = 564 || (n > 565 && n < 820) || n = 821 || (n > 822 && n < 1077) || n = 1078 || (n > 1079 && n < 1251))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 563)
	; 9 bytes
	php
	pha
	lax #2
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 820)
	; 9 bytes
	php
	pha
	lax #3
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1077)
	; 9 bytes
	php
	pha
	lax #4
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 1334) || n = 1335 || (n > 1336 && n < 1591) || n = 1592 || (n > 1593 && n < 1848) || n = 1849 || (n > 1850 && n < 2105) || n = 2106 || (n > 2107 && n < 2362) || n = 2363 || (n > 2364 && n < 2501))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1334)
	; 9 bytes
	php
	pha
	lax #5
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1336 || n = 1593 || n = 1850 || n = 2107 || n = 2364)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1591)
	; 9 bytes
	php
	pha
	lax #6
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1848)
	; 9 bytes
	php
	pha
	lax #7
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2105)
	; 9 bytes
	php
	pha
	lax #8
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2362)
	; 9 bytes
	php
	pha
	lax #9
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 2619) || n = 2620 || (n > 2621 && n < 2876) || n = 2877 || (n > 2878 && n < 3133) || n = 3134 || (n > 3135 && n < 3390) || n = 3391 || (n > 3392 && n < 3647) || n = 3648 || (n > 3649 && n < 3904) || n = 3905 || (n > 3906 && n < 4161) || n = 4162 || (n > 4163 && n < 4418) || n = 4419 || (n > 4420 && n < 4675) || n = 4676 || (n > 4677 && n < 4932) || n = 4933 || (n > 4934 && n < 5001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2619)
	; 9 bytes
	php
	pha
	lax #10
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2621 || n = 2878 || n = 3135 || n = 3392 || n = 3649 || n = 3906 || n = 4163 || n = 4420 || n = 4677 || n = 4934)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2876)
	; 9 bytes
	php
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3133)
	; 9 bytes
	php
	pha
	lax #12
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3390)
	; 9 bytes
	php
	pha
	lax #13
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3647)
	; 9 bytes
	php
	pha
	lax #14
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3904)
	; 9 bytes
	php
	pha
	lax #15
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4161)
	; 9 bytes
	php
	pha
	lax #16
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4418)
	; 9 bytes
	php
	pha
	lax #17
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4675)
	; 9 bytes
	php
	pha
	lax #18
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4932)
	; 9 bytes
	php
	pha
	lax #19
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5189) || n = 5190 || (n > 5191 && n < 5446) || n = 5447 || (n > 5448 && n < 5703) || n = 5704 || (n > 5705 && n < 5960) || n = 5961 || (n > 5962 && n < 6217) || n = 6218 || (n > 6219 && n < 6474) || n = 6475 || (n > 6476 && n < 6731) || n = 6732 || (n > 6733 && n < 6988) || n = 6989 || (n > 6990 && n < 7245) || n = 7246 || (n > 7247 && n < 7502) || n = 7503 || (n > 7504 && n < 7759) || n = 7760 || (n > 7761 && n < 8016) || n = 8017 || (n > 8018 && n < 8273) || n = 8274 || (n > 8275 && n < 8530) || n = 8531 || (n > 8532 && n < 8787) || n = 8788 || (n > 8789 && n < 9044) || n = 9045 || (n > 9046 && n < 9301) || n = 9302 || (n > 9303 && n < 9558) || n = 9559 || (n > 9560 && n < 9815) || n = 9816 || (n > 9817 && n < 10001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5189)
	; 9 bytes
	php
	pha
	lax #20
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5191 || n = 5448 || n = 5705 || n = 5962 || n = 6219 || n = 6476 || n = 6733 || n = 6990 || n = 7247 || n = 7504 || n = 7761 || n = 8018 || n = 8275 || n = 8532 || n = 8789 || n = 9046 || n = 9303 || n = 9560 || n = 9817)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5446)
	; 9 bytes
	php
	pha
	lax #21
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5703)
	; 9 bytes
	php
	pha
	lax #22
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5960)
	; 9 bytes
	php
	pha
	lax #23
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6217)
	; 9 bytes
	php
	pha
	lax #24
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6474)
	; 9 bytes
	php
	pha
	lax #25
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6731)
	; 9 bytes
	php
	pha
	lax #26
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6988)
	; 9 bytes
	php
	pha
	lax #27
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7245)
	; 9 bytes
	php
	pha
	lax #28
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7502)
	; 9 bytes
	php
	pha
	lax #29
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7759)
	; 9 bytes
	php
	pha
	lax #30
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8016)
	; 9 bytes
	php
	pha
	lax #31
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8273)
	; 9 bytes
	php
	pha
	lax #32
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8530)
	; 9 bytes
	php
	pha
	lax #33
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8787)
	; 9 bytes
	php
	pha
	lax #34
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9044)
	; 9 bytes
	php
	pha
	lax #35
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9301)
	; 9 bytes
	php
	pha
	lax #36
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9558)
	; 9 bytes
	php
	pha
	lax #37
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9815)
	; 9 bytes
	php
	pha
	lax #38
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10072) || n = 10073 || (n > 10074 && n < 10329) || n = 10330 || (n > 10331 && n < 10586) || n = 10587 || (n > 10588 && n < 10843) || n = 10844 || (n > 10845 && n < 11100) || n = 11101 || (n > 11102 && n < 11357) || n = 11358 || (n > 11359 && n < 11614) || n = 11615 || (n > 11616 && n < 11871) || n = 11872 || (n > 11873 && n < 12128) || n = 12129 || (n > 12130 && n < 12385) || n = 12386 || (n > 12387 && n < 12642) || n = 12643 || (n > 12644 && n < 12899) || n = 12900 || (n > 12901 && n < 13156) || n = 13157 || (n > 13158 && n < 13413) || n = 13414 || (n > 13415 && n < 13670) || n = 13671 || (n > 13672 && n < 13927) || n = 13928 || (n > 13929 && n < 14184) || n = 14185 || (n > 14186 && n < 14441) || n = 14442 || (n > 14443 && n < 14698) || n = 14699 || (n > 14700 && n < 14955) || n = 14956 || (n > 14957 && n < 15001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10072)
	; 9 bytes
	php
	pha
	lax #39
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10074 || n = 10331 || n = 10588 || n = 10845 || n = 11102 || n = 11359 || n = 11616 || n = 11873 || n = 12130 || n = 12387 || n = 12644 || n = 12901 || n = 13158 || n = 13415 || n = 13672 || n = 13929 || n = 14186 || n = 14443 || n = 14700 || n = 14957)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10329)
	; 9 bytes
	php
	pha
	lax #40
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10586)
	; 9 bytes
	php
	pha
	lax #41
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10843)
	; 9 bytes
	php
	pha
	lax #42
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11100)
	; 9 bytes
	php
	pha
	lax #43
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11357)
	; 9 bytes
	php
	pha
	lax #44
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11614)
	; 9 bytes
	php
	pha
	lax #45
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11871)
	; 9 bytes
	php
	pha
	lax #46
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12128)
	; 9 bytes
	php
	pha
	lax #47
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12385)
	; 9 bytes
	php
	pha
	lax #48
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12642)
	; 9 bytes
	php
	pha
	lax #49
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12899)
	; 9 bytes
	php
	pha
	lax #50
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13156)
	; 9 bytes
	php
	pha
	lax #51
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13413)
	; 9 bytes
	php
	pha
	lax #52
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13670)
	; 9 bytes
	php
	pha
	lax #53
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13927)
	; 9 bytes
	php
	pha
	lax #54
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14184)
	; 9 bytes
	php
	pha
	lax #55
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14441)
	; 9 bytes
	php
	pha
	lax #56
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14698)
	; 9 bytes
	php
	pha
	lax #57
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14955)
	; 9 bytes
	php
	pha
	lax #58
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15212) || n = 15213 || (n > 15214 && n < 15469) || n = 15470 || (n > 15471 && n < 15726) || n = 15727 || (n > 15728 && n < 15983) || n = 15984 || (n > 15985 && n < 16240) || n = 16241 || (n > 16242 && n < 16497) || n = 16498 || (n > 16499 && n < 16754) || n = 16755 || (n > 16756 && n < 17011) || n = 17012 || (n > 17013 && n < 17268) || n = 17269 || (n > 17270 && n < 17525) || n = 17526 || (n > 17527 && n < 17782) || n = 17783 || (n > 17784 && n < 18039) || n = 18040 || (n > 18041 && n < 18296) || n = 18297 || (n > 18298 && n < 18553) || n = 18554 || (n > 18555 && n < 18810) || n = 18811 || (n > 18812 && n < 19067) || n = 19068 || (n > 19069 && n < 19324) || n = 19325 || (n > 19326 && n < 19581) || n = 19582 || (n > 19583 && n < 19838) || n = 19839 || (n > 19840 && n < 20001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15212)
	; 9 bytes
	php
	pha
	lax #59
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15214 || n = 15471 || n = 15728 || n = 15985 || n = 16242 || n = 16499 || n = 16756 || n = 17013 || n = 17270 || n = 17527 || n = 17784 || n = 18041 || n = 18298 || n = 18555 || n = 18812 || n = 19069 || n = 19326 || n = 19583 || n = 19840)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15469)
	; 9 bytes
	php
	pha
	lax #60
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15726)
	; 9 bytes
	php
	pha
	lax #61
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15983)
	; 9 bytes
	php
	pha
	lax #62
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16240)
	; 9 bytes
	php
	pha
	lax #63
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16497)
	; 9 bytes
	php
	pha
	lax #64
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16754)
	; 9 bytes
	php
	pha
	lax #65
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17011)
	; 9 bytes
	php
	pha
	lax #66
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17268)
	; 9 bytes
	php
	pha
	lax #67
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17525)
	; 9 bytes
	php
	pha
	lax #68
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17782)
	; 9 bytes
	php
	pha
	lax #69
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18039)
	; 9 bytes
	php
	pha
	lax #70
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18296)
	; 9 bytes
	php
	pha
	lax #71
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18553)
	; 9 bytes
	php
	pha
	lax #72
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18810)
	; 9 bytes
	php
	pha
	lax #73
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19067)
	; 9 bytes
	php
	pha
	lax #74
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19324)
	; 9 bytes
	php
	pha
	lax #75
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19581)
	; 9 bytes
	php
	pha
	lax #76
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19838)
	; 9 bytes
	php
	pha
	lax #77
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.endif
.endif

.endif
	rest
.endmacro
