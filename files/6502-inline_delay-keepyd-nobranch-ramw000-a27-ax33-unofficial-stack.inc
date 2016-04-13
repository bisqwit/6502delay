; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, X, S, Z+N, C, and V
; Will preserve Y, D, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
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
; - 1 bytes or less when n <= 4
; - 2 bytes or less when n <= 8
; - 3 bytes or less when n <= 12
; - 4 bytes or less when n <= 19
; - 5 bytes or less when n <= 23
; - 6 bytes or less when n <= 288
; - 7 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=1.9, max=3
; - When 11 <= n <= 20: min=3, avg=3.7, max=5
; - When 21 <= n <= 30: min=5, avg=5.5, max=6
; - When 31 <= n <= 50: min=5, avg=5.0, max=5
; - When 51 <= n <= 75: min=5, avg=5.0, max=5
; - When 76 <= n <= 100: min=5, avg=5.0, max=5
; - When 101 <= n <= 200: min=5, avg=5.0, max=5
; - When 201 <= n <= 300: min=5, avg=5.2, max=7
; - When 301 <= n <= 500: min=7, avg=7.0, max=7
; - When 501 <= n <= 750: min=5, avg=7.0, max=7
; - When 751 <= n <= 1000: min=5, avg=7.0, max=7
; - When 1001 <= n <= 2000: min=5, avg=7.0, max=7
; - When 2001 <= n <= 3000: min=5, avg=7.0, max=7
; - When 3001 <= n <= 5000: min=5, avg=7.0, max=7
; - When 5001 <= n <= 7500: min=5, avg=7.0, max=7
; - When 7501 <= n <= 10000: min=5, avg=7.0, max=7
; - When 10001 <= n <= 20000: min=5, avg=7.0, max=7
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
; 1 to 625
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
.elseif(n = 8 || n = 291)
	; 2..7 bytes
	delay_n (n-4)
	pla
.elseif(n = 9 || n = 10 || n = 21 || n = 22 || n = 25)
	; 3..6 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 11)
	; 3 bytes
	delay_n (n-3)
	pha
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
.elseif(n = 16 || n = 17 || n = 285 || n = 286 || n = 294 || n = 551)
	; 4..6 bytes
	delay_n (n-2)
	nop
.elseif((n > 17 && n < 21) || n = 23 || n = 27)
	; 4..6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 24)
	; 6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 26 || n = 28)
	; 6 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 28 && n < 285))
	; 5 bytes
	lda #n-29
	jsr delay_a_27_clocks
.elseif(n = 287 || n = 295 || n = 552)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 288 || n = 296 || n = 553)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 289 || n = 290)
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 292)
	; 5 bytes
	lax #1
	jsr delay_256a_x_33_clocks
.elseif(n = 293 || (n > 296 && n < 549) || n = 550 || (n > 553 && n < 626))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 549)
	; 5 bytes
	lax #2
	jsr delay_256a_x_33_clocks
.endif
.else
; 626 to 1250
.if((n > 625 && n < 806) || n = 807 || (n > 810 && n < 1063) || n = 1064 || (n > 1067 && n < 1251))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 806)
	; 5 bytes
	lax #3
	jsr delay_256a_x_33_clocks
.elseif(n = 808 || n = 1065)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 809 || n = 1066)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 810 || n = 1067)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 1063)
	; 5 bytes
	lax #4
	jsr delay_256a_x_33_clocks
.endif
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 1320) || n = 1321 || (n > 1324 && n < 1577) || n = 1578 || (n > 1581 && n < 1834) || n = 1835 || (n > 1838 && n < 2091) || n = 2092 || (n > 2095 && n < 2348) || n = 2349 || (n > 2352 && n < 2501))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 1320)
	; 5 bytes
	lax #5
	jsr delay_256a_x_33_clocks
