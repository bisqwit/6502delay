; delay_n, delays n cycles with small code
; Will clobber A, X, Z+N, and V
; Will preserve Y, S, C, D, and I (interrupt-disable flag)
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
; - 8 bytes or less when n <= 293
; - 9 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.3, max=4
; - When 11 <= n <= 20: min=3, avg=4.0, max=5
; - When 21 <= n <= 30: min=5, avg=5.9, max=7
; - When 31 <= n <= 50: min=7, avg=7.2, max=8
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.2, max=9
; - When 301 <= n <= 500: min=9, avg=9.0, max=9
; - When 501 <= n <= 750: min=9, avg=9.0, max=9
; - When 751 <= n <= 1000: min=9, avg=9.0, max=9
; - When 1001 <= n <= 2000: min=9, avg=9.0, max=9
; - When 2001 <= n <= 3000: min=9, avg=9.0, max=9
; - When 3001 <= n <= 5000: min=9, avg=9.0, max=9
; - When 5001 <= n <= 7500: min=9, avg=9.0, max=9
; - When 7501 <= n <= 10000: min=9, avg=9.0, max=9
; - When 10001 <= n <= 20000: min=9, avg=9.0, max=9
; Written by Joel Yliluoma


.macro delay_n n, rest
.assert (n = 0 || n >= 2), error, "delay_n: delay must be 0 or >= 2"
.if(n = 0)
.elseif(n < 0)
	.error .sprintf("delay_n: n cannot be < 0, you gave %d", n)
.elseif(n > 20000)
	delay_n (n/2), { delay_n (n-(n/2)) }
.else
; 1 to 20000
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	bit $24
.elseif(n = 4 || n = 8 || n = 16 || n = 17 || n = 31 || n = 32 || n = 34 || n = 292 || n = 293)
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
.elseif(n = 9 || n = 11 || n = 19 || (n > 20 && n < 24) || n = 33 || n = 35)
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
.elseif(n = 20 || n = 27 || n = 29 || n = 30)
	; 5..6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 26 || n = 28)
	; 6 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 35 && n < 292))
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif((n > 293 && n < 299))
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
	pla
	plp
.elseif((n > 298 && n < 20001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.endif

.endif
	rest
.endmacro