.elseif(n = 1322 || n = 1579 || n = 1836 || n = 2093 || n = 2350)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 1323 || n = 1580 || n = 1837 || n = 2094 || n = 2351)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 1324 || n = 1581 || n = 1838 || n = 2095 || n = 2352)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 1577)
	; 5 bytes
	lax #6
	jsr delay_256a_x_33_clocks
.elseif(n = 1834)
	; 5 bytes
	lax #7
	jsr delay_256a_x_33_clocks
.elseif(n = 2091)
	; 5 bytes
	lax #8
	jsr delay_256a_x_33_clocks
.elseif(n = 2348)
	; 5 bytes
	lax #9
	jsr delay_256a_x_33_clocks
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 2605) || n = 2606 || (n > 2609 && n < 2862) || n = 2863 || (n > 2866 && n < 3119) || n = 3120 || (n > 3123 && n < 3376) || n = 3377 || (n > 3380 && n < 3633) || n = 3634 || (n > 3637 && n < 3890) || n = 3891 || (n > 3894 && n < 4147) || n = 4148 || (n > 4151 && n < 4404) || n = 4405 || (n > 4408 && n < 4661) || n = 4662 || (n > 4665 && n < 4918) || n = 4919 || (n > 4922 && n < 5001))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 2605)
	; 5 bytes
	lax #10
	jsr delay_256a_x_33_clocks
.elseif(n = 2607 || n = 2864 || n = 3121 || n = 3378 || n = 3635 || n = 3892 || n = 4149 || n = 4406 || n = 4663 || n = 4920)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 2608 || n = 2865 || n = 3122 || n = 3379 || n = 3636 || n = 3893 || n = 4150 || n = 4407 || n = 4664 || n = 4921)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 2609 || n = 2866 || n = 3123 || n = 3380 || n = 3637 || n = 3894 || n = 4151 || n = 4408 || n = 4665 || n = 4922)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 2862)
	; 5 bytes
	lax #11
	jsr delay_256a_x_33_clocks
.elseif(n = 3119)
	; 5 bytes
	lax #12
	jsr delay_256a_x_33_clocks
.elseif(n = 3376)
	; 5 bytes
	lax #13
	jsr delay_256a_x_33_clocks
.elseif(n = 3633)
	; 5 bytes
	lax #14
	jsr delay_256a_x_33_clocks
.elseif(n = 3890)
	; 5 bytes
	lax #15
	jsr delay_256a_x_33_clocks
.elseif(n = 4147)
	; 5 bytes
	lax #16
	jsr delay_256a_x_33_clocks
.elseif(n = 4404)
	; 5 bytes
	lax #17
	jsr delay_256a_x_33_clocks
.elseif(n = 4661)
	; 5 bytes
	lax #18
	jsr delay_256a_x_33_clocks
.elseif(n = 4918)
	; 5 bytes
	lax #19
	jsr delay_256a_x_33_clocks
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5175) || n = 5176 || (n > 5179 && n < 5432) || n = 5433 || (n > 5436 && n < 5689) || n = 5690 || (n > 5693 && n < 5946) || n = 5947 || (n > 5950 && n < 6203) || n = 6204 || (n > 6207 && n < 6460) || n = 6461 || (n > 6464 && n < 6717) || n = 6718 || (n > 6721 && n < 6974) || n = 6975 || (n > 6978 && n < 7231) || n = 7232 || (n > 7235 && n < 7488) || n = 7489 || (n > 7492 && n < 7745) || n = 7746 || (n > 7749 && n < 8002) || n = 8003 || (n > 8006 && n < 8259) || n = 8260 || (n > 8263 && n < 8516) || n = 8517 || (n > 8520 && n < 8773) || n = 8774 || (n > 8777 && n < 9030) || n = 9031 || (n > 9034 && n < 9287) || n = 9288 || (n > 9291 && n < 9544) || n = 9545 || (n > 9548 && n < 9801) || n = 9802 || (n > 9805 && n < 10001))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 5175)
	; 5 bytes
	lax #20
	jsr delay_256a_x_33_clocks
.elseif(n = 5177 || n = 5434 || n = 5691 || n = 5948 || n = 6205 || n = 6462 || n = 6719 || n = 6976 || n = 7233 || n = 7490 || n = 7747 || n = 8004 || n = 8261 || n = 8518 || n = 8775 || n = 9032 || n = 9289 || n = 9546 || n = 9803)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 5178 || n = 5435 || n = 5692 || n = 5949 || n = 6206 || n = 6463 || n = 6720 || n = 6977 || n = 7234 || n = 7491 || n = 7748 || n = 8005 || n = 8262 || n = 8519 || n = 8776 || n = 9033 || n = 9290 || n = 9547 || n = 9804)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 5179 || n = 5436 || n = 5693 || n = 5950 || n = 6207 || n = 6464 || n = 6721 || n = 6978 || n = 7235 || n = 7492 || n = 7749 || n = 8006 || n = 8263 || n = 8520 || n = 8777 || n = 9034 || n = 9291 || n = 9548 || n = 9805)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 5432)
	; 5 bytes
	lax #21
	jsr delay_256a_x_33_clocks
.elseif(n = 5689)
	; 5 bytes
	lax #22
	jsr delay_256a_x_33_clocks
.elseif(n = 5946)
	; 5 bytes
	lax #23
	jsr delay_256a_x_33_clocks
.elseif(n = 6203)
	; 5 bytes
	lax #24
	jsr delay_256a_x_33_clocks
.elseif(n = 6460)
	; 5 bytes
	lax #25
	jsr delay_256a_x_33_clocks
.elseif(n = 6717)
	; 5 bytes
	lax #26
	jsr delay_256a_x_33_clocks
.elseif(n = 6974)
	; 5 bytes
	lax #27
	jsr delay_256a_x_33_clocks
.elseif(n = 7231)
	; 5 bytes
	lax #28
	jsr delay_256a_x_33_clocks
.elseif(n = 7488)
	; 5 bytes
	lax #29
	jsr delay_256a_x_33_clocks
.elseif(n = 7745)
	; 5 bytes
	lax #30
	jsr delay_256a_x_33_clocks
.elseif(n = 8002)
	; 5 bytes
	lax #31
	jsr delay_256a_x_33_clocks
.elseif(n = 8259)
	; 5 bytes
	lax #32
	jsr delay_256a_x_33_clocks
.elseif(n = 8516)
	; 5 bytes
	lax #33
	jsr delay_256a_x_33_clocks
.elseif(n = 8773)
	; 5 bytes
	lax #34
	jsr delay_256a_x_33_clocks
.elseif(n = 9030)
	; 5 bytes
	lax #35
	jsr delay_256a_x_33_clocks
.elseif(n = 9287)
	; 5 bytes
	lax #36
	jsr delay_256a_x_33_clocks
.elseif(n = 9544)
	; 5 bytes
	lax #37
	jsr delay_256a_x_33_clocks
.elseif(n = 9801)
	; 5 bytes
	lax #38
	jsr delay_256a_x_33_clocks
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10058) || n = 10059 || (n > 10062 && n < 10315) || n = 10316 || (n > 10319 && n < 10572) || n = 10573 || (n > 10576 && n < 10829) || n = 10830 || (n > 10833 && n < 11086) || n = 11087 || (n > 11090 && n < 11343) || n = 11344 || (n > 11347 && n < 11600) || n = 11601 || (n > 11604 && n < 11857) || n = 11858 || (n > 11861 && n < 12114) || n = 12115 || (n > 12118 && n < 12371) || n = 12372 || (n > 12375 && n < 12628) || n = 12629 || (n > 12632 && n < 12885) || n = 12886 || (n > 12889 && n < 13142) || n = 13143 || (n > 13146 && n < 13399) || n = 13400 || (n > 13403 && n < 13656) || n = 13657 || (n > 13660 && n < 13913) || n = 13914 || (n > 13917 && n < 14170) || n = 14171 || (n > 14174 && n < 14427) || n = 14428 || (n > 14431 && n < 14684) || n = 14685 || (n > 14688 && n < 14941) || n = 14942 || (n > 14945 && n < 15001))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 10058)
	; 5 bytes
	lax #39
	jsr delay_256a_x_33_clocks
.elseif(n = 10060 || n = 10317 || n = 10574 || n = 10831 || n = 11088 || n = 11345 || n = 11602 || n = 11859 || n = 12116 || n = 12373 || n = 12630 || n = 12887 || n = 13144 || n = 13401 || n = 13658 || n = 13915 || n = 14172 || n = 14429 || n = 14686 || n = 14943)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 10061 || n = 10318 || n = 10575 || n = 10832 || n = 11089 || n = 11346 || n = 11603 || n = 11860 || n = 12117 || n = 12374 || n = 12631 || n = 12888 || n = 13145 || n = 13402 || n = 13659 || n = 13916 || n = 14173 || n = 14430 || n = 14687 || n = 14944)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 10062 || n = 10319 || n = 10576 || n = 10833 || n = 11090 || n = 11347 || n = 11604 || n = 11861 || n = 12118 || n = 12375 || n = 12632 || n = 12889 || n = 13146 || n = 13403 || n = 13660 || n = 13917 || n = 14174 || n = 14431 || n = 14688 || n = 14945)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 10315)
	; 5 bytes
	lax #40
	jsr delay_256a_x_33_clocks
.elseif(n = 10572)
	; 5 bytes
	lax #41
	jsr delay_256a_x_33_clocks
.elseif(n = 10829)
	; 5 bytes
	lax #42
	jsr delay_256a_x_33_clocks
.elseif(n = 11086)
	; 5 bytes
	lax #43
	jsr delay_256a_x_33_clocks
.elseif(n = 11343)
	; 5 bytes
	lax #44
	jsr delay_256a_x_33_clocks
.elseif(n = 11600)
	; 5 bytes
	lax #45
	jsr delay_256a_x_33_clocks
.elseif(n = 11857)
	; 5 bytes
	lax #46
	jsr delay_256a_x_33_clocks
.elseif(n = 12114)
	; 5 bytes
	lax #47
	jsr delay_256a_x_33_clocks
.elseif(n = 12371)
	; 5 bytes
	lax #48
	jsr delay_256a_x_33_clocks
.elseif(n = 12628)
	; 5 bytes
	lax #49
	jsr delay_256a_x_33_clocks
.elseif(n = 12885)
	; 5 bytes
	lax #50
	jsr delay_256a_x_33_clocks
.elseif(n = 13142)
	; 5 bytes
	lax #51
	jsr delay_256a_x_33_clocks
.elseif(n = 13399)
	; 5 bytes
	lax #52
	jsr delay_256a_x_33_clocks
.elseif(n = 13656)
	; 5 bytes
	lax #53
	jsr delay_256a_x_33_clocks
.elseif(n = 13913)
	; 5 bytes
	lax #54
	jsr delay_256a_x_33_clocks
.elseif(n = 14170)
	; 5 bytes
	lax #55
	jsr delay_256a_x_33_clocks
.elseif(n = 14427)
	; 5 bytes
	lax #56
	jsr delay_256a_x_33_clocks
.elseif(n = 14684)
	; 5 bytes
	lax #57
	jsr delay_256a_x_33_clocks
.elseif(n = 14941)
	; 5 bytes
	lax #58
	jsr delay_256a_x_33_clocks
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15198) || n = 15199 || (n > 15202 && n < 15455) || n = 15456 || (n > 15459 && n < 15712) || n = 15713 || (n > 15716 && n < 15969) || n = 15970 || (n > 15973 && n < 16226) || n = 16227 || (n > 16230 && n < 16483) || n = 16484 || (n > 16487 && n < 16740) || n = 16741 || (n > 16744 && n < 16997) || n = 16998 || (n > 17001 && n < 17254) || n = 17255 || (n > 17258 && n < 17511) || n = 17512 || (n > 17515 && n < 17768) || n = 17769 || (n > 17772 && n < 18025) || n = 18026 || (n > 18029 && n < 18282) || n = 18283 || (n > 18286 && n < 18539) || n = 18540 || (n > 18543 && n < 18796) || n = 18797 || (n > 18800 && n < 19053) || n = 19054 || (n > 19057 && n < 19310) || n = 19311 || (n > 19314 && n < 19567) || n = 19568 || (n > 19571 && n < 19824) || n = 19825 || (n > 19828 && n < 20001))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 15198)
	; 5 bytes
	lax #59
	jsr delay_256a_x_33_clocks
.elseif(n = 15200 || n = 15457 || n = 15714 || n = 15971 || n = 16228 || n = 16485 || n = 16742 || n = 16999 || n = 17256 || n = 17513 || n = 17770 || n = 18027 || n = 18284 || n = 18541 || n = 18798 || n = 19055 || n = 19312 || n = 19569 || n = 19826)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 15201 || n = 15458 || n = 15715 || n = 15972 || n = 16229 || n = 16486 || n = 16743 || n = 17000 || n = 17257 || n = 17514 || n = 17771 || n = 18028 || n = 18285 || n = 18542 || n = 18799 || n = 19056 || n = 19313 || n = 19570 || n = 19827)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 15202 || n = 15459 || n = 15716 || n = 15973 || n = 16230 || n = 16487 || n = 16744 || n = 17001 || n = 17258 || n = 17515 || n = 17772 || n = 18029 || n = 18286 || n = 18543 || n = 18800 || n = 19057 || n = 19314 || n = 19571 || n = 19828)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 15455)
	; 5 bytes
	lax #60
	jsr delay_256a_x_33_clocks
.elseif(n = 15712)
	; 5 bytes
	lax #61
	jsr delay_256a_x_33_clocks
.elseif(n = 15969)
	; 5 bytes
	lax #62
	jsr delay_256a_x_33_clocks
.elseif(n = 16226)
	; 5 bytes
	lax #63
	jsr delay_256a_x_33_clocks
.elseif(n = 16483)
	; 5 bytes
	lax #64
	jsr delay_256a_x_33_clocks
.elseif(n = 16740)
	; 5 bytes
	lax #65
	jsr delay_256a_x_33_clocks
.elseif(n = 16997)
	; 5 bytes
	lax #66
	jsr delay_256a_x_33_clocks
.elseif(n = 17254)
	; 5 bytes
	lax #67
	jsr delay_256a_x_33_clocks
.elseif(n = 17511)
	; 5 bytes
	lax #68
	jsr delay_256a_x_33_clocks
.elseif(n = 17768)
	; 5 bytes
	lax #69
	jsr delay_256a_x_33_clocks
.elseif(n = 18025)
	; 5 bytes
	lax #70
	jsr delay_256a_x_33_clocks
.elseif(n = 18282)
	; 5 bytes
	lax #71
	jsr delay_256a_x_33_clocks
.elseif(n = 18539)
	; 5 bytes
	lax #72
	jsr delay_256a_x_33_clocks
.elseif(n = 18796)
	; 5 bytes
	lax #73
	jsr delay_256a_x_33_clocks
.elseif(n = 19053)
	; 5 bytes
	lax #74
	jsr delay_256a_x_33_clocks
.elseif(n = 19310)
	; 5 bytes
	lax #75
	jsr delay_256a_x_33_clocks
.elseif(n = 19567)
	; 5 bytes
	lax #76
	jsr delay_256a_x_33_clocks
.elseif(n = 19824)
	; 5 bytes
	lax #77
	jsr delay_256a_x_33_clocks
.endif
.endif
.endif

.endif
	rest
.endmacro
