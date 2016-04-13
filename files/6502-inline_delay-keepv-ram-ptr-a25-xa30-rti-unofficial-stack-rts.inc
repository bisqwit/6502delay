; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, X, Y, S, Z+N, C, and D
; Will preserve V, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; Requires a dummy interrupt handler that does nothing but RTI.
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).

; Requires a disposable zeropage variable @zptemp.

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
; - 1 bytes or less when n <= 4
; - 2 bytes or less when n <= 8
; - 3 bytes or less when n <= 13
; - 4 bytes or less when n <= 21
; - 5 bytes or less when n <= 31
; - 6 bytes or less when n <= 76
; - 7 bytes or less when n <= 1301
; - 8 bytes or less when n <= 2371
; - 9 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=1.9, max=3
; - When 11 <= n <= 20: min=2, avg=3.3, max=4
; - When 21 <= n <= 30: min=4, avg=4.7, max=5
; - When 31 <= n <= 50: min=4, avg=5.5, max=6
; - When 51 <= n <= 75: min=5, avg=5.6, max=6
; - When 76 <= n <= 100: min=5, avg=5.8, max=7
; - When 101 <= n <= 200: min=4, avg=5.8, max=7
; - When 201 <= n <= 300: min=4, avg=5.8, max=7
; - When 301 <= n <= 500: min=4, avg=5.8, max=7
; - When 501 <= n <= 750: min=4, avg=5.8, max=7
; - When 751 <= n <= 1000: min=5, avg=5.9, max=7
; - When 1001 <= n <= 2000: min=4, avg=6.4, max=8
; - When 2001 <= n <= 3000: min=6, avg=7.4, max=9
; - When 3001 <= n <= 5000: min=6, avg=8.4, max=9
; - When 5001 <= n <= 7500: min=7, avg=8.9, max=9
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
.if((n >= 1) && (n < 626))
.if((n >= 1) && (n < 313))
.if((n >= 1) && (n < 157))
.if((n >= 1) && (n < 79))
.if((n >= 1) && (n < 40))
.if((n >= 1) && (n < 20))
; 1 to 19
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
.elseif(n = 6 || n = 11)
	; 2..3 bytes
	delay_n (n-3)
	pha
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 8)
	; 2 bytes
	delay_n (n-4)
	pla
.elseif(n = 9 || n = 10)
	; 3 bytes
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
.elseif(n = 15)
	; 3 bytes
	delay_n (n-2)
	nop
.elseif(n = 16 || n = 17 || n = 19)
	; 3..4 bytes
	delay_n (n-13)
	brk
.elseif(n = 18)
	; 4 bytes
	delay_n (n-5)
	inc @zptemp
.endif
.else
; 20 to 39
.if(n = 20 || n = 22)
	; 4..5 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 21 || n = 26 || n = 29 || n = 30)
	; 4..5 bytes
	delay_n (n-13)
	brk
.elseif(n = 23)
	; 5 bytes
	lda #$2A ;hides 'rol a'
	pha
	Jpl *-2
.elseif(n = 24)
	; 4 bytes
	lda #$0A ;hides 'asl a'
	Jpl *-1
.elseif(n = 25)
	; 5 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27)
	; 5 bytes
	pha
	delay_n (n-3)
.elseif(n = 28 || n = 38)
	; 5..6 bytes
	delay_n (n-2)
	nop
.elseif(n = 31)
	; 5 bytes
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
.elseif(n = 32)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 33)
	; 6 bytes
	clc
	delay_n (n-2)
.elseif(n = 34)
	; 5 bytes
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
.elseif(n = 35)
	; 6 bytes
	lda #$2A ;hides 'rol a'
	delay_n 7
	Jpl *-3
.elseif(n = 36)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 37)
	; 6 bytes
	brk
	delay_n (n-13)
.elseif(n = 39)
	; 4 bytes
	lda #$4A ;hides 'lsr a'
	Jne *-1
.endif
.endif
.else
; 40 to 78
.if(n = 40)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	nop
	dex
	Jne *-3
.elseif(n = 41)
	; 5 bytes
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
.elseif(n = 42 || n = 52 || n = 72 || n = 74)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 43 || n = 53 || n = 60 || n = 67 || n = 70 || n = 75)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif(n = 44)
	; 6 bytes
	lda #$0A ;hides 'asl a'
	nop
	nop
	Jpl *-3
.elseif(n = 45 || n = 48 || n = 58 || n = 68 || n = 73)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 46 || n = 51 || n = 56 || n = 66)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 47)
	; 6 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 49)
	; 4 bytes
	ldy #136 ;hides 'dey'
	Jmi *-1
.elseif(n = 50 || n = 64 || n = 78)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 54)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 55)
	; 5 bytes
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
.elseif(n = 57)
	; 6 bytes
	sec
	lda #$0A ;hides 'asl a'
	clc
	Jne *-2
.elseif(n = 59)
	; 6 bytes
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
.elseif(n = 61 || n = 71 || n = 76)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 62)
	; 5 bytes
	ldx #136 ;hides 'dey'
	dex
	Jmi *-2
.elseif(n = 63)
	; 5 bytes
	lda #$4A ;hides 'lsr a'
	pha
	Jne *-2
.elseif(n = 65)
	; 6 bytes
	clc
	lda #$0A ;hides 'asl a'
	pha
	Jne *-2
.elseif(n = 69)
	; 5 bytes
	ldx #10 ;hides 'asl a'
	dex
	Jne *-2
.elseif(n = 77)
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.endif
.endif
.else
; 79 to 156
.if(n = 79)
	; 5 bytes
	ldy #136 ;hides 'dey'
	pha
	Jmi *-2
.elseif(n = 80)
	; 6 bytes
	ldx #136 ;hides 'dey'
	nop
	dex
	Jmi *-3
.elseif(n = 81 || n = 96 || n = 121 || n = 131 || n = 146 || n = 151 || n = 156)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 82 || n = 100)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif((n .mod 5 = 3) && ((n > 82 && n < 109) || n = 118 || n = 123 || (n > 132 && n < 144)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 84 || n = 94 || n = 104 || n = 109 || n = 114 || n = 127 || n = 139 || n = 144 || n = 149 || n = 154)
	; 5..6 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 87)
	; 6 bytes
	lda #$4A ;hides 'lsr a'
	delay_n 6
	Jne *-3
.elseif(n = 89)
	; 6 bytes
	ldx #10 ;hides 'asl a'
	nop
	dex
	Jne *-3
.elseif(n = 90 || n = 110 || n = 115 || n = 128 || n = 135 || n = 140 || n = 150)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif(n = 92 || n = 120 || n = 148 || n = 155)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 95)
	; 6 bytes
	lda #$4A ;hides 'lsr a'
	delay_n 7
	Jne *-3
.elseif(n = 97)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 102 || n = 112 || n = 117 || n = 122 || n = 142 || n = 147 || n = 152)
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 105 || n = 145)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 107)
	; 6 bytes
	ldx #24 ;hides 'clc'
	dex
	dex
	Jne *-3
.elseif(n = 119)
	; 6 bytes
	ldy #136 ;hides 'dey'
	delay_n 7
	Jmi *-3
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
.elseif(n = 129)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 130)
	; 6 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 132)
	; 6 bytes
	delay_n (n-8)
	slo (@ptrtemp,x)
.elseif(n = 137)
	; 6 bytes
	brk
	delay_n (n-13)
.elseif(n = 153)
	; 5 bytes
	ldx #234 ;hides 'nop'
	inx
	Jne *-2
.endif
.endif
.else
.if((n >= 157) && (n < 235))
; 157 to 234
.if(n = 157 || n = 160 || n = 165 || n = 175 || n = 180 || n = 192 || n = 195 || n = 200 || n = 205 || n = 210 || n = 220 || n = 230)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 3) && ((n > 157 && n < 179) || (n > 192 && n < 214) || (n > 222 && n < 234)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 159 || n = 164 || n = 184 || n = 189 || n = 194 || n = 202 || n = 214 || n = 219 || n = 229 || n = 234)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 161 || n = 166 || n = 171 || n = 201 || n = 226)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 162 || n = 190 || n = 197)
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
.elseif(n = 169 || n = 183 || n = 204 || n = 218 || n = 225 || n = 232)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 170)
	; 6 bytes
	ldy #200 ;hides 'iny'
	iny
	iny
	Jne *-3
.elseif(n = 172)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 174)
	; 5 bytes
	ldx #24 ;hides 'clc'
	dex
	Jpl *-2
.elseif((n .mod 5 = 1) && ((n > 175 && n < 197) || (n > 205 && n < 222) || n = 231))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 177 || n = 185 || n = 217)
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
.elseif(n = 182 || n = 187 || n = 207 || n = 212 || n = 222 || n = 227)
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 188)
	; 5 bytes
	ldx #26 ;hides 'nop'
	dex
	Jpl *-2
.elseif(n = 199)
	; 5 bytes
	ldx #232 ;hides 'inx'
	pha
	Jne *-2
.elseif(n = 209)
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
.elseif(n = 224)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	dex
	Jpl *-3
.endif
.else
; 235 to 312
.if(n = 235 || n = 262 || n = 280 || n = 307)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 236 || n = 246 || n = 261 || n = 286 || n = 291 || n = 296 || n = 306 || n = 311)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 237 || n = 247 || n = 252 || n = 272 || n = 277)
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif((n .mod 5 = 3) && ((n > 237 && n < 249) || (n > 257 && n < 284) || (n > 297 && n < 309)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 239)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 240 || n = 245 || n = 250 || n = 255 || n = 270 || n = 275 || n = 285 || n = 288 || n = 310)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif(n = 241 || n = 251 || n = 256 || n = 266 || n = 271 || n = 276 || n = 281 || n = 301)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 242)
	; 6 bytes
	ldx #74 ;hides 'lsr a'
	inx
	inx
	Jpl *-3
.elseif(n = 244 || n = 254 || n = 259 || n = 264 || n = 282 || n = 287 || n = 294)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 249 || n = 257)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 7 = 1) && ((n > 252 && n < 275) || (n > 294 && n < 310)))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 265)
	; 5 bytes
	ldx #202 ;hides 'dex'
	dex
	Jmi *-2
.elseif(n = 269)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	dey
	dey
	Jmi *-3
.elseif(n = 279)
	; 5 bytes
	ldx #216 ;hides 'cld'
	inx
	Jne *-2
.elseif(n = 284)
	; 4 bytes
	ldy #200 ;hides 'iny'
	Jmi *-1
.elseif(n = 289)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 290)
	; 6 bytes
	ldy #200 ;hides 'iny'
	Jne *-1
	sta (@ptrtemp),y
.elseif(n = 292)
	; 6 bytes
	ldy #200 ;hides 'iny'
	Jne *-1
	slo (@ptrtemp),y
.elseif(n = 293)
	; 5 bytes
	ldx #42 ;hides 'rol a'
	dex
	Jne *-2
.elseif(n = 297)
	; 6 bytes
	brk
	delay_n (n-13)
.elseif(n = 299)
	; 6 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
.elseif(n = 300)
	; 5 bytes
	ldx #42 ;hides 'rol a'
	dex
	Jpl *-2
.elseif(n = 304)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	nop
	dex
	Jmi *-3
.elseif(n = 305)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 312)
	; 6 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	dex
	Jpl *-3
.endif
.endif
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
; 313 to 390
.if(n = 313 || n = 318 || n = 322 || n = 328 || n = 333 || n = 338 || n = 343 || n = 348 || n = 353 || n = 363 || n = 367 || n = 368 || n = 373 || n = 378 || n = 388)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 314)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	iny
	iny
	Jne *-3
.elseif(n = 315 || n = 335 || n = 340 || n = 350 || n = 355 || n = 360 || n = 375 || n = 380 || n = 383 || n = 390)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 1) && (n = 316 || n = 331 || (n > 340 && n < 352) || (n > 360 && n < 387)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 317)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
.elseif(n = 319 || n = 324 || n = 334 || n = 339 || n = 349 || n = 354 || n = 364 || n = 374 || n = 382 || n = 389)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 320)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jmi *-3
.elseif(n = 321 || n = 326 || n = 336 || n = 356)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 323 || n = 330 || n = 344 || n = 365)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 325 || n = 352 || n = 370)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 327 || n = 362)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 329 || n = 345 || n = 369)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 332)
	; 6 bytes
	ldx #74 ;hides 'lsr a'
	dex
	dex
	Jne *-3
.elseif(n = 337 || n = 358 || n = 372)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 342)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 347 || n = 357)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
.elseif(n = 359)
	; 6 bytes
	ldx #216 ;hides 'cld'
	nop
	inx
	Jne *-3
.elseif(n = 377)
	; 5 bytes
	ldx #74 ;hides 'lsr a'
	inx
	Jpl *-2
.elseif(n = 379)
	; 4 bytes
	ldx #202 ;hides 'dex'
	Jmi *-1
.elseif(n = 384)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 385)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 387)
	; 6 bytes
	delay_n 8
	delay_n (n-8)
.endif
.else
; 391 to 468
.if(n = 391 || n = 396 || n = 401 || n = 411 || n = 426 || n = 441 || n = 451 || n = 456 || n = 461)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 392)
	; 6 bytes
	brk
	delay_n (n-13)
.elseif(n = 393 || n = 449)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 5 = 4) && (n = 394 || n = 399 || (n > 418 && n < 445) || n = 454 || n = 459))
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 395)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 397 || n = 415)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 398)
	; 5 bytes
	ldx #56 ;hides 'sec'
	dex
	Jpl *-2
.elseif(n = 400 || n = 407 || n = 414 || n = 428 || n = 435 || n = 442 || n = 463)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 402 || n = 410 || n = 420 || n = 430 || n = 440 || n = 445 || n = 450)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 403 || n = 408 || n = 413 || n = 418 || n = 423 || n = 433 || n = 437 || n = 438 || n = 443 || n = 448 || n = 453 || n = 458 || n = 468)
	; 6..7 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 406 || n = 416 || n = 421 || n = 431 || n = 436 || n = 446 || n = 466)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 409 || n = 425)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 412)
	; 5 bytes
	ldy #186 ;hides 'tsx'
	dey
	Jmi *-2
.elseif(n = 417 || n = 457 || n = 465)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 422)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 427 || n = 452)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 432 || n = 447)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
.elseif(n = 455)
	; 5 bytes
	ldy #200 ;hides 'iny'
	pha
	Jmi *-2
.elseif(n = 460)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 462)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
.elseif(n = 464)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 467)
	; 6 bytes
	ldx #24 ;hides 'clc'
	inx
	inx
	Jpl *-3
.endif
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 474 || n = 479 || n = 492 || n = 499 || n = 504 || n = 509 || n = 514 || n = 527 || n = 534)
	; 6 bytes
	pha
	delay_n (n-3)
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
	ldx #197 ;hides 'cmp zp'
	nop
	inx
	Jne *-3
.elseif(n = 473 || n = 478 || n = 482 || n = 483 || n = 488 || n = 493 || n = 508 || n = 513 || n = 518 || n = 523 || n = 528 || n = 538 || n = 542 || n = 543)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 475 || n = 490 || n = 495 || n = 500 || n = 507 || n = 515 || n = 520 || n = 525 || n = 535)
	; 6 bytes
	pla
	delay_n (n-4)
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
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 485)
	; 6 bytes
	ldx #234 ;hides 'nop'
	dex
	dex
	Jmi *-3
.elseif(n = 487)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 489)
	; 5 bytes
	ldy #186 ;hides 'tsx'
	iny
	Jne *-2
.elseif(n = 494)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	dex
	Jpl *-3
.elseif(n = 497 || n = 529 || n = 545)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 502)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	sta (@ptrtemp,x)
.elseif(n = 503)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jpl *-2
.elseif(n = 510)
	; 5 bytes
	ldx #200 ;hides 'iny'
	dex
	Jmi *-2
.elseif(n = 517)
	; 5 bytes
	ldx #74 ;hides 'lsr a'
	dex
	Jne *-2
.elseif(n = 522)
	; 7 bytes
	ldx #(n-7)/5
	dex
	Jne *-1
	inc @zptemp,x
.elseif(n = 524)
	; 5 bytes
	ldx #74 ;hides 'lsr a'
	dex
	Jpl *-2
.elseif(n = 530)
	; 6 bytes
	ldx #138 ;hides 'txa'
	inx
	inx
	Jne *-3
.elseif(n = 532)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 537)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 539)
	; 6 bytes
	ldx #136 ;hides 'dey'
	inx
	inx
	Jne *-3
.elseif(n = 544)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
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
.elseif(n = 548 || n = 553 || n = 558 || n = 562 || n = 563 || n = 573 || n = 578 || n = 583 || n = 588 || n = 593 || n = 598 || n = 608 || n = 612 || n = 613 || n = 618 || n = 623)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 549 || n = 559 || n = 564 || n = 574 || n = 579 || n = 584 || n = 594 || n = 599 || n = 604 || n = 614 || n = 619)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 550 || n = 595)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 551 || n = 566 || n = 586 || n = 591 || n = 601 || n = 621)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 552)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jmi *-3
.elseif(n = 555 || n = 565 || n = 570 || n = 580 || n = 590 || n = 600 || n = 605)
	; 6 bytes
	pla
	delay_n (n-4)
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
	ldx #197 ;hides 'cmp zp'
	nop
	dex
	Jmi *-3
.elseif(n = 567)
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
.elseif(n = 569)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 572)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 577 || n = 585 || n = 609 || n = 625)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 587)
	; 7 bytes
	ldx #42 ;hides 'rol a'
	pla
	pha
	dex
	Jne *-4
.elseif(n = 592)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	dex
	Jpl *-4
.elseif(n = 597)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 602)
	; 6 bytes
	ldx #122 ;hides 'nop'
	inx
	inx
	Jne *-3
.elseif(n = 607)
	; 5 bytes
	ldx #202 ;hides 'dex'
	pha
	Jmi *-2
.elseif(n = 615)
	; 5 bytes
	ldx #168 ;hides 'tay'
	inx
	Jne *-2
.elseif(n = 620)
	; 6 bytes
	ldx #138 ;hides 'txa'
	dex
	dex
	Jne *-3
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
.if((n >= 626) && (n < 704))
; 626 to 703
.if((n .mod 5 = 1) && (n = 626 || n = 636 || n = 641 || (n > 650 && n < 702)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 627 || n = 642 || n = 702)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 628 || n = 633 || n = 643 || n = 648 || n = 653 || n = 658 || n = 663 || n = 668 || n = 678 || n = 682 || n = 683 || n = 693 || n = 698 || n = 703)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 629)
	; 5 bytes
	ldx #90 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 630 || n = 635 || n = 650 || n = 655 || n = 660 || n = 670 || n = 675 || n = 688 || n = 695 || n = 700)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif(n = 631 || n = 646)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 632 || n = 634 || n = 639 || n = 644 || n = 654 || n = 664 || n = 669 || n = 679 || n = 687 || n = 699)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 637)
	; 6 bytes
	ldx #58 ;hides 'nop'
	pla
	dex
	Jne *-3
.elseif(n = 638 || n = 645 || n = 673 || n = 680 || n = 694)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 640)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 647)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
.elseif(n = 649)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 652 || n = 659)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 657)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 662 || n = 672 || n = 677)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 665)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 667 || n = 685)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 674)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 684)
	; 4 bytes
	ldy #136 ;hides 'dey'
	Jne *-1
.elseif(n = 689)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 690)
	; 6 bytes
	delay_n (n-6)
	sta (@ptrtemp),y
.elseif(n = 692)
	; 6 bytes
	ldx #180 ;hides 'ldy zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 697)
	; 6 bytes
	brk
	delay_n (n-13)
.endif
.else
; 704 to 781
.if(n = 704)
	; 6 bytes
	ldy #167 ;hides 'lax zp'
	iny ;first loop only
	iny
	Jne *-3
.elseif(n = 705 || n = 737 || n = 769 || n = 777)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif((n .mod 5 = 1) && ((n > 705 && n < 782)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 707)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif((n .mod 7 = 1) && ((n > 707 && n < 723) || (n > 742 && n < 765) || n = 778))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 709 || n = 714 || n = 719 || n = 724 || n = 730 || n = 739 || n = 749 || n = 754 || n = 759 || n = 774 || n = 779)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 710 || n = 717 || n = 725 || n = 735 || n = 740 || n = 752 || n = 760 || n = 765 || n = 770 || n = 780)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 712)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	iny
	Jne *-3
.elseif(n = 713)
	; 5 bytes
	ldx #26 ;hides 'nop'
	inx
	Jpl *-2
.elseif((n .mod 5 = 3) && ((n > 717 && n < 739) || (n > 752 && n < 774)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 720)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 727)
	; 5 bytes
	ldx #24 ;hides 'clc'
	inx
	Jpl *-2
.elseif(n = 729)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 732 || n = 742 || n = 762 || n = 767 || n = 772)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 734)
	; 5 bytes
	ldy #232 ;hides 'inx'
	dey
	Jmi *-2
.elseif(n = 744)
	; 6 bytes
	ldx #164 ;hides 'ldy zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 745)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 747)
	; 7 bytes
	ldx #136 ;hides 'dey'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 748)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jmi *-2
.elseif(n = 755)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 775)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.endif
.endif
.else
.if((n >= 782) && (n < 860))
; 782 to 859
.if(n = 782)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jmi *-3
.elseif(n = 783 || n = 787 || n = 788 || n = 793 || n = 798 || n = 803 || n = 808 || n = 818 || n = 822 || n = 823 || n = 828 || n = 833 || n = 838 || n = 843 || n = 858)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 784 || n = 789 || n = 794 || n = 804 || n = 814 || n = 819 || n = 824 || n = 829 || n = 842 || n = 844 || n = 854 || n = 859)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 785 || n = 799 || n = 813 || n = 834 || n = 848 || n = 855)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif((n .mod 5 = 1) && ((n > 785 && n < 857)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 790 || n = 795 || n = 805 || n = 810 || n = 815 || n = 830 || n = 840 || n = 845 || n = 850)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 792 || n = 820 || n = 827)
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
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 807 || n = 837 || n = 852)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 809)
	; 6 bytes
	ldx #90 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 812)
	; 7 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $EA ;hides 'nop'
	iny
	Jne *-4
.elseif(n = 817 || n = 849 || n = 857)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 825)
	; 5 bytes
	ldx #138 ;hides 'txa'
	inx
	Jne *-2
.elseif(n = 832)
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 835)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 6
	Jmi *-3
.elseif(n = 839)
	; 5 bytes
	ldx #136 ;hides 'dey'
	inx
	Jne *-2
.elseif(n = 853)
	; 5 bytes
	ldx #122 ;hides 'nop'
	dex
	Jne *-2
.endif
.else
; 860 to 937
.if(n = 860)
	; 5 bytes
	ldx #250 ;hides 'nop'
	dex
	Jmi *-2
.elseif((n .mod 5 = 1) && ((n > 860 && n < 937)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 862 || n = 883 || n = 904)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 863 || n = 868 || n = 873 || n = 878 || n = 888 || n = 893 || n = 898 || n = 903 || n = 908 || n = 913 || n = 923 || n = 927 || n = 928 || n = 933)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 4) && (n = 864 || (n > 873 && n < 885) || n = 894 || (n > 908 && n < 925) || n = 934))
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 865 || n = 889 || n = 929)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 867 || n = 882 || n = 887 || n = 907 || n = 912)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 869 || n = 890 || n = 897 || n = 918 || n = 925 || n = 932)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 870 || n = 875 || n = 880 || n = 885 || n = 895 || n = 900 || n = 915 || n = 920 || n = 930)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 872 || n = 902 || n = 922)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 877)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 892 || n = 910)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 899)
	; 6 bytes
	ldx #56 ;hides 'sec'
	inx
	inx
	Jne *-3
.elseif(n = 905)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
.endif
.else
.if((n >= 938) && (n < 1094))
.if((n >= 938) && (n < 1016))
; 938 to 1015
.if(n = 938 || n = 943 || n = 948 || n = 963 || n = 968 || n = 973 || n = 978 || n = 983 || n = 993 || n = 997 || n = 998 || n = 1003 || n = 1008 || n = 1013)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 939 || (n > 952 && n < 975) || (n > 987 && n < 1010)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 940 || n = 950 || n = 970 || n = 975 || n = 990 || n = 1005 || n = 1010 || n = 1015)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 1) && ((n > 940 && n < 1012)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 942 || n = 957 || n = 1012)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 944)
	; 6 bytes
	ldy #232 ;hides 'inx'
	nop
	dey
	Jmi *-3
.elseif(n = 945)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 947 || n = 952 || n = 987)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif((n .mod 5 = 4) && ((n > 948 && n < 965) || n = 979 || (n > 988 && n < 1005)))
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 955 || n = 982 || n = 1000)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 958)
	; 5 bytes
	ldy #136 ;hides 'dey'
	nop
	Jne *-2
.elseif(n = 962)
	; 6 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	Jmi *-3
.elseif(n = 965)
	; 5 bytes
	ldx #138 ;hides 'txa'
	dex
	Jne *-2
.elseif(n = 969 || n = 977 || n = 985)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 972)
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 980)
	; 6 bytes
	ldx #218 ;hides 'nop'
	dex
	dex
	Jne *-3
.elseif(n = 984)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	nop
	inx
	Jpl *-3
.elseif(n = 992)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inx
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
.endif
.else
; 1016 to 1093
.if((n .mod 5 = 1) && ((n > 1015 && n < 1092)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 1017 || n = 1024 || n = 1029 || n = 1039 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1084)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 1018 || n = 1035 || n = 1040 || n = 1050 || n = 1055 || n = 1060 || n = 1067 || n = 1075 || n = 1080 || n = 1085)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif(n = 1019)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1020 || n = 1042 || n = 1047)
	; 6..7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1022)
	; 6 bytes
	delay_n (n-8)
	slo (@ptrtemp,x)
.elseif((n .mod 7 = 1) && ((n > 1022 && n < 1045) || (n > 1057 && n < 1080) || n = 1093))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1025)
	; 6 bytes
	ldy #200 ;hides 'iny'
	brk
	Jmi *-3
.elseif(n = 1027)
	; 6 bytes
	brk
	delay_n (n-13)
.elseif(n = 1028 || n = 1032 || n = 1033 || n = 1038 || n = 1043 || n = 1048 || n = 1053 || n = 1068 || n = 1073 || n = 1078 || n = 1083 || n = 1088)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1034)
	; 6 bytes
	ldx #26 ;hides 'nop'
	inx
	inx
	Jne *-3
.elseif(n = 1045 || n = 1090)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 1049)
	; 5 bytes
	ldx #106 ;hides 'ror a'
	inx
	Jne *-2
.elseif(n = 1052)
	; 6 bytes
	ldx #234 ;hides 'nop'
	dex
	dex
	Jne *-3
.elseif(n = 1057 || n = 1089)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 5 = 2) && (n = 1062 || (n > 1076 && n < 1093)))
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1063)
	; 5 bytes
	ldx #152 ;hides 'tya'
	dex
	Jne *-2
.elseif(n = 1070)
	; 6 bytes
	ldy #136 ;hides 'dey'
	iny
	iny
	Jne *-3
.endif
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1099 || n = 1104 || n = 1109 || n = 1119 || n = 1134 || n = 1139 || n = 1144 || n = 1154 || n = 1159 || n = 1164 || n = 1174 || n = 1179 || n = 1192 || n = 1194 || n = 1199 || n = 1204 || n = 1214 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1244)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 1095)
	; 5 bytes
	ldy #136 ;hides 'dey'
	pha
	Jne *-2
.elseif((n .mod 5 = 1) && ((n > 1095 && n < 1247)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 1097)
	; 6 bytes
	ldx #122 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 1098 || n = 1102 || n = 1103 || n = 1108 || n = 1113 || n = 1118 || n = 1123 || n = 1133 || n = 1138 || n = 1143 || n = 1148 || n = 1153 || n = 1158 || n = 1168 || n = 1172 || n = 1173 || n = 1178 || n = 1183 || n = 1188 || n = 1193 || n = 1203 || n = 1208 || n = 1213 || n = 1218 || n = 1223 || n = 1228 || n = 1238 || n = 1242 || n = 1243 || n = 1248)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1099 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1105 || n = 1129 || n = 1137 || n = 1145 || n = 1169 || n = 1185 || n = 1209 || n = 1217 || n = 1225 || n = 1249)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 5 = 0) && (n = 1110 || (n > 1119 && n < 1131) || n = 1140 || (n > 1149 && n < 1166) || (n > 1189 && n < 1201) || (n > 1209 && n < 1221) || n = 1230 || n = 1235 || n = 1245))
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 1112 || n = 1122 || n = 1132 || n = 1147 || n = 1157 || n = 1202 || n = 1222)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1115)
	; 6 bytes
	ldx #248 ;hides 'sed'
	dex
	dex
	Jne *-3
.elseif(n = 1117 || n = 1180 || n = 1207)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1124)
	; 6 bytes
	ldx #250 ;hides 'nop'
	dex
	dex
	Jne *-3
.elseif(n = 1127 || n = 1152 || n = 1167 || n = 1197 || n = 1227)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1162)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 1175)
	; 5 bytes
	ldx #168 ;hides 'tay'
	dex
	Jne *-2
.elseif(n = 1182)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1187)
	; 7 bytes
	ldx #216 ;hides 'cld'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1189)
	; 5 bytes
	ldy #170 ;hides 'tax'
	dey
	Jne *-2
.elseif(n = 1232)
	; 6 bytes
	ldy #136 ;hides 'dey'
	nop
	nop
	Jne *-3
.elseif(n = 1237)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1250)
	; 6 bytes
	ldx #116 ;hides 'nop zp,x'
	inx ;first loop only
	inx
	Jne *-3
.endif
.endif
.endif
.endif
.else
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
.if((n >= 1251) && (n < 1329))
.if((n >= 1251) && (n < 1290))
; 1251 to 1289
.if((n .mod 5 = 1) && ((n > 1250 && n < 1277)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 1252 || n = 1270 || n = 1288)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1253 || n = 1258 || n = 1263 || n = 1278 || n = 1283)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 1254 || (n > 1267 && n < 1290)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1255 || n = 1260 || n = 1277 || n = 1280 || n = 1285)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 1257 || n = 1265)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1259)
	; 6 bytes
	ldx #116 ;hides 'nop zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 1262 || n = 1272)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif((n .mod 5 = 4) && ((n > 1263 && n < 1285)))
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 1267 || n = 1287)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1273)
	; 5 bytes
	ldx #74 ;hides 'lsr a'
	inx
	Jne *-2
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
.endif
.else
; 1290 to 1328
.if(n = 1290 || n = 1291 || n = 1298 || n = 1314 || n = 1322 || n = 1326)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1292 || n = 1300 || n = 1316 || n = 1318 || n = 1327)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1293 || n = 1325)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1294)
	; 7 bytes
	php
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
	plp
.elseif(n = 1295)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $70A2'
	ldx #112
	inx
	Jne *-4
.elseif((n .mod 7 = 1) && ((n > 1295 && n < 1325)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 8 = 1) && ((n > 1296 && n < 1322)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1299)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1301)
	; 5 bytes
	ldy #186 ;hides 'tsx'
	dey
	Jne *-2
.elseif(n = 1302)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1304)
	; 6 bytes
	ldx #164 ;hides 'ldy zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1306 || n = 1315)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1307)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp),y
.elseif(n = 1308)
	; 7 bytes
	ldy #136 ;hides 'dey'
	nop
	iny
	iny
	Jne *-4
.elseif(n = 1309)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1311)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1312)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1319)
	; 6 bytes
	ldx #136 ;hides 'dey'
	pla
	inx
	Jne *-3
.elseif(n = 1320)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	nop
	dex
	Jne *-3
.elseif(n = 1323)
	; 7 bytes
	cmp <$CD ;hides 'cmp $6DA2'
	ldx #109
	inx
	Jne *-4
.elseif(n = 1328)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	dex
	Jne *-3
.endif
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329 || n = 1337 || n = 1353 || n = 1361)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1330)
	; 6 bytes
	ldx #248 ;hides 'sed'
	pla
	dex
	Jmi *-3
.elseif((n .mod 7 = 1) && ((n > 1330 && n < 1367)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1332)
	; 7 bytes
	cmp <$CD ;hides 'cmp $6CA2'
	ldx #108
	inx
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1332 && n < 1361)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1334 || n = 1348 || n = 1356 || n = 1362 || n = 1364)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1335 || n = 1340 || n = 1346 || n = 1347 || n = 1354 || n = 1355)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1336)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jne *-3
.elseif(n = 1339)
	; 6 bytes
	ldx #122 ;hides 'nop'
	pha
	inx
	Jne *-3
.elseif(n = 1341)
	; 6 bytes
	ldx #122 ;hides 'nop'
	pla
	dex
	Jne *-3
.elseif(n = 1343)
	; 7 bytes
	ldx #121
	delay_n 6
	dex
	Jpl *-3
.elseif(n = 1344)
	; 6 bytes
	ldy #167 ;hides 'lax zp'
	iny ;first loop only
	dey
	Jne *-3
.elseif(n = 1349)
	; 6 bytes
	ldx #106 ;hides 'ror a'
	nop
	inx
	Jne *-3
.elseif(n = 1350)
	; 7 bytes
	cmp <$CD ;hides 'cmp $96A2'
	ldx #150
	dex
	Jne *-4
.elseif(n = 1357)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1358)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $97A2'
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
.elseif(n = 1365)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1367)
	; 6 bytes
	ldx #152 ;hides 'tya'
	nop
	dex
	Jne *-3
.endif
.else
; 1368 to 1406
.if(n = 1368 || n = 1375 || n = 1382)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1369 || n = 1377 || n = 1393)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1370 || n = 1384 || n = 1389 || n = 1400 || n = 1403)
	; 6..7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1371 || n = 1391)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1372 || n = 1383 || n = 1388 || n = 1397 || n = 1402)
	; 6..7 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1378 || n = 1396 || n = 1405)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1379)
	; 6 bytes
	ldx #138 ;hides 'txa'
	pha
	dex
	Jne *-3
.elseif(n = 1381)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1385)
	; 5 bytes
	ldx #58 ;hides 'nop'
	inx
	Jne *-2
.elseif(n = 1386)
	; 7 bytes
	cmp <$CD ;hides 'cmp $66A2'
	ldx #102
	inx
	Jne *-4
.elseif(n = 1390 || n = 1404)
	; 7 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1392 || n = 1406)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1395)
	; 7 bytes
	cmp <$CD ;hides 'cmp $9BA2'
	ldx #155
	dex
	Jne *-4
.elseif(n = 1398)
	; 7 bytes
	brk
	delay_n (n-13)
.elseif(n = 1399)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if(n = 1407 || n = 1428)
	; 7 bytes
	delay_n (n-8)
	slo (@ptrtemp,x)
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1444)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 8 = 1) && ((n > 1408 && n < 1442)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1410 || n = 1431 || n = 1434 || n = 1435 || n = 1438 || n = 1445)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1411 || n = 1421)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1412)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $9DA2'
	ldx #157
	dex
	Jne *-4
.elseif(n = 1413)
	; 5 bytes
	ldy #202 ;hides 'dex'
	dey
	Jne *-2
.elseif(n = 1414 || n = 1432)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1416 || n = 1423 || n = 1444)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1418)
	; 7 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1419)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp),y
.elseif(n = 1420)
	; 5 bytes
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
.elseif(n = 1424 || n = 1437)
	; 6..7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1426)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1427)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1430)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $61A2'
	ldx #97
	inx
	Jne *-4
.elseif(n = 1439)
	; 7 bytes
	ldx #136 ;hides 'dey'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 1440)
	; 7 bytes
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
.elseif(n = 1442)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.endif
.else
; 1446 to 1484
.if(n = 1446 || n = 1460 || n = 1462 || n = 1474)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1447 || n = 1454 || n = 1472 || n = 1482)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1448)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $A1A2'
	ldx #161
	dex
	Jne *-4
.elseif(n = 1449 || n = 1465 || n = 1473 || n = 1481)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 7 = 1) && ((n > 1449 && n < 1479)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1451 || n = 1461)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1452 || n = 1466 || n = 1480)
	; 7 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1456)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1458)
	; 7 bytes
	ldy #162 ;hides 'ldx #imm'
	ldx $EA ;hides 'nop'
	dey
	Jne *-4
.elseif(n = 1459 || n = 1468 || n = 1477)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1463)
	; 7 bytes
	ldx #122 ;hides 'nop'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 1467)
	; 7 bytes
	cmp <$CD ;hides 'cmp $A3A2'
	ldx #163
	dex
	Jne *-4
.elseif(n = 1469)
	; 7 bytes
	ldy #232 ;hides 'inx'
	delay_n 7
	dey
	Jmi *-4
.elseif(n = 1470)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 1475)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $A4A2'
	ldx #164
	dex
	Jne *-4
.elseif(n = 1476)
	; 7 bytes
	cmp <$CD ;hides 'cmp $A4A2'
	ldx #164
	dex
	Jne *-4
.elseif(n = 1479)
	; 7 bytes
	ldx #74 ;hides 'lsr a'
	brk
	dex
	Jne *-4
.elseif(n = 1483)
	; 7 bytes
	ldx #106 ;hides 'ror a'
	pla
	pha
	dex
	Jne *-4
.elseif(n = 1484)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $A5A2'
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
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1487 || n = 1490 || n = 1494 || n = 1498 || n = 1507 || n = 1508 || n = 1523)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1488)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1489 || n = 1505 || n = 1521)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1491)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
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
	ldx #69 ;hides 'eor zp'
	nop
	inx
	Jne *-3
.elseif(n = 1497)
	; 5 bytes
	ldx #42 ;hides 'rol a'
	inx
	Jne *-2
.elseif(n = 1500 || n = 1509 || n = 1514)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1501 || n = 1515)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 1502 || n = 1516)
	; 7 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1503)
	; 7 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1504)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1510)
	; 7 bytes
	brk
	delay_n (n-13)
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
	; 6 bytes
	ldx #138 ;hides 'txa'
	pla
	dex
	Jne *-3
.elseif(n = 1518)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1519)
	; 6 bytes
	ldx #152 ;hides 'tya'
	pha
	dex
	Jne *-3
.endif
.else
; 1524 to 1562
.if(n = 1524)
	; 7 bytes
	brk
	delay_n (n-13)
.elseif(n = 1525)
	; 5 bytes
	ldx #218 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1526)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 7 = 1) && ((n > 1526 && n < 1563)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1528 || n = 1544)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1529)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1530)
	; 7 bytes
	cmp <$CD ;hides 'cmp $AAA2'
	ldx #170
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1530 && n < 1559)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1532)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1533)
	; 7 bytes
	delay_n (n-8)
	slo (@ptrtemp,x)
.elseif(n = 1535)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 1536 || n = 1542 || n = 1543 || n = 1550 || n = 1557)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 1) && ((n > 1536 && n < 1562)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1538)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 1539)
	; 7 bytes
	cmp <$CD ;hides 'cmp $ABA2'
	ldx #171
	dex
	Jne *-4
.elseif(n = 1546)
	; 7 bytes
	ldy #136 ;hides 'dey'
	pla
	iny
	iny
	Jne *-4
.elseif(n = 1547)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1551)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1552)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1554)
	; 8 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1556)
	; 6 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1559)
	; 7 bytes
	ldx #173 ;hides 'lda abs'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 1560)
	; 7 bytes
	ldx #100 ;hides 'nop zp'
	nop
	nop
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
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1564 || n = 1570 || n = 1571 || n = 1578 || n = 1579 || n = 1586 || n = 1587 || n = 1592 || n = 1595 || n = 1599)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1565)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $AEA2'
	ldx #174
	dex
	Jne *-4
.elseif(n = 1566)
	; 7 bytes
	cmp <$CD ;hides 'cmp $AEA2'
	ldx #174
	dex
	Jne *-4
.elseif(n = 1567 || n = 1594)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1568)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1598)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1572 || n = 1580 || n = 1588 || n = 1600)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1573)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1574)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $AFA2'
	ldx #175
	dex
	Jne *-4
.elseif(n = 1575)
	; 7 bytes
	cmp <$CD ;hides 'cmp $AFA2'
	ldx #175
	dex
	Jne *-4
.elseif((n .mod 8 = 1) && ((n > 1576 && n < 1602)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1581)
	; 7 bytes
	php
	lax #6
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 1582)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1584)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1589)
	; 7 bytes
	pla
	ldx #(n-5)/9
	pla
	dex
	Jne *-2
.elseif(n = 1591)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1598)
	; 7 bytes
	pla
	delay_n (n-4)
.endif
.else
; 1602 to 1640
.if(n = 1602)
	; 7 bytes
	cmp <$CD ;hides 'cmp $B2A2'
	ldx #178
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1602 && n < 1631)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif((n .mod 7 = 1) && ((n > 1603 && n < 1640)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1605 || n = 1606 || n = 1620 || n = 1634 || n = 1635)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1607)
	; 7 bytes
	ldx #122 ;hides 'nop'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 1608 || n = 1613 || n = 1627)
	; 6..7 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 1614)
	; 7 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1615 || n = 1629)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1616)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1617 || n = 1633)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1619)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1622 || n = 1636)
	; 7 bytes
	brk
	delay_n (n-13)
.elseif(n = 1623)
	; 5 bytes
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
.elseif(n = 1624 || n = 1626 || n = 1640)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1628)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 1631)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1637)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1638)
	; 7 bytes
	cmp <$CD ;hides 'cmp $B6A2'
	ldx #182
	dex
	Jne *-4
.endif
.endif
.else
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if((n .mod 8 = 1) && ((n > 1640 && n < 1666)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1642 || n = 1658)
	; 7..8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1643)
	; 6 bytes
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
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
	ldx #73 ;hides 'eor #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1648 || n = 1666 || n = 1675)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1650)
	; 7 bytes
	ldx #165 ;hides 'lda zp'
	nop
	nop
	dex
	Jne *-4
.elseif(n = 1651)
	; 7 bytes
	ldx #138 ;hides 'txa'
	pla
	pha
	inx
	Jne *-4
.elseif(n = 1652 || n = 1663 || n = 1668 || n = 1677)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1654)
	; 8 bytes
	delay_n (n-8)
	slo (@ptrtemp,x)
.elseif(n = 1655 || n = 1662 || n = 1669 || n = 1672 || n = 1676)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1656)
	; 7 bytes
	cmp <$CD ;hides 'cmp $B8A2'
	ldx #184
	dex
	Jne *-4
.elseif(n = 1659)
	; 6 bytes
	ldx #90 ;hides 'nop'
	pha
	inx
	Jne *-3
.elseif(n = 1661)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1664)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $B9A2'
	ldx #185
	dex
	Jne *-4
.elseif(n = 1670)
	; 7 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
.elseif(n = 1671)
	; 6 bytes
	ldx #152 ;hides 'tya'
	pla
	dex
	Jne *-3
.elseif(n = 1673)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	nop
	dey
	Jne *-3
.elseif(n = 1678)
	; 7 bytes
	ldx #128
	delay_n 8
	dex
	Jpl *-3
.elseif(n = 1679)
	; 6 bytes
	ldx #168 ;hides 'tay'
	pha
	dex
	Jne *-3
.endif
.else
; 1680 to 1718
.if(n = 1680)
	; 7 bytes
	ldy #167 ;hides 'lax zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
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
.elseif(n = 1683 || n = 1686 || n = 1690 || n = 1704 || n = 1715 || n = 1718)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1684 || n = 1693 || n = 1711)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1685 || n = 1703)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1687 || n = 1696 || n = 1698 || n = 1708 || n = 1712 || n = 1714)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 8 = 1) && ((n > 1688 && n < 1714)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1691 || n = 1701)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1692)
	; 7 bytes
	cmp <$CD ;hides 'cmp $BCA2'
	ldx #188
	dex
	Jne *-4
.elseif(n = 1694)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1699)
	; 6 bytes
	ldy #170 ;hides 'tax'
	pha
	dey
	Jne *-3
.elseif(n = 1700)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $BDA2'
	ldx #189
	dex
	Jne *-4
.elseif(n = 1706)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
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
	cmp <$CD ;hides 'cmp $BEA2'
	ldx #190
	dex
	Jne *-4
.elseif(n = 1717)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.endif
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
; 1719 to 1757
.if(n = 1719)
	; 7 bytes
	cmp <$CD ;hides 'cmp $BFA2'
	ldx #191
	dex
	Jne *-4
.elseif(n = 1720 || n = 1747 || n = 1756)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1721)
	; 5 bytes
	ldx #10 ;hides 'asl a'
	inx
	Jne *-2
.elseif(n = 1722 || n = 1732 || n = 1746 || n = 1754)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1752)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1724 || n = 1733 || n = 1738)
	; 6..7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1725 || n = 1739)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 1726 || n = 1740)
	; 7 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1727)
	; 7 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1728)
	; 7 bytes
	delay_n (n-4)
	pla
.elseif(n = 1729 || n = 1745 || n = 1753)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1731)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1734 || n = 1748)
	; 7 bytes
	brk
	delay_n (n-13)
.elseif(n = 1735)
	; 5 bytes
	ldx #248 ;hides 'sed'
	dex
	Jne *-2
.elseif(n = 1736)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $C1A2'
	ldx #193
	dex
	Jne *-4
.elseif(n = 1741)
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
.elseif(n = 1743 || n = 1757)
	; 7 bytes
	delay_n (n-8)
	slo (@ptrtemp,x)
.elseif(n = 1749)
	; 5 bytes
	ldx #250 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1750)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1752)
	; 6 bytes
	ldx #37 ;hides 'and zp'
	nop
	inx
	Jne *-3
.elseif(n = 1755)
	; 7 bytes
	cmp <$CD ;hides 'cmp $C3A2'
	ldx #195
	dex
	Jne *-4
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
	; 7 bytes
	ldx #168 ;hides 'tay'
	brk
	inx
	Jne *-4
.elseif(n = 1760)
	; 6 bytes
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif((n .mod 8 = 1) && ((n > 1760 && n < 1786)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1762 || n = 1767 || n = 1776 || n = 1787 || n = 1788 || n = 1795)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1763)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $C4A2'
	ldx #196
	dex
	Jne *-4
.elseif(n = 1764)
	; 7 bytes
	cmp <$CD ;hides 'cmp $C4A2'
	ldx #196
	dex
	Jne *-4
.elseif(n = 1766)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 1768 || n = 1775 || n = 1780 || n = 1784 || n = 1796)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1770)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1771 || n = 1791)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1773)
	; 7 bytes
	cmp <$CD ;hides 'cmp $C5A2'
	ldx #197
	dex
	Jne *-4
.elseif(n = 1774 || n = 1783 || n = 1792)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1778)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1781)
	; 6 bytes
	ldx #58 ;hides 'nop'
	nop
	inx
	Jne *-3
.elseif(n = 1782)
	; 7 bytes
	cmp <$CD ;hides 'cmp $C6A2'
	ldx #198
	dex
	Jne *-4
.elseif(n = 1789)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1790)
	; 6 bytes
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
.elseif(n = 1793)
	; 6 bytes
	ldx #0
	nop
	dex
	Jne *-2
.elseif(n = 1794)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.endif
.endif
.else
.if((n >= 1797) && (n < 1836))
; 1797 to 1835
.if(n = 1797 || n = 1814 || n = 1832)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1798)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1799)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
.elseif(n = 1800)
	; 7 bytes
	cmp <$CD ;hides 'cmp $C8A2'
	ldx #200
	dex
	Jne *-4
.elseif(n = 1801 || n = 1809 || n = 1825 || n = 1833)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1802 || n = 1804 || n = 1812 || n = 1822 || n = 1829)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1803 || n = 1830)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1805 || n = 1816)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1806)
	; 7 bytes
	ldx #116 ;hides 'nop zp,x'
	inx ;first loop only
	pla
	inx
	Jne *-4
.elseif(n = 1807)
	; 8 bytes
	ldx #128
	delay_n 9
	dex
	Jpl *-4
.elseif(n = 1808)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $C9A2'
	ldx #201
	dex
	Jne *-4
.elseif(n = 1810 || n = 1819 || n = 1828)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1811 || n = 1821 || n = 1831)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1813)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1815)
	; 7 bytes
	ldx #165 ;hides 'lda zp'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 1817)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1818)
	; 7 bytes
	cmp <$CD ;hides 'cmp $CAA2'
	ldx #202
	dex
	Jne *-4
.elseif(n = 1820)
	; 7 bytes
	pha
	ldx #(n-4)/8
	pha
	dex
	Jne *-2
.elseif(n = 1823)
	; 7 bytes
	ldx #152 ;hides 'tya'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 1824)
	; 7 bytes
	ldy #166 ;hides 'ldx zp'
	ldx $48 ;hides 'pha'
	dey
	Jne *-4
.elseif(n = 1826)
	; 6 bytes
	ldx #202 ;hides 'dex'
	nop
	nop
	Jne *-3
.elseif(n = 1827)
	; 7 bytes
	cmp <$CD ;hides 'cmp $CBA2'
	ldx #203
	dex
	Jne *-4
.elseif(n = 1834)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 1835)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inx
	Jne *-3
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 7 bytes
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 1836 && n < 1865)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1838)
	; 7 bytes
	php
	lax #7
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 1839 || n = 1843 || n = 1848 || n = 1856 || n = 1866 || n = 1867 || n = 1874)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1840 || n = 1852 || n = 1868)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 8 = 1) && ((n > 1840 && n < 1874)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1842 || n = 1850)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 1844)
	; 6 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1845)
	; 7 bytes
	cmp <$CD ;hides 'cmp $CDA2'
	ldx #205
	dex
	Jne *-4
.elseif(n = 1847)
	; 6 bytes
	ldx #168 ;hides 'tay'
	pla
	dex
	Jne *-3
.elseif(n = 1851 || n = 1871)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1853)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $CEA2'
	ldx #206
	dex
	Jne *-4
.elseif(n = 1854)
	; 7 bytes
	cmp <$CD ;hides 'cmp $CEA2'
	ldx #206
	dex
	Jne *-4
.elseif(n = 1858)
	; 7 bytes
	ldx #169 ;hides 'lda #imm'
	lda $68,X ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 1859)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	pha
	dey
	Jne *-3
.elseif(n = 1860)
	; 7 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 1861)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1862)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $CFA2'
	ldx #207
	dex
	Jne *-4
.elseif(n = 1863)
	; 7 bytes
	cmp <$CD ;hides 'cmp $CFA2'
	ldx #207
	dex
	Jne *-4
.elseif(n = 1869)
	; 6 bytes
	ldy #170 ;hides 'tax'
	pla
	dey
	Jne *-3
.elseif(n = 1870)
	; 7 bytes
	ldx #69 ;hides 'eor zp'
	nop
	nop
	inx
	Jne *-4
.elseif(n = 1872)
	; 7 bytes
	cmp <$CD ;hides 'cmp $D0A2'
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
.if(n = 1876 || n = 1884 || n = 1892 || n = 1894 || n = 1896 || n = 1910)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1877)
	; 7 bytes
	pla
	ldx #(n-5)/9
	pla
	dex
	Jne *-2
.elseif(n = 1878 || n = 1883 || n = 1887)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1879)
	; 7 bytes
	ldx #85 ;hides 'eor zp,x'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1880)
	; 7 bytes
	ldx #85 ;hides 'eor zp,x'
	nop
	nop
	inx
	Jne *-4
.elseif((n .mod 8 = 1) && ((n > 1880 && n < 1914)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1910)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1885)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1886 || n = 1912)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 1888)
	; 8 bytes
	ldx #110
	jsr @nearby_rts
	dex
	Jpl *-4
.elseif(n = 1890)
	; 7 bytes
	cmp <$CD ;hides 'cmp $D2A2'
	ldx #210
	dex
	Jne *-4
.elseif(n = 1893 || n = 1904)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1895)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1898)
	; 6 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1899)
	; 7 bytes
	cmp <$CD ;hides 'cmp $D3A2'
	ldx #211
	dex
	Jne *-4
.elseif(n = 1901 || n = 1911)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1902)
	; 7 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 1903)
	; 7 bytes
	ldx #136 ;hides 'dey'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 1906)
	; 8 bytes
	delay_n (n-8)
	slo (@ptrtemp,x)
.elseif(n = 1907)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1908)
	; 7 bytes
	cmp <$CD ;hides 'cmp $D4A2'
	ldx #212
	dex
	Jne *-4
.elseif(n = 1914)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
; 1915 to 1953
.if(n = 1915 || n = 1948)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1916)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 1917)
	; 7 bytes
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
.elseif(n = 1918 || n = 1927 || n = 1936)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1919 || n = 1924 || n = 1928 || n = 1930 || n = 1932 || n = 1940 || n = 1946)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1920 || n = 1923 || n = 1939 || n = 1947)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 1) && ((n > 1920 && n < 1954)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1922)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1925)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1926)
	; 7 bytes
	cmp <$CD ;hides 'cmp $D6A2'
	ldx #214
	dex
	Jne *-4
.elseif(n = 1931)
	; 7 bytes
	ldx #138 ;hides 'txa'
	pla
	pha
	dex
	Jne *-4
.elseif(n = 1933)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1934)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $D7A2'
	ldx #215
	dex
	Jne *-4
.elseif(n = 1935)
	; 7 bytes
	ldx #41 ;hides 'and #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1938)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 1941 || n = 1951)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1942)
	; 8 bytes
	ldy #162 ;hides 'ldx #imm'
	.byte $AE,$E6,@zptemp ;ldx abs; hides 'inc @zptemp'
	dey
	Jne *-5
.elseif(n = 1943)
	; 6 bytes
	ldx #216 ;hides 'cld'
	nop
	dex
	Jne *-3
.elseif(n = 1944)
	; 7 bytes
	cmp <$CD ;hides 'cmp $D8A2'
	ldx #216
	dex
	Jne *-4
.elseif(n = 1949)
	; 7 bytes
	pla
	ldx #(n-5)/9
	pla
	dex
	Jne *-2
.elseif(n = 1950)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1952)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $D9A2'
	ldx #217
	dex
	Jne *-4
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
; 1954 to 1992
.if((n .mod 9 = 1) && ((n > 1953 && n < 1991)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1955 || n = 1965 || n = 1974 || n = 1983 || n = 1986 || n = 1987)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1956 || n = 1966 || n = 1982 || n = 1984)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1957)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1958 || n = 1967 || n = 1973 || n = 1976)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1959 || n = 1992)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 1960)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 1961)
	; 6 bytes
	ldx #218 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 1962)
	; 7 bytes
	cmp <$CD ;hides 'cmp $DAA2'
	ldx #218
	dex
	Jne *-4
.elseif(n = 1964)
	; 7 bytes
	pha
	ldx #(n-4)/8
	pha
	dex
	Jne *-2
.elseif(n = 1968)
	; 7 bytes
	ldx #232 ;hides 'inx'
	nop
	inx
	inx
	Jne *-4
.elseif(n = 1969 || n = 1977 || n = 1985)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1970)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	nop
	dex
	Jne *-4
.elseif(n = 1971 || n = 1991)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 1975)
	; 7 bytes
	ldx #152 ;hides 'tya'
	nop
	pla
	dex
	Jne *-4
.elseif(n = 1978)
	; 7 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 1979)
	; 6 bytes
	ldx #58 ;hides 'nop'
	pha
	inx
	Jne *-3
.elseif(n = 1980)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 1988)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $DDA2'
	ldx #221
	dex
	Jne *-4
.elseif(n = 1989)
	; 7 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $EA ;hides 'nop'
	dex
	Jne *-4
.endif
.else
; 1993 to 2031
.if((n .mod 8 = 1) && ((n > 1992 && n < 2026)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1994 || n = 2013 || n = 2023 || n = 2030)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 1995 || n = 2002 || n = 2010 || n = 2018 || n = 2027)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1996 || n = 2004 || n = 2020 || n = 2022 || n = 2028)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 1997)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $DEA2'
	ldx #222
	dex
	Jne *-4
.elseif(n = 1998)
	; 7 bytes
	cmp <$CD ;hides 'cmp $DEA2'
	ldx #222
	dex
	Jne *-4
.elseif(n = 1999)
	; 6 bytes
	ldx #56 ;hides 'sec'
	pha
	inx
	Jne *-3
.elseif(n = 2000)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 2003 || n = 2014)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2005)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2006)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $DFA2'
	ldx #223
	dex
	Jne *-4
.elseif(n = 2007)
	; 7 bytes
	cmp <$CD ;hides 'cmp $DFA2'
	ldx #223
	dex
	Jne *-4
.elseif(n = 2008)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	nop
	inx
	Jne *-3
.elseif(n = 2011 || n = 2021 || n = 2031)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2012)
	; 7 bytes
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	nop
	dey
	Jne *-4
.elseif(n = 2015)
	; 7 bytes
	ldx #168 ;hides 'tay'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 2016)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 2019)
	; 6 bytes
	ldy #202 ;hides 'dex'
	pha
	dey
	Jne *-3
.elseif(n = 2024)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $E1A2'
	ldx #225
	dex
	Jne *-4
.elseif(n = 2026)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2029)
	; 6 bytes
	ldx #202 ;hides 'dex'
	nop
	pha
	Jne *-3
.endif
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032 || n = 2038 || n = 2056)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 2033 || n = 2041)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 2034)
	; 7 bytes
	cmp <$CD ;hides 'cmp $E2A2'
	ldx #226
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2034 && n < 2063)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2036 || n = 2047 || n = 2058)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2037 || n = 2046 || n = 2064)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 2039)
	; 7 bytes
	ldy #170 ;hides 'tax'
	inc @zptemp
	dey
	Jne *-4
.elseif(n = 2040)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 2042)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $E3A2'
	ldx #227
	dex
	Jne *-4
.elseif(n = 2043)
	; 7 bytes
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	inx
	Jne *-4
.elseif(n = 2045)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	pla
	dey
	Jne *-3
.elseif(n = 2048 || n = 2057 || n = 2066)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2049)
	; 6 bytes
	ldx #0
	pha
	dex
	Jne *-2
.elseif(n = 2050)
	; 7 bytes
	ldx #85 ;hides 'eor zp,x'
	lda $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2051 || n = 2061)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2052)
	; 7 bytes
	cmp <$CD ;hides 'cmp $E4A2'
	ldx #228
	dex
	Jne *-4
.elseif(n = 2054)
	; 7 bytes
	ldy #136 ;hides 'dey'
	delay_n 10
	Jne *-4
.elseif(n = 2055 || n = 2068)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2059)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2060)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	inx
	Jne *-4
.elseif(n = 2063)
	; 7 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2065)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2067)
	; 7 bytes
	ldx #68 ;hides 'nop zp'
	nop $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2069)
	; 6 bytes
	ldx #26 ;hides 'nop'
	nop
	inx
	Jne *-3
.elseif(n = 2070)
	; 7 bytes
	cmp <$CD ;hides 'cmp $E6A2'
	ldx #230
	dex
	Jne *-4
.endif
.else
; 2071 to 2109
.if((n .mod 9 = 1) && ((n > 2070 && n < 2108)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2072 || n = 2074 || n = 2083 || n = 2090 || n = 2092 || n = 2108)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 2073 || n = 2082 || n = 2100 || n = 2103 || n = 2104 || n = 2109)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2075 || n = 2084 || n = 2093)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2076 || n = 2085)
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
	cmp <$CD ;hides 'cmp $E7A2'
	ldx #231
	dex
	Jne *-4
.elseif(n = 2081 || n = 2091)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2086)
	; 8 bytes
	delay_n (n-8)
	slo (@ptrtemp,x)
.elseif(n = 2087)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	inx
	Jne *-3
.elseif(n = 2088)
	; 7 bytes
	cmp <$CD ;hides 'cmp $18A2'
	ldx #24
	inx
	Jne *-4
.elseif(n = 2094)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2095)
	; 7 bytes
	php
	lax #8
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 2096)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $17A2'
	ldx #23
	inx
	Jne *-4
.elseif(n = 2097)
	; 7 bytes
	cmp <$CD ;hides 'cmp $17A2'
	ldx #23
	inx
	Jne *-4
.elseif(n = 2099)
	; 7 bytes
	ldx #106 ;hides 'ror a'
	pla
	pha
	inx
	Jne *-4
.elseif(n = 2102)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2105)
	; 6 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 2106)
	; 7 bytes
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	inx
	Jne *-4
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110 || n = 2117 || n = 2128)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 10 = 1) && ((n > 2110 && n < 2142)))
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
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
	ldx #21 ;hides 'ora zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 2115)
	; 7 bytes
	cmp <$CD ;hides 'cmp $EBA2'
	ldx #235
	dex
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2115 && n < 2144)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2118 || n = 2136 || n = 2144 || n = 2145 || n = 2148)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2119)
	; 7 bytes
	ldx #106 ;hides 'ror a'
	brk
	dex
	Jne *-4
.elseif(n = 2120)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2122)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $C1A2'
	ldx #193
	nop
	dex
	Jne *-5
.elseif(n = 2123)
	; 6 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 2124)
	; 7 bytes
	cmp <$CD ;hides 'cmp $14A2'
	ldx #20
	inx
	Jne *-4
.elseif(n = 2126)
	; 7 bytes
	ldx #236 ;hides 'cpx abs'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 2127)
	; 7 bytes
	ldx #152 ;hides 'tya'
	pla
	pha
	dex
	Jne *-4
.elseif(n = 2129 || n = 2138)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2130)
	; 7 bytes
	ldx #43 ;hides 'anc #imm'
	nop $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2132)
	; 6 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 2133)
	; 7 bytes
	cmp <$CD ;hides 'cmp $EDA2'
	ldx #237
	dex
	Jne *-4
.elseif(n = 2135 || n = 2146)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2139)
	; 6 bytes
	ldx #42 ;hides 'rol a'
	pha
	inx
	Jne *-3
.elseif(n = 2140)
	; 8 bytes
	delay_n (n-8)
	slo (@ptrtemp,x)
.elseif(n = 2142)
	; 7 bytes
	cmp <$CD ;hides 'cmp $12A2'
	ldx #18
	inx
	Jne *-4
.elseif(n = 2147)
	; 7 bytes
	ldx #232 ;hides 'inx'
	pha
	inx
	inx
	Jne *-4
.endif
.else
.if((n >= 2149) && (n < 2168))
; 2149 to 2167
.if(n = 2149)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2150)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $EFA2'
	ldx #239
	dex
	Jne *-4
.elseif(n = 2151)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2152 || n = 2161)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2153 || n = 2154 || n = 2163)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2155 || n = 2162 || n = 2164)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 2156)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	pha
	dex
	Jne *-4
.elseif(n = 2157)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2158)
	; 7 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $48,X ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2159)
	; 6 bytes
	ldx #216 ;hides 'cld'
	pha
	dex
	Jne *-3
.elseif(n = 2160)
	; 7 bytes
	cmp <$CD ;hides 'cmp $10A2'
	ldx #16
	inx
	Jne *-4
.elseif(n = 2165)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	cmp $48 ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2166)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	lda $48,X ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2167)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	pha
	dex
	Jne *-4
.endif
.else
; 2168 to 2187
.if(n = 2168)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $F1A2'
	ldx #241
	dex
	Jne *-4
.elseif(n = 2169)
	; 7 bytes
	cmp <$CD ;hides 'cmp $0FA2'
	ldx #15
	inx
	Jne *-4
.elseif(n = 2170 || n = 2179)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2171 || n = 2181)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2172)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2173 || n = 2185)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2174)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2175)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2176)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2177)
	; 6 bytes
	ldx #58 ;hides 'nop'
	pla
	inx
	Jne *-3
.elseif(n = 2178)
	; 7 bytes
	cmp <$CD ;hides 'cmp $F2A2'
	ldx #242
	dex
	Jne *-4
.elseif(n = 2180)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2182)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 2183)
	; 7 bytes
	ldx #74 ;hides 'lsr a'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 2184)
	; 8 bytes
	ldy #167 ;hides 'lax zp'
	iny ;first loop only
	inc @zptemp
	dey
	Jne *-5
.elseif(n = 2186)
	; 6 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 2187)
	; 7 bytes
	cmp <$CD ;hides 'cmp $F3A2'
	ldx #243
	dex
	Jne *-4
.endif
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
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2189)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 2190)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	nop
	nop
	inx
	Jne *-4
.elseif(n = 2191 || n = 2201 || n = 2211)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2192 || n = 2203 || n = 2210 || n = 2219 || n = 2225)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2193 || n = 2208 || n = 2217 || n = 2226)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2194)
	; 8 bytes
	ldx #73 ;hides 'eor #imm'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 2195)
	; 6 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 2196)
	; 7 bytes
	cmp <$CD ;hides 'cmp $0CA2'
	ldx #12
	inx
	Jne *-4
.elseif(n = 2198)
	; 7 bytes
	ldx #12 ;hides 'nop abs'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 2199)
	; 6 bytes
	ldx #56 ;hides 'sec'
	pla
	inx
	Jne *-3
.elseif(n = 2200)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2202 || n = 2207 || n = 2216 || n = 2218)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 2204)
	; 6 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 2205)
	; 7 bytes
	cmp <$CD ;hides 'cmp $0BA2'
	ldx #11
	inx
	Jne *-4
.elseif(n = 2209)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2212)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2213)
	; 6 bytes
	ldx #10 ;hides 'asl a'
	nop
	inx
	Jne *-3
.elseif(n = 2214)
	; 7 bytes
	cmp <$CD ;hides 'cmp $0AA2'
	ldx #10
	inx
	Jne *-4
.elseif(n = 2220)
	; 7 bytes
	ldx #85 ;hides 'eor zp,x'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2221)
	; 6 bytes
	ldy #202 ;hides 'dex'
	pla
	dey
	Jne *-3
.elseif(n = 2222)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $F7A2'
	ldx #247
	dex
	Jne *-4
.elseif(n = 2223)
	; 7 bytes
	ldx #9 ;hides 'ora #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.endif
.else
; 2227 to 2265
.if(n = 2227 || n = 2235 || n = 2236 || n = 2252)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 2228 || n = 2246 || n = 2255 || n = 2264)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2229 || n = 2230 || n = 2244 || n = 2247 || n = 2253 || n = 2262)
	; 7..8 bytes
	delay_n (n-2)
	nop
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
.elseif((n .mod 9 = 1) && ((n > 2232 && n < 2261)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2234 || n = 2256)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2237 || n = 2263)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
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
	cmp #<$CD ;hides 'cmp $07A2'
	ldx #7
	inx
	Jne *-4
.elseif(n = 2241)
	; 7 bytes
	cmp <$CD ;hides 'cmp $07A2'
	ldx #7
	inx
	Jne *-4
.elseif(n = 2243)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2245 || n = 2257)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2248)
	; 7 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	pla
	inx
	Jne *-4
.elseif(n = 2249)
	; 6 bytes
	ldx #250 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 2250)
	; 7 bytes
	cmp <$CD ;hides 'cmp $06A2'
	ldx #6
	inx
	Jne *-4
.elseif(n = 2254)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2258)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $05A2'
	ldx #5
	inx
	Jne *-4
.elseif(n = 2259)
	; 7 bytes
	cmp <$CD ;hides 'cmp $05A2'
	ldx #5
	inx
	Jne *-4
.elseif(n = 2261)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2265)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $CEA2'
	ldx #206
	nop
	dex
	Jne *-5
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if(n = 2266)
	; 8 bytes
	cmp <$CD ;hides 'cmp $CEA2'
	ldx #206
	nop
	dex
	Jne *-5
.elseif(n = 2267)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $04A2'
	ldx #4
	inx
	Jne *-4
.elseif(n = 2268)
	; 7 bytes
	cmp <$CD ;hides 'cmp $04A2'
	ldx #4
	inx
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2268 && n < 2297)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2270 || n = 2275 || n = 2280 || n = 2288 || n = 2298)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2271 || n = 2291 || n = 2301)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2272 || n = 2284 || n = 2290 || n = 2292)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2273 || n = 2282)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2274)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2276)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $03A2'
	ldx #3
	inx
	Jne *-4
.elseif(n = 2277)
	; 7 bytes
	cmp <$CD ;hides 'cmp $03A2'
	ldx #3
	inx
	Jne *-4
.elseif(n = 2279)
	; 7 bytes
	ldx #152 ;hides 'tya'
	delay_n 8
	dex
	Jne *-4
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
.elseif(n = 2285)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $02A2'
	ldx #2
	inx
	Jne *-4
.elseif(n = 2286)
	; 7 bytes
	cmp <$CD ;hides 'cmp $02A2'
	ldx #2
	inx
	Jne *-4
.elseif(n = 2289 || n = 2300)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2294)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $01A2'
	ldx #1
	inx
	Jne *-4
.elseif(n = 2295)
	; 7 bytes
	cmp <$CD ;hides 'cmp $01A2'
	ldx #1
	inx
	Jne *-4
.elseif(n = 2299)
	; 6 bytes
	ldx #26 ;hides 'nop'
	pha
	inx
	Jne *-3
.elseif(n = 2302)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2303)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $00A2'
	ldx #0
	inx
	Jne *-4
.elseif(n = 2304)
	; 7 bytes
	cmp <$CD ;hides 'cmp $00A2'
	ldx #0
	inx
	Jne *-4
.endif
.else
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
.if(n = 2305)
	; 6 bytes
	ldx #0
	pla
	dex
	Jne *-2
.elseif(n = 2306 || n = 2307 || n = 2313)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2308 || n = 2318)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2309 || n = 2312)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2310)
	; 8 bytes
	cmp <$CD ;hides 'cmp $D2A2'
	ldx #210
	nop
	dex
	Jne *-5
.elseif(n = 2311 || n = 2321)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2314)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2315)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2316)
	; 8 bytes
	cmp <$CD ;hides 'cmp $C1A2'
	ldx #193
	pha
	dex
	Jne *-5
.elseif(n = 2317)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2319)
	; 6 bytes
	ldx #24 ;hides 'clc'
	pha
	inx
	Jne *-3
.elseif(n = 2320)
	; 7 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2322)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
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
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2325)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2326)
	; 7 bytes
	ldx #232 ;hides 'inx'
	pla
	inx
	inx
	Jne *-4
.elseif(n = 2327)
	; 8 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2328)
	; 7 bytes
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
	Jne *-4
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
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2332)
	; 8 bytes
	cmp <$CD ;hides 'cmp $D4A2'
	ldx #212
	nop
	dex
	Jne *-5
.elseif(n = 2333)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2334)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2335)
	; 8 bytes
	ldx #100 ;hides 'nop zp'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 2336)
	; 7 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $68 ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 2337)
	; 7 bytes
	ldx #180 ;hides 'ldy zp,x'
	lda $68,X ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 2338)
	; 8 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2339)
	; 6 bytes
	ldx #234 ;hides 'nop'
	pha
	dex
	Jne *-3
.elseif(n = 2340)
	; 8 bytes
	ldx #100 ;hides 'nop zp'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 2342)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	nop
	dex
	Jne *-4
.elseif(n = 2343)
	; 7 bytes
	pla
	delay_n (n-4)
.endif
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if(n = 2344 || n = 2355 || n = 2366)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2345)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2346 || n = 2368 || n = 2373 || n = 2382)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2348)
	; 8 bytes
	pla
	ldx #(n-5)/11
	nop
	pla
	dex
	Jne *-3
.elseif(n = 2349)
	; 7 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $68 ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 2350)
	; 7 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $68,X ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 2351)
	; 7 bytes
	ldx #168 ;hides 'tay'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 2352)
	; 7 bytes
	php
	lax #9
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 2353)
	; 6 bytes
	ldx #42 ;hides 'rol a'
	pla
	inx
	Jne *-3
.elseif(n = 2354 || n = 2367 || n = 2380)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2356 || n = 2358 || n = 2369 || n = 2378)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2357 || n = 2360)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2359)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2361 || n = 2371 || n = 2381)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2362)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	lda $68,X ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 2363)
	; 7 bytes
	ldx #41 ;hides 'and #imm'
	lda $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2364)
	; 7 bytes
	ldx #41 ;hides 'and #imm'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2365 || n = 2377)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2370)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jpl *-5
.elseif(n = 2372)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 2374)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2375)
	; 6 bytes
	ldx #216 ;hides 'cld'
	pla
	dex
	Jne *-3
.elseif(n = 2376)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	pla
	dex
	Jne *-4
.elseif(n = 2379)
	; 7 bytes
	ldy #170 ;hides 'tax'
	delay_n 7
	dey
	Jne *-4
.endif
.else
.if((n >= 2383) && (n < 2402))
; 2383 to 2401
.if(n = 2383)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2384 || n = 2394)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2385)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 2386)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $D9A2'
	ldx #217
	nop
	dex
	Jne *-5
.elseif(n = 2387)
	; 7 bytes
	ldy #200 ;hides 'iny'
	pha
	dey
	dey
	Jne *-4
.elseif(n = 2388)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2389 || n = 2401)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2390)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 2391)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2392 || n = 2396 || n = 2400)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2393)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2395)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2397)
	; 6 bytes
	ldx #218 ;hides 'nop'
	pla
	dex
	Jne *-3
.elseif(n = 2398)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2399)
	; 7 bytes
	ldx #56 ;hides 'sec'
	inc @zptemp
	inx
	Jne *-4
.endif
.else
; 2402 to 2421
.if(n = 2402)
	; 8 bytes
	pha
	ldx #218
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2403 || n = 2412 || n = 2415)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2404)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 2405)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 2406)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2407)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	lda $48 ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2408)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	lda $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2409)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	nop
	pha
	inx
	Jne *-4
.elseif(n = 2410)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2411 || n = 2421)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2413)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2414)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2416)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2417)
	; 7 bytes
	ldy #186 ;hides 'tsx'
	delay_n 6
	dey
	Jne *-4
.elseif(n = 2418)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2419)
	; 7 bytes
	ldx #248 ;hides 'sed'
	brk
	dex
	Jmi *-4
.elseif(n = 2420)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	pha
	inx
	Jne *-4
.endif
.endif
.endif
.else
.if((n >= 2422) && (n < 2461))
.if((n >= 2422) && (n < 2441))
; 2422 to 2440
.if(n = 2422)
	; 8 bytes
	pha
	ldx #186
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2423)
	; 7 bytes
	ldy #202 ;hides 'dex'
	inc @zptemp
	dey
	Jne *-4
.elseif(n = 2424)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2425 || n = 2437)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2426)
	; 8 bytes
	pha
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2427)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2428)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 2429)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 2430)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $DDA2'
	ldx #221
	nop
	dex
	Jne *-5
.elseif(n = 2431)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2432)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2433)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $48 ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2434)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2435)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
.elseif(n = 2436)
	; 8 bytes
	pla
	ldx #(n-5)/11
	nop
	pla
	dex
	Jne *-3
.elseif(n = 2438)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 2439)
	; 7 bytes
	ldx #122 ;hides 'nop'
	brk
	dex
	Jne *-4
.elseif(n = 2440)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.endif
.else
; 2441 to 2460
.if(n = 2441 || n = 2451)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2442)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2443 || n = 2454)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2444)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2445 || n = 2458)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2446)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2447)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2448)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 2449)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2450)
	; 7 bytes
	ldx #11 ;hides 'anc #imm'
	lda $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2452)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $DFA2'
	ldx #223
	nop
	dex
	Jne *-5
.elseif(n = 2453 || n = 2456)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2455)
	; 8 bytes
	ldx #164 ;hides 'ldy zp'
	ldy $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 2457)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2459)
	; 6 bytes
	ldx #10 ;hides 'asl a'
	pha
	inx
	Jne *-3
.elseif(n = 2460)
	; 8 bytes
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	pla
	pha
	dex
	Jne *-5
.endif
.endif
.else
.if((n >= 2461) && (n < 2481))
; 2461 to 2480
.if(n = 2461 || n = 2473)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2462 || n = 2468)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 2463)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2464)
	; 8 bytes
	cmp <$CD ;hides 'cmp $E0A2'
	ldx #224
	nop
	dex
	Jne *-5
.elseif(n = 2465)
	; 6 bytes
	ldy #136 ;hides 'dey'
	brk
	Jne *-3
.elseif(n = 2466)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2467)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2469)
	; 7 bytes
	ldx #9 ;hides 'ora #imm'
	lda $48 ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2470)
	; 7 bytes
	ldx #9 ;hides 'ora #imm'
	lda $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2471)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2472)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2474)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $E1A2'
	ldx #225
	nop
	dex
	Jne *-5
.elseif(n = 2475)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2476)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2477)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 2478)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 2479)
	; 6 bytes
	ldx #248 ;hides 'sed'
	pha
	dex
	Jne *-3
.elseif(n = 2480)
	; 8 bytes
	pla
	ldx #(n-5)/11
	nop
	pla
	dex
	Jne *-3
.endif
.else
; 2481 to 2500
.if(n = 2481 || n = 2491)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2482 || n = 2488 || n = 2492)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2483)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2484)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2485 || n = 2497)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2486)
	; 8 bytes
	cmp <$CD ;hides 'cmp $E2A2'
	ldx #226
	nop
	dex
	Jne *-5
.elseif(n = 2487 || n = 2498)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2489)
	; 7 bytes
	ldx #90 ;hides 'nop'
	delay_n 8
	inx
	Jne *-4
.elseif(n = 2490)
	; 8 bytes
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	pla
	pha
	dex
	Jne *-5
.elseif(n = 2493)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2494)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2495)
	; 8 bytes
	pla
	ldx #(n-5)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2496)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $E3A2'
	ldx #227
	nop
	dex
	Jne *-5
.elseif(n = 2499)
	; 6 bytes
	ldx #250 ;hides 'nop'
	pha
	dex
	Jne *-3
.elseif(n = 2500)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
.endif
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
.if(n = 2501 || n = 2511 || n = 2531)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2502 || n = 2524 || n = 2526 || n = 2532)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2503 || n = 2539)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2504 || n = 2514)
	; 8 bytes
	pha
	ldx #(n-4)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2505)
	; 8 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
.elseif(n = 2506)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2507 || n = 2535)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2508)
	; 8 bytes
	cmp <$CD ;hides 'cmp $E4A2'
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
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	nop
	nop
	inx
	Jne *-4
.elseif(n = 2512 || n = 2513 || n = 2522 || n = 2525 || n = 2538)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2515)
	; 8 bytes
	pla
	ldx #(n-5)/10
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2516)
	; 8 bytes
	ldx #180 ;hides 'ldy zp,x'
	.byte $AC,$E6,@zptemp ;ldy abs; hides 'inc @zptemp'
	dex
	Jne *-5
.elseif(n = 2517 || n = 2534)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2518)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	nop
	inx
	Jne *-5
.elseif(n = 2519)
	; 7 bytes
	ldx #168 ;hides 'tay'
	delay_n 8
	dex
	Jne *-4
.elseif(n = 2520)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2523 || n = 2536)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2527)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2528)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 2529)
	; 6 bytes
	ldx #26 ;hides 'nop'
	pla
	inx
	Jne *-3
.elseif(n = 2530)
	; 8 bytes
	cmp <$CD ;hides 'cmp $E6A2'
	ldx #230
	nop
	dex
	Jne *-5
.elseif(n = 2537)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.endif
.else
.if((n >= 2540) && (n < 2559))
; 2540 to 2558
.if(n = 2540)
	; 7 bytes
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 2541)
	; 7 bytes
	ldx #(n-1)/10
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2542 || n = 2553)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2543)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	pha
	dex
	Jne *-4
.elseif(n = 2544)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	.byte $AE,$E6,@zptemp ;ldx abs; hides 'inc @zptemp'
	dey
	Jne *-5
.elseif(n = 2545 || n = 2557)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2546 || n = 2554 || n = 2556)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2547)
	; 7 bytes
	ldx #74 ;hides 'lsr a'
	pla
	pha
	inx
	Jne *-4
.elseif(n = 2548)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 2549)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2550)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	nop
	inx
	inx
	Jne *-4
.elseif(n = 2551)
	; 6 bytes
	ldx #24 ;hides 'clc'
	pla
	inx
	Jne *-3
.elseif(n = 2552)
	; 8 bytes
	cmp <$CD ;hides 'cmp $18A2'
	ldx #24
	nop
	inx
	Jne *-5
.elseif(n = 2555)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 2558)
	; 8 bytes
	ldx #73 ;hides 'eor #imm'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.endif
.else
; 2559 to 2578
.if(n = 2559 || n = 2571)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2560 || n = 2576)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2561)
	; 7 bytes
	ldx #0
	inc @zptemp
	dex
	Jne *-3
.elseif(n = 2562)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2563)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2564)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2565)
	; 8 bytes
	pla
	ldx #0
	nop
	pha
	dex
	Jne *-3
.elseif(n = 2566)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2567)
	; 7 bytes
	ldx #42 ;hides 'rol a'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 2568)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2569)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2570)
	; 8 bytes
	ldx #128 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jpl *-5
.elseif(n = 2572)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jpl *-5
.elseif(n = 2573)
	; 6 bytes
	ldx #234 ;hides 'nop'
	pla
	dex
	Jne *-3
.elseif(n = 2574)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 2575)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2577)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2578)
	; 8 bytes
	ldx #41 ;hides 'and #imm'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579)
	; 8 bytes
	pla
	ldx #(n-5)/11
	nop
	pla
	dex
	Jne *-3
.elseif(n = 2580 || n = 2587 || n = 2592 || n = 2613)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 12 = 1) && ((n > 2580 && n < 2618)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2582)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 2583)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 2584)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	nop
	inx
	Jne *-4
.elseif(n = 2585 || n = 2607 || n = 2610)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2586 || n = 2597 || n = 2608)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2588 || n = 2601 || n = 2614)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2589 || n = 2600 || n = 2612)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2590)
	; 8 bytes
	ldx #130 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 2591)
	; 7 bytes
	ldx #216 ;hides 'cld'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 2594)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 2595)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2596)
	; 8 bytes
	cmp <$CD ;hides 'cmp $14A2'
	ldx #20
	nop
	inx
	Jne *-5
.elseif(n = 2598)
	; 8 bytes
	ldx #236 ;hides 'cpx abs'
	inc @zptemp
	nop
	dex
	Jne *-5
.elseif(n = 2599)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 6
	inx
	Jne *-4
.elseif(n = 2602)
	; 8 bytes
	ldy #136 ;hides 'dey'
	delay_n 14
	Jne *-5
.elseif(n = 2603)
	; 7 bytes
	ldy #186 ;hides 'tsx'
	delay_n 7
	dey
	Jne *-4
.elseif(n = 2604)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	brk
	inx
	Jpl *-5
.elseif(n = 2606)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2609)
	; 7 bytes
	php
	lax #10
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 2611)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2615)
	; 7 bytes
	ldx #218 ;hides 'nop'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 2616)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	pla
	inx
	Jne *-4
.endif
.else
.if((n >= 2618) && (n < 2637))
; 2618 to 2636
.if(n = 2618)
	; 8 bytes
	cmp <$CD ;hides 'cmp $12A2'
	ldx #18
	nop
	inx
	Jne *-5
.elseif(n = 2619 || n = 2630)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2620 || n = 2622)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2621)
	; 8 bytes
	ldx #138 ;hides 'txa'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 2623 || n = 2634)
	; 8 bytes
	pla
	ldx #(n-5)/11
	nop
	pla
	dex
	Jne *-3
.elseif(n = 2624)
	; 8 bytes
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	delay_n 8
	dex
	Jne *-5
.elseif(n = 2625)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	lda $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2626)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2627)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2628)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	nop
	pla
	inx
	Jne *-4
.elseif(n = 2629)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2631 || n = 2632)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2633)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2635)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2636)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.endif
.else
; 2637 to 2656
.if(n = 2637)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 2638)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	delay_n 6
	Jne *-4
.elseif(n = 2639)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	nop
	inx
	Jne *-5
.elseif(n = 2640)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2641 || n = 2653)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2642 || n = 2643 || n = 2654 || n = 2655)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2644)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2645)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 2646)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	delay_n 6
	inx
	Jne *-5
.elseif(n = 2647)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2648)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	lda $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2649)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2650)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $F1A2'
	ldx #241
	nop
	dex
	Jne *-5
.elseif(n = 2651)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	pla
	inx
	Jne *-4
.elseif(n = 2652)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2656)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.endif
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
.if((n >= 2657) && (n < 2696))
.if((n >= 2657) && (n < 2676))
; 2657 to 2675
.if(n = 2657)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif((n > 2657 && n < 2661))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 2661 || n = 2675)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2662)
	; 8 bytes
	cmp <$CD ;hides 'cmp $F2A2'
	ldx #242
	nop
	dex
	Jne *-5
.elseif(n = 2663 || n = 2674)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2664)
	; 7 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	pla
	inx
	Jne *-4
.elseif(n = 2665)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2666)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2667)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2668)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2669)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 2670)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2671)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2672)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2673)
	; 8 bytes
	cmp <$CD ;hides 'cmp $F3A2'
	ldx #243
	nop
	dex
	Jne *-5
.endif
.else
; 2676 to 2695
.if(n = 2676 || n = 2691)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2677 || n = 2689)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2678)
	; 8 bytes
	pla
	ldx #(n-5)/11
	nop
	pla
	dex
	Jne *-3
.elseif(n = 2679)
	; 7 bytes
	ldx #122 ;hides 'nop'
	brk
	inx
	Jne *-4
.elseif(n = 2680 || n = 2688)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2681)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
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
	nop
	nop
	dex
	Jne *-4
.elseif(n = 2684)
	; 8 bytes
	cmp <$CD ;hides 'cmp $0CA2'
	ldx #12
	nop
	inx
	Jne *-5
.elseif(n = 2685)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2686)
	; 8 bytes
	ldx #12 ;hides 'nop abs'
	inc @zptemp
	nop
	inx
	Jne *-5
.elseif(n = 2687)
	; 8 bytes
	ldx #168 ;hides 'tay'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 2690)
	; 8 bytes
	lda #<$AD ;hides 'lda $CFA2'
	ldx #207
	pla
	dex
	Jne *-5
.elseif(n = 2692)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2693)
	; 7 bytes
	ldx #11 ;hides 'anc #imm'
	lda $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2694)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2695)
	; 8 bytes
	cmp <$CD ;hides 'cmp $0BA2'
	ldx #11
	nop
	inx
	Jne *-5
.endif
.endif
.else
; 2696 to 2734
.if((n .mod 11 = 1) && ((n > 2695 && n < 2730)))
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2697)
	; 8 bytes
	ldx #180 ;hides 'ldy zp,x'
	ldy $48 ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 2698 || n = 2720 || n = 2733)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2699 || n = 2708 || n = 2730 || n = 2732 || n = 2734)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2700 || n = 2711 || n = 2722)
	; 8 bytes
	pla
	ldx #(n-5)/11
	nop
	pla
	dex
	Jne *-3
.elseif(n = 2701 || n = 2713 || n = 2725)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2702)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 2703 || n = 2717)
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
	; 6 bytes
	ldx #10 ;hides 'asl a'
	pla
	inx
	Jne *-3
.elseif(n = 2706)
	; 8 bytes
	cmp <$CD ;hides 'cmp $0AA2'
	ldx #10
	nop
	inx
	Jne *-5
.elseif(n = 2709)
	; 7 bytes
	pla
	delay_n (n-4)
.elseif(n = 2710)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	dex
	Jne *-5
.elseif(n = 2712)
	; 8 bytes
	ldx #194 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 2714)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	nop
	nop
	pla
	dex
	Jne *-5
.elseif(n = 2715)
	; 7 bytes
	ldx #9 ;hides 'ora #imm'
	lda $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2716)
	; 7 bytes
	ldx #9 ;hides 'ora #imm'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 2719)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2723)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $E3A2'
	ldx #227
	pha
	dex
	Jne *-5
.elseif(n = 2724)
	; 8 bytes
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	pha
	inx
	Jne *-5
.elseif(n = 2726)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	lda $68 ;hides 'pla'
	nop
	dey
	Jne *-5
.elseif(n = 2727)
	; 6 bytes
	ldx #248 ;hides 'sed'
	pla
	dex
	Jne *-3
.elseif(n = 2728)
	; 8 bytes
	cmp <$CD ;hides 'cmp $F8A2'
	ldx #248
	nop
	dex
	Jne *-5
.elseif(n = 2731)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
.if((n >= 2735) && (n < 2754))
; 2735 to 2753
.if(n = 2735)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	nop
	pla
	pha
	inx
	Jne *-5
.elseif(n = 2736)
	; 8 bytes
	cmp <$CD ;hides 'cmp $E4A2'
	ldx #228
	pha
	dex
	Jne *-5
.elseif(n = 2737)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2738)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $07A2'
	ldx #7
	nop
	inx
	Jne *-5
.elseif(n = 2739)
	; 7 bytes
	ldy #136 ;hides 'dey'
	nop
	brk
	Jne *-4
.elseif(n = 2740 || n = 2751)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2741)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2742)
	; 7 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	pla
	dex
	Jne *-4
.elseif(n = 2743 || n = 2747)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2744)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2745)
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
.elseif(n = 2748 || n = 2753)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2749)
	; 6 bytes
	ldx #250 ;hides 'nop'
	pla
	dex
	Jne *-3
.elseif(n = 2750)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2752)
	; 7 bytes
	ldx #212 ;hides 'nop zp,x'
	lda $68 ;hides 'pla'
	dex
	Jne *-4
.endif
.else
; 2754 to 2773
.if(n = 2754)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2755)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2756)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	cmp $48 ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 2757 || n = 2770)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2758)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	delay_n 6
	dex
	Jne *-5
.elseif(n = 2759)
	; 7 bytes
	ldx #26 ;hides 'nop'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 2760)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	lda $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2761 || n = 2773)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2762)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2763 || n = 2764)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2765)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	lda $68 ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 2766)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	lda $68,X ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 2767)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 2768)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	pla
	dex
	Jne *-4
.elseif(n = 2769)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 2771)
	; 7 bytes
	ldx #58 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 2772)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	pha
	inx
	Jne *-4
.endif
.endif
.else
; 2774 to 2812
.if(n = 2774)
	; 8 bytes
	pha
	ldx #232 ;hides 'inx'
	pha
	dex
	dex
	Jne *-4
.elseif(n = 2775 || n = 2786 || n = 2808 || n = 2811)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2776)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2777)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif((n > 2777 && n < 2781))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 2781)
	; 7 bytes
	ldx #42 ;hides 'rol a'
	nop
	pla
	inx
	Jne *-4
.elseif(n = 2782)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $03A2'
	ldx #3
	nop
	inx
	Jne *-5
.elseif(n = 2783)
	; 7 bytes
	ldx #24 ;hides 'clc'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 2784 || n = 2795 || n = 2806)
	; 7 bytes
	ldx #(n-1)/11
	delay_n 6
	dex
	Jne *-3
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
.elseif(n = 2788 || n = 2792 || n = 2798 || n = 2802 || n = 2812)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2789)
	; 7 bytes
	ldy #186 ;hides 'tsx'
	delay_n 8
	dey
	Jne *-4
.elseif(n = 2790)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif(n = 2791)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2793)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $02A2'
	ldx #2
	nop
	inx
	Jne *-5
.elseif(n = 2794)
	; 8 bytes
	cmp <$CD ;hides 'cmp $02A2'
	ldx #2
	nop
	inx
	Jne *-5
.elseif(n = 2796)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2799)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 2800)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.elseif(n = 2803)
	; 8 bytes
	pla
	ldx #56 ;hides 'sec'
	pla
	pha
	inx
	Jne *-4
.elseif(n = 2804)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $01A2'
	ldx #1
	nop
	inx
	Jne *-5
.elseif(n = 2805)
	; 8 bytes
	cmp <$CD ;hides 'cmp $01A2'
	ldx #1
	nop
	inx
	Jne *-5
.elseif(n = 2807)
	; 7 bytes
	ldx #234 ;hides 'nop'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 2810)
	; 8 bytes
	ldx #201 ;hides 'cmp #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
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
.if((n >= 2813) && (n < 2832))
; 2813 to 2831
.if(n = 2813 || n = 2825)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 2814)
	; 8 bytes
	ldx #201 ;hides 'cmp #imm'
	lda $48,X ;hides 'pha'
	pla
	dex
	Jne *-5
.elseif(n = 2815)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2816)
	; 8 bytes
	cmp <$CD ;hides 'cmp $00A2'
	ldx #0
	nop
	inx
	Jne *-5
.elseif(n = 2817)
	; 7 bytes
	ldx #0
	delay_n 6
	dex
	Jne *-3
.elseif(n = 2818)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2819)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	pha
	inx
	Jne *-4
.elseif(n = 2820)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif(n = 2821)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2822)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2823)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2824)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2826)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2827)
	; 7 bytes
	ldy #202 ;hides 'dex'
	delay_n 7
	dey
	Jne *-4
.elseif(n = 2828)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 16
	dex
	Jmi *-5
.elseif(n = 2829)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2830)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	inx
	Jne *-4
.elseif(n = 2831)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	pha
	inx
	Jne *-4
.endif
.else
; 2832 to 2851
.if(n = 2832)
	; 8 bytes
	cmp <$CD ;hides 'cmp $14A2'
	ldx #20
	pha
	inx
	Jne *-5
.elseif(n = 2833 || n = 2845)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2834)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2835 || n = 2848)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2836 || n = 2851)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2837 || n = 2847 || n = 2850)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2838)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 2839)
	; 8 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 2842)
	; 8 bytes
	ldy #203 ;hides 'sbx #imm'
	nop $48,X ;hides 'pha'
	pla
	dey
	Jne *-5
.elseif(n = 2843)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	pha
	inx
	Jne *-4
.elseif(n = 2844)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 2846)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	pha
	inx
	Jne *-5
.elseif(n = 2849)
	; 8 bytes
	ldx #106 ;hides 'ror a'
	jsr @nearby_rts
	inx
	Jne *-5
.endif
.endif
.else
; 2852 to 2890
.if(n = 2852)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 2853)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	inx
	Jne *-5
.elseif(n = 2854)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	inx
	Jne *-5
.elseif(n = 2855)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2856)
	; 8 bytes
	cmp <$CD ;hides 'cmp $12A2'
	ldx #18
	pha
	inx
	Jne *-5
.elseif(n = 2857 || n = 2869 || n = 2881)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2858 || n = 2862 || n = 2875 || n = 2882 || n = 2888)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 2859 || n = 2865 || n = 2868 || n = 2883 || n = 2889)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2860)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 2861 || n = 2874 || n = 2887)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2863)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2864 || n = 2872 || n = 2877 || n = 2884)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2866)
	; 7 bytes
	php
	lax #11
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 2867)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $EFA2'
	ldx #239
	pha
	dex
	Jne *-5
.elseif(n = 2870 || n = 2878)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2871 || n = 2885)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2873)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 2876)
	; 7 bytes
	ldy #136 ;hides 'dey'
	delay_n 16
	Jne *-4
.elseif(n = 2879)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	pla
	pha
	dex
	Jne *-5
.elseif(n = 2880)
	; 8 bytes
	cmp <$CD ;hides 'cmp $10A2'
	ldx #16
	pha
	inx
	Jne *-5
.elseif(n = 2886)
	; 8 bytes
	lda <$AD ;hides 'lda $DEA2'
	ldx #222
	pla
	dex
	Jne *-5
.elseif(n = 2890)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	dex
	Jne *-5
.endif
.endif
.else
.if((n >= 2891) && (n < 2930))
.if((n >= 2891) && (n < 2910))
; 2891 to 2909
.if(n = 2891 || n = 2908)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 2892)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $68 ;hides 'pla'
	pha
	dex
	Jne *-5
.elseif(n = 2893 || n = 2905)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2894)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $48,X ;hides 'pha'
	pla
	dex
	Jne *-5
.elseif(n = 2895 || n = 2902 || n = 2907)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2896)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2897)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 2898)
	; 8 bytes
	lda #<$AD ;hides 'lda $DFA2'
	ldx #223
	pla
	dex
	Jne *-5
.elseif(n = 2899)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2900)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2901)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2903)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	lda $68 ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 2904)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2906)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
.elseif(n = 2909)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.endif
.else
; 2910 to 2929
.if(n = 2910)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $48,X ;hides 'pha'
	pla
	dey
	Jne *-5
.elseif(n = 2911)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	pla
	pha
	dex
	Jne *-5
.elseif(n = 2912)
	; 8 bytes
	lda <$AD ;hides 'lda $E0A2'
	ldx #224
	pla
	dex
	Jne *-5
.elseif(n = 2913 || n = 2926)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2914 || n = 2922)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 2915)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	pha
	dex
	Jne *-4
.elseif(n = 2916 || n = 2920)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2917 || n = 2929)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2918)
	; 8 bytes
	ldx #13 ;hides 'ora abs'
	inc @zptemp
	pha
	inx
	Jne *-5
.elseif(n = 2919 || n = 2928)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2921)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2923)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	delay_n 8
	inx
	Jne *-5
.elseif(n = 2924)
	; 8 bytes
	lda #<$AD ;hides 'lda $1FA2'
	ldx #31
	pla
	inx
	Jne *-5
.elseif(n = 2925)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 2927)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	pha
	dex
	Jne *-4
.endif
.endif
.else
; 2930 to 2968
.if(n = 2930)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2931 || n = 2937 || n = 2943 || n = 2954 || n = 2967)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2932 || n = 2944 || n = 2968)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2933 || n = 2957)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 2934)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	pha
	inx
	inx
	Jne *-5
.elseif(n = 2935)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2936)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	lda $68 ;hides 'pla'
	nop
	dex
	Jne *-5
.elseif(n = 2938)
	; 8 bytes
	ldx #11 ;hides 'anc #imm'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 2939 || n = 2952)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2940)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
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
.elseif(n = 2945)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 2946 || n = 2948 || n = 2958 || n = 2960)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 2947)
	; 8 bytes
	ldx #122 ;hides 'nop'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 2949)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 2950)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 2951)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	inc @zptemp
	inx
	Jne *-4
.elseif(n = 2955)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2956)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2961)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 2962)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 2963)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $48 ;hides 'pha'
	nop
	inx
	Jne *-5
.elseif(n = 2964)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	dex
	Jne *-5
.elseif(n = 2966)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	dex
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
.if((n >= 2969) && (n < 3008))
; 2969 to 3007
.if(n = 2969)
	; 7 bytes
	ldx #58 ;hides 'nop'
	delay_n 8
	inx
	Jne *-4
.elseif(n = 2970)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 2971 || n = 3007)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 2972 || n = 2974 || n = 2980 || n = 2992 || n = 2994 || n = 2998)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2973 || n = 2979 || n = 3003)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2975)
	; 7 bytes
	ldx #248 ;hides 'sed'
	inc @zptemp
	dex
	Jne *-4
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
.elseif(n = 2978 || n = 2991 || n = 3004)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 2981)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 2982)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 2983 || n = 2997)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 2984)
	; 8 bytes
	ldy #200 ;hides 'iny'
	delay_n 6
	dey
	dey
	Jne *-5
.elseif(n = 2985 || n = 2996)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 2986)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 2987)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $07A2'
	ldx #7
	pha
	inx
	Jne *-5
.elseif(n = 2988)
	; 8 bytes
	cmp <$CD ;hides 'cmp $07A2'
	ldx #7
	pha
	inx
	Jne *-5
.elseif(n = 2990)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	lda $68,X ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 2995)
	; 7 bytes
	ldx #42 ;hides 'rol a'
	pla
	pha
	inx
	Jne *-4
.elseif(n = 2999)
	; 7 bytes
	ldx #250 ;hides 'nop'
	inc @zptemp
	dex
	Jne *-4
.elseif(n = 3000)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	pla
	inx
	Jne *-4
.elseif(n = 3002)
	; 7 bytes
	ldx #232 ;hides 'inx'
	pla
	dex
	dex
	Jne *-4
.elseif(n = 3005)
	; 8 bytes
	pla
	ldx #(n-5)/12
	pla
	pha
	dex
	Jne *-3
.elseif(n = 3006)
	; 8 bytes
	ldx #41 ;hides 'and #imm'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.endif
.else
; 3008 to 3046
.if(n = 3008 || n = 3034)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3009)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	lda $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 3010)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 3011)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3012)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	nop
	pla
	inx
	Jne *-4
.elseif(n = 3013 || n = 3025 || n = 3037)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3014 || n = 3019 || n = 3027 || n = 3032 || n = 3045)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3015)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 6
	inx
	Jne *-4
.elseif(n = 3016 || n = 3031 || n = 3046)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3017 || n = 3030)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3018 || n = 3020 || n = 3026 || n = 3028 || n = 3033 || n = 3040)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3021)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3022 || n = 3038)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3023)
	; 7 bytes
	ldx #216 ;hides 'cld'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3024)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	pla
	inx
	Jne *-4
.elseif(n = 3029)
	; 7 bytes
	ldy #202 ;hides 'dex'
	delay_n 8
	dey
	Jne *-4
.elseif(n = 3035)
	; 8 bytes
	ldx #138 ;hides 'txa'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 3036)
	; 8 bytes
	cmp <$CD ;hides 'cmp $03A2'
	ldx #3
	pha
	inx
	Jne *-5
.elseif(n = 3039)
	; 7 bytes
	ldx #152 ;hides 'tya'
	brk
	dex
	Jne *-4
.elseif(n = 3041)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3042)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 8
	inx
	inx
	Jne *-5
.elseif(n = 3043)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3044)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-4
.endif
.endif
.else
.if((n >= 3047) && (n < 3086))
.if((n >= 3047) && (n < 3066))
; 3047 to 3065
.if(n = 3047)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3048)
	; 8 bytes
	cmp <$CD ;hides 'cmp $02A2'
	ldx #2
	pha
	inx
	Jne *-5
.elseif(n = 3049 || n = 3061)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3050)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3051)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3052)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 3053)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3054)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	pla
	inx
	Jne *-4
.elseif(n = 3055)
	; 8 bytes
	pla
	ldx #21 ;hides 'ora zp,x'
	lda $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 3056)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3057)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	inx ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 3058 || n = 3063)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3059)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $01A2'
	ldx #1
	pha
	inx
	Jne *-5
.elseif(n = 3060)
	; 8 bytes
	cmp <$CD ;hides 'cmp $01A2'
	ldx #1
	pha
	inx
	Jne *-5
.elseif(n = 3062)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $4848 ;hides two 'pha's
	inx
	Jne *-5
.elseif(n = 3064)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 3065)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	lda $68,X ;hides 'pla'
	inx
	Jne *-4
.endif
.else
; 3066 to 3085
.if(n = 3066)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 3067)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	pla
	inx
	Jne *-4
.elseif(n = 3068)
	; 8 bytes
	ldx #116 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 3069 || n = 3082)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3070 || n = 3076 || n = 3083 || n = 3085)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3071)
	; 8 bytes
	cmp #<$CD ;hides 'cmp $00A2'
	ldx #0
	pha
	inx
	Jne *-5
.elseif(n = 3072)
	; 8 bytes
	cmp <$CD ;hides 'cmp $00A2'
	ldx #0
	pha
	inx
	Jne *-5
.elseif(n = 3073)
	; 7 bytes
	ldx #0
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3074)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $68,X ;hides 'pla'
	pla
	dex
	Jne *-5
.elseif(n = 3075 || n = 3084)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3077)
	; 8 bytes
	pla
	ldx #0
	pla
	pha
	dex
	Jne *-3
.elseif(n = 3078)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3079)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3080)
	; 7 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	pla
	inx
	Jne *-4
.elseif(n = 3081)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.endif
.endif
.else
; 3086 to 3125
.if(n = 3086)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3087)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	ldx $6868 ;hides two 'pla's
	dey
	Jne *-5
.elseif(n = 3088)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3089 || n = 3105)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3090)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	lda $68 ;hides 'pla'
	pla
	dey
	Jne *-5
.elseif(n = 3091 || n = 3106)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3092 || n = 3096 || n = 3102 || n = 3104 || n = 3113 || n = 3114 || n = 3116 || n = 3122)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3093)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3094)
	; 8 bytes
	lda <$AD ;hides 'lda $EEA2'
	ldx #238
	pla
	dex
	Jne *-5
.elseif(n = 3095 || n = 3108 || n = 3121)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3097 || n = 3115)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3098 || n = 3100 || n = 3111 || n = 3118 || n = 3124)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3099 || n = 3110 || n = 3117 || n = 3125)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3101)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3103)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 3107)
	; 8 bytes
	lda <$AD ;hides 'lda $EFA2'
	ldx #239
	pla
	dex
	Jne *-5
.elseif(n = 3109)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3112)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3119)
	; 8 bytes
	lda #<$AD ;hides 'lda $10A2'
	ldx #16
	pla
	inx
	Jne *-5
.elseif(n = 3120)
	; 8 bytes
	lda <$AD ;hides 'lda $F0A2'
	ldx #240
	pla
	dex
	Jne *-5
.elseif(n = 3123)
	; 7 bytes
	php
	lax #12
	jsr delay_256x_a_30_clocks
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
.if((n >= 3126) && (n < 3165))
; 3126 to 3164
.if(n = 3126 || n = 3150 || n = 3154)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3127 || n = 3138 || n = 3155 || n = 3164)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3128 || n = 3131 || n = 3139 || n = 3140 || (n > 3141 && n < 3145) || n = 3148 || n = 3156 || n = 3157)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3129 || n = 3146 || n = 3163)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3130)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3132)
	; 8 bytes
	lda #<$AD ;hides 'lda $F1A2'
	ldx #241
	pla
	dex
	Jne *-5
.elseif(n = 3133 || n = 3151)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3134 || n = 3147 || n = 3160)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3135 || n = 3149 || n = 3162)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3136)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3137)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3141 || n = 3161)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3145)
	; 8 bytes
	lda #<$AD ;hides 'lda $0EA2'
	ldx #14
	pla
	inx
	Jne *-5
.elseif(n = 3152)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3153)
	; 8 bytes
	ldx #90 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 3158)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	pla
	dex
	Jne *-4
.elseif(n = 3159)
	; 8 bytes
	lda <$AD ;hides 'lda $0DA2'
	ldx #13
	pla
	inx
	Jne *-5
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
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3167)
	; 8 bytes
	ldx #58 ;hides 'nop'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3168)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $68 ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 3169)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3170)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dex
	Jne *-5
.elseif(n = 3171)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	pla
	dex
	Jne *-4
.elseif(n = 3172)
	; 8 bytes
	lda <$AD ;hides 'lda $0CA2'
	ldx #12
	pla
	inx
	Jne *-5
.elseif(n = 3173)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3174 || n = 3176)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3175)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3177)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3178)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 3180)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3181)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3182)
	; 8 bytes
	ldx #11 ;hides 'anc #imm'
	lda $4848 ;hides two 'pha's
	inx
	Jne *-5
.elseif(n = 3183)
	; 8 bytes
	ldy #200 ;hides 'iny'
	delay_n 7
	dey
	dey
	Jne *-5
.endif
.else
; 3184 to 3203
.if(n = 3184)
	; 7 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	pla
	dex
	Jne *-4
.elseif(n = 3185)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3186 || n = 3199)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3187)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3188 || n = 3189)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3190)
	; 8 bytes
	ldx #160 ;hides 'ldy #imm'
	.byte $AC,0,0 ;ldy abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 3191)
	; 8 bytes
	ldx #168 ;hides 'tay'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 3192)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	cmp $48 ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 3193)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3194)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3195)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3196)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3197)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	nop
	pla
	inx
	Jne *-4
.elseif(n = 3198)
	; 8 bytes
	lda <$AD ;hides 'lda $0AA2'
	ldx #10
	pla
	inx
	Jne *-5
.elseif(n = 3200 || n = 3202)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3201)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3203)
	; 8 bytes
	pla
	delay_n (n-4)
.endif
.endif
.endif
.else
.if((n >= 3204) && (n < 3243))
; 3204 to 3242
.if(n = 3204 || n = 3206 || n = 3218 || n = 3232 || n = 3234)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3205 || n = 3223 || n = 3241)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3207 || n = 3221 || n = 3235)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3208)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $4848 ;hides two 'pha's
	inx
	Jne *-5
.elseif(n = 3209)
	; 7 bytes
	ldx #42 ;hides 'rol a'
	delay_n 8
	inx
	Jne *-4
.elseif(n = 3210)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $68,X ;hides 'pla'
	nop
	inx
	Jne *-5
.elseif(n = 3211 || n = 3226)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3212 || n = 3225 || n = 3238)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3213 || n = 3216)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3214)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3215 || n = 3222 || n = 3228)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3217)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3219)
	; 7 bytes
	ldx #26 ;hides 'nop'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 3220)
	; 8 bytes
	ldx #41 ;hides 'and #imm'
	lda $6868 ;hides two 'pla's
	inx
	Jne *-5
.elseif(n = 3224)
	; 8 bytes
	ldx #41 ;hides 'and #imm'
	lda $68,X ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3227 || n = 3240)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3229)
	; 8 bytes
	ldy #170 ;hides 'tax'
	jsr @nearby_rts
	dey
	Jne *-5
.elseif(n = 3230)
	; 8 bytes
	ldy #162 ;hides 'ldx #imm'
	.byte $AE,0,0 ;ldx abs; hides 'brk'
	dey
	Jne *-5
.elseif(n = 3231)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3233)
	; 8 bytes
	ldx #232 ;hides 'inx'
	inc @zptemp
	dex
	dex
	Jne *-5
.elseif(n = 3236)
	; 8 bytes
	lda #<$AD ;hides 'lda $07A2'
	ldx #7
	pla
	inx
	Jne *-5
.elseif(n = 3237)
	; 8 bytes
	lda <$AD ;hides 'lda $07A2'
	ldx #7
	pla
	inx
	Jne *-5
.elseif(n = 3239)
	; 7 bytes
	ldx #216 ;hides 'cld'
	delay_n 8
	dex
	Jne *-4
.elseif(n = 3242)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.endif
.else
.if((n >= 3243) && (n < 3262))
; 3243 to 3261
.if(n = 3243 || n = 3253)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3244)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $68 ;hides 'pla'
	pha
	inx
	Jne *-5
.elseif(n = 3245)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $48 ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3246)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $48,X ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3247)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 3248)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3249)
	; 7 bytes
	ldx #250 ;hides 'nop'
	delay_n 6
	dex
	Jne *-4
.elseif(n = 3250)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 3251)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3252 || n = 3254)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3255)
	; 8 bytes
	ldx #100 ;hides 'nop zp'
	inx ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 3256)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3257)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 3258)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 3259)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3260)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 3261)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.endif
.else
; 3262 to 3281
.if(n = 3262)
	; 8 bytes
	lda #<$AD ;hides 'lda $05A2'
	ldx #5
	pla
	inx
	Jne *-5
.elseif(n = 3263)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3264)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3265)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3266)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3267 || n = 3272 || n = 3278)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3268 || n = 3273)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3269)
	; 7 bytes
	ldx #218 ;hides 'nop'
	delay_n 8
	dex
	Jne *-4
.elseif(n = 3270)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3271)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3274)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3275)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3276)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 3277)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3279)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	inx
	Jne *-5
.elseif(n = 3280)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.elseif(n = 3281)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.endif
.endif
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282 || n = 3299)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3283)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $48 ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3284)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $48,X ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3285)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	nop
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3286)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	.byte $AD,$E6,@zptemp ;lda abs; hides 'inc @zptemp'
	inx
	Jne *-5
.elseif(n = 3287)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $48 ;hides 'pha'
	nop
	inx
	Jne *-5
.elseif(n = 3288)
	; 8 bytes
	lda #<$AD ;hides 'lda $03A2'
	ldx #3
	pla
	inx
	Jne *-5
.elseif(n = 3289)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 3290 || n = 3303 || n = 3316)
	; 7 bytes
	ldx #(n-1)/13
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3291 || n = 3305)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3292 || n = 3297 || n = 3315 || n = 3318)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3293 || n = 3306)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3294 || n = 3307 || n = 3320)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3295 || n = 3313)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3296 || n = 3304 || (n > 3307 && n < 3311) || n = 3312)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3298)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3300)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3301)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3302)
	; 8 bytes
	lda <$AD ;hides 'lda $02A2'
	ldx #2
	pla
	inx
	Jne *-5
.elseif(n = 3311)
	; 8 bytes
	ldx #138 ;hides 'txa'
	delay_n 17
	dex
	Jne *-5
.elseif(n = 3314)
	; 8 bytes
	lda #<$AD ;hides 'lda $01A2'
	ldx #1
	pla
	inx
	Jne *-5
.elseif(n = 3317)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inc @zptemp
	inx
	Jne *-5
.elseif(n = 3319)
	; 7 bytes
	ldx #90 ;hides 'nop'
	brk
	inx
	Jne *-4
.endif
.else
; 3321 to 3359
.if(n = 3321 || n = 3341)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3322 || n = 3332 || n = 3334 || n = 3352)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3323)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif((n > 3323 && n < 3327) || n = 3330 || (n > 3335 && n < 3340) || n = 3342 || n = 3344 || n = 3348 || (n > 3353 && n < 3359))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3327)
	; 8 bytes
	lda #<$AD ;hides 'lda $00A2'
	ldx #0
	pla
	dex
	Jne *-5
.elseif(n = 3328)
	; 8 bytes
	lda <$AD ;hides 'lda $00A2'
	ldx #0
	pla
	dex
	Jne *-5
.elseif(n = 3329)
	; 7 bytes
	ldx #0
	delay_n 8
	dex
	Jne *-3
.elseif(n = 3331 || n = 3349)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3333 || n = 3347)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3335 || n = 3353)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3340)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3343)
	; 8 bytes
	ldx #152 ;hides 'tya'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 3345)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3346)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3350)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3351)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3359)
	; 7 bytes
	ldx #168 ;hides 'tay'
	brk
	dex
	Jne *-4
.endif
.endif
.else
.if((n >= 3360) && (n < 3399))
; 3360 to 3398
.if(n = 3360 || n = 3364 || n = 3366 || n = 3368 || (n > 3371 && n < 3375) || n = 3378 || n = 3379 || n = 3392 || (n > 3393 && n < 3399))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3361 || n = 3375 || n = 3389)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3362 || n = 3383)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3363 || n = 3371 || n = 3384)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3365)
	; 8 bytes
	ldx #202 ;hides 'dex'
	brk
	dex
	dex
	Jne *-5
.elseif(n = 3367 || n = 3385)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3369 || n = 3382 || n = 3387)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3370)
	; 8 bytes
	ldx #169 ;hides 'lda #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 3376)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3377 || n = 3393)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3380)
	; 7 bytes
	php
	lax #13
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3381)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3386)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 3388)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	lda $68 ;hides 'pla'
	pla
	dex
	Jne *-5
.elseif(n = 3390)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pla
	dex
	Jne *-5
.elseif(n = 3391)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.endif
.else
; 3399 to 3437
.if(n = 3399)
	; 7 bytes
	ldy #170 ;hides 'tax'
	brk
	dey
	Jne *-4
.elseif(n = 3400 || n = 3408 || n = 3416 || n = 3419 || n = 3420 || n = 3422 || n = 3427 || n = 3432 || n = 3434 || n = 3437)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3401 || n = 3418 || n = 3435)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3402)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dex
	Jne *-5
.elseif(n = 3403 || n = 3421)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3404)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	lda $68 ;hides 'pla'
	pha
	dex
	Jne *-5
.elseif(n = 3405)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3406 || n = 3436)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3407)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3409 || n = 3425)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3410)
	; 8 bytes
	ldy #171 ;hides 'lax #imm'
	.byte $AE,0,0 ;ldx abs; hides 'brk'
	dey
	Jne *-5
.elseif(n = 3411)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 3412)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	dex
	Jne *-5
.elseif(n = 3413)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	dex
	Jne *-5
.elseif(n = 3414)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	dex
	Jne *-5
.elseif(n = 3415)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 3417 || n = 3431)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3423)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3424)
	; 8 bytes
	ldy #136 ;hides 'dey'
	delay_n 7
	brk
	Jne *-5
.elseif(n = 3426)
	; 8 bytes
	ldx #11 ;hides 'anc #imm'
	cmp $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.elseif(n = 3428)
	; 8 bytes
	ldx #11 ;hides 'anc #imm'
	lda $68 ;hides 'pla'
	pha
	inx
	Jne *-5
.elseif(n = 3429)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 3430)
	; 8 bytes
	ldx #11 ;hides 'anc #imm'
	lda $48,X ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3433)
	; 8 bytes
	ldx #164 ;hides 'ldy zp'
	.byte $AC,0,0 ;ldy abs; hides 'brk'
	dex
	Jne *-5
.endif
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
.if((n >= 3438) && (n < 3477))
; 3438 to 3476
.if(n = 3438 || n = 3440 || n = 3462 || n = 3463 || n = 3468 || n = 3470 || n = 3472 || n = 3476)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3439 || n = 3457 || n = 3475)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3441 || n = 3461)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3442 || n = 3446 || n = 3453 || n = 3460 || n = 3474)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3443)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	pla
	pha
	inx
	Jne *-4
.elseif(n = 3444)
	; 8 bytes
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 3445 || n = 3459 || n = 3473)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3447)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $68 ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3448)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	lda $68,X ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3449)
	; 7 bytes
	ldx #26 ;hides 'nop'
	delay_n 8
	inx
	Jne *-4
.elseif(n = 3450)
	; 7 bytes
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
.elseif(n = 3451 || n = 3466)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3452 || n = 3469)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3454)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.elseif(n = 3455)
	; 8 bytes
	ldx #216 ;hides 'cld'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3456)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $68 ;hides 'pla'
	pha
	inx
	Jne *-5
.elseif(n = 3458)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $48,X ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3464)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-5
.elseif(n = 3465)
	; 8 bytes
	ldx #165 ;hides 'lda zp'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 3467)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	delay_n 8
	inx
	Jne *-5
.elseif(n = 3471)
	; 7 bytes
	ldx #248 ;hides 'sed'
	delay_n 7
	dex
	Jne *-4
.endif
.else
; 3477 to 3515
.if(n = 3477 || n = 3495 || n = 3513)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3478 || n = 3482)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3479)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 8
	inx
	Jne *-4
.elseif(n = 3480 || n = 3485 || (n > 3489 && n < 3493) || n = 3494 || n = 3506)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3481)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3483 || n = 3497)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3484)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 8
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
.elseif(n = 3488)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	delay_n 8
	inx
	Jne *-5
.elseif(n = 3489 || n = 3505)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3493 || n = 3511)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3496)
	; 8 bytes
	ldy #167 ;hides 'lax zp'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
.elseif(n = 3498)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $6868 ;hides two 'pla's
	inx
	Jne *-5
.elseif(n = 3499)
	; 7 bytes
	ldx #250 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3500)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	nop
	pla
	inx
	Jne *-5
.elseif(n = 3502)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	lda $68,X ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3504)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	nop
	delay_n 8
	inx
	Jne *-5
.elseif(n = 3507)
	; 8 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	brk
	dey
	Jne *-5
.elseif(n = 3508)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3509)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 8
	dex
	Jne *-4
.elseif(n = 3510)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	lda $4848 ;hides two 'pha's
	inx
	Jne *-5
.elseif(n = 3512)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	lda $68,X ;hides 'pla'
	nop
	inx
	Jne *-5
.elseif(n = 3514)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	nop
	nop
	pla
	inx
	Jne *-5
.endif
.endif
.else
; 3516 to 3593
.if((n > 3515 && n < 3520) || n = 3525 || n = 3527 || n = 3530 || n = 3534 || n = 3540 || n = 3542 || (n > 3543 && n < 3547) || n = 3548 || n = 3552 || n = 3555 || (n > 3557 && n < 3561) || n = 3562 || n = 3564 || n = 3566 || n = 3570 || (n > 3571 && n < 3581) || n = 3582 || n = 3584 || (n > 3588 && n < 3592) || n = 3593)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
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
.elseif(n = 3522)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $68,X ;hides 'pla'
	nop
	inx
	Jne *-5
.elseif(n = 3523)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	pla
	inx
	Jne *-5
.elseif(n = 3524)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	nop
	pla
	inx
	Jne *-5
.elseif(n = 3526 || n = 3556 || n = 3586)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3528)
	; 8 bytes
	ldy #167 ;hides 'lax zp'
	iny ;first loop only
	brk
	dey
	Jne *-5
.elseif(n = 3529 || n = 3547 || n = 3565 || n = 3583)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3531 || n = 3549 || n = 3567)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3532 || n = 3568)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3533)
	; 8 bytes
	ldy #186 ;hides 'tsx'
	jsr @nearby_rts
	dey
	Jne *-5
.elseif(n = 3535)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	inx
	Jne *-5
.elseif(n = 3536)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $68 ;hides 'pla'
	nop
	inx
	Jne *-5
.elseif(n = 3538)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	inx
	Jne *-5
.elseif(n = 3539)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	nop
	pla
	inx
	Jne *-5
.elseif(n = 3543 || n = 3557 || n = 3571)
	; 8 bytes
	ldx #(n-1)/14
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3550 || n = 3592)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3551)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3553 || n = 3569)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3563)
	; 8 bytes
	ldx #58 ;hides 'nop'
	delay_n 11
	inx
	Jne *-5
.elseif(n = 3585)
	; 8 bytes
	ldx #0
	delay_n 9
	dex
	Jne *-4
.elseif(n = 3587)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
; 3594 to 3632
.if((n > 3593 && n < 3597) || n = 3598 || n = 3602 || (n > 3605 && n < 3609) || n = 3611 || n = 3612 || n = 3615 || (n > 3623 && n < 3631) || n = 3632)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3597)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	lda $6868 ;hides two 'pla's
	dex
	Jne *-5
.elseif(n = 3599)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 11
	inx
	Jne *-5
.elseif(n = 3600)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	lda $68 ;hides 'pla'
	pla
	dex
	Jne *-5
.elseif(n = 3601 || n = 3619)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3603)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3604)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3605 || n = 3622)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3609)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 3610)
	; 8 bytes
	ldx #75 ;hides 'asr #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 3613)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3614)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	cmp $6868 ;hides two 'pla's
	dex
	Jne *-5
.elseif(n = 3616 || n = 3631)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3617)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3618)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	lda $68,X ;hides 'pla'
	pla
	dex
	Jne *-5
.elseif(n = 3620)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	delay_n 8
	dex
	Jne *-5
.elseif(n = 3621)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3623)
	; 8 bytes
	pla
	delay_n (n-4)
.endif
.else
; 3633 to 3671
.if(n = 3633 || n = 3649 || n = 3665)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3634)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3635)
	; 8 bytes
	ldy #202 ;hides 'dex'
	delay_n 11
	dey
	Jne *-5
.elseif(n = 3636 || n = 3638 || n = 3645 || n = 3648 || n = 3652 || n = 3654 || (n > 3661 && n < 3665) || (n > 3666 && n < 3670))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3637)
	; 7 bytes
	php
	lax #14
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3639)
	; 7 bytes
	ldx #74 ;hides 'lsr a'
	brk
	inx
	Jne *-4
.elseif(n = 3640 || n = 3642 || n = 3656)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3641 || n = 3643 || n = 3657)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3644)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3646)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3647)
	; 8 bytes
	ldx #152 ;hides 'tya'
	delay_n 17
	dex
	Jne *-5
.elseif(n = 3650)
	; 8 bytes
	ldx #73 ;hides 'eor #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 3651)
	; 8 bytes
	ldx #90 ;hides 'nop'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 3653)
	; 6 bytes
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
.elseif(n = 3655)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3658)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3659)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 3660)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3661)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3666)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 3670)
	; 8 bytes
	ldx #11 ;hides 'anc #imm'
	lda $6868 ;hides two 'pla's
	inx
	Jne *-5
.elseif(n = 3671)
	; 8 bytes
	ldx #216 ;hides 'cld'
	delay_n 10
	dex
	Jne *-5
.endif
.endif
.else
.if((n >= 3672) && (n < 3711))
; 3672 to 3710
.if(n = 3672 || n = 3678 || n = 3680 || (n > 3681 && n < 3689) || n = 3696 || n = 3698 || n = 3699 || n = 3702 || n = 3708 || n = 3710)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3673 || n = 3691 || n = 3709)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3674)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 3675 || n = 3693)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3676 || n = 3706)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3677)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3679)
	; 8 bytes
	ldx #26 ;hides 'nop'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3681 || n = 3701)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3689)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	delay_n 8
	inx
	Jne *-4
.elseif(n = 3690 || n = 3707)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3692 || n = 3694)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3695)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
.elseif(n = 3697)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3700)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $6868 ;hides two 'pla's
	inx
	Jne *-5
.elseif(n = 3703)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $68 ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3704)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	lda $68,X ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3705)
	; 8 bytes
	ldx #218 ;hides 'nop'
	delay_n 10
	dex
	Jne *-5
.endif
.else
; 3711 to 3750
.if(n = 3711)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 9
	inx
	Jne *-5
.elseif(n = 3712 || n = 3730 || n = 3748)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3713)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif((n > 3713 && n < 3718) || n = 3720 || n = 3725 || n = 3726 || n = 3728 || (n > 3731 && n < 3736) || n = 3737 || n = 3738 || n = 3740 || n = 3742 || n = 3744 || n = 3746)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3718)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3719)
	; 7 bytes
	ldy #186 ;hides 'tsx'
	brk
	dey
	Jne *-4
.elseif(n = 3721)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3722)
	; 8 bytes
	pha
	ldx #248 ;hides 'sed'
	delay_n 8
	dex
	Jne *-4
.elseif(n = 3723)
	; 8 bytes
	pla
	ldx #248 ;hides 'sed'
	delay_n 8
	dex
	Jne *-4
.elseif(n = 3724 || n = 3741)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3727 || n = 3745)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3729 || n = 3747)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3731)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3736)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3739)
	; 8 bytes
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 3743)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3749)
	; 7 bytes
	ldx #250 ;hides 'nop'
	delay_n 8
	dex
	Jne *-4
.elseif(n = 3750)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	pla
	pha
	inx
	Jne *-5
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
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3752 || n = 3784)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3753 || n = 3767)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3754)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.elseif(n = 3755 || n = 3768 || n = 3769 || n = 3771 || n = 3776 || n = 3778 || n = 3779 || n = 3782 || (n > 3785 && n < 3790))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3756)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $68 ;hides 'pla'
	pha
	inx
	Jne *-5
.elseif(n = 3757)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $48 ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3758)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3759)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3760)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	lda $4868 ;hides 'pla' and 'pha'
	inx
	Jne *-5
.elseif(n = 3761)
	; 8 bytes
	ldx #58 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 3762)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	lda $68 ;hides 'pla'
	pha
	inx
	Jne *-5
.elseif(n = 3763 || n = 3781)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3764)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	lda $48,X ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3765)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	nop
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3770)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 3772)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 3773)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3774)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 3775)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3777)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3780)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3783)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3785)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.endif
.else
; 3790 to 3828
.if(n = 3790 || n = 3794 || n = 3795 || n = 3797 || n = 3798 || n = 3800 || n = 3804 || n = 3806 || n = 3808 || n = 3810 || (n > 3811 && n < 3817) || n = 3818 || n = 3822 || n = 3824 || n = 3827 || n = 3828)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
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
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3796 || n = 3811)
	; 8 bytes
	ldx #(n-1)/15
	delay_n 10
	dex
	Jne *-4
.elseif(n = 3799 || n = 3817)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3801 || n = 3821)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3802 || n = 3823)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3803)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3805)
	; 8 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 3807)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 3819)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3820)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.endif
.else
; 3829 to 3906
.if(n = 3829 || n = 3895)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif((n > 3829 && n < 3835) || n = 3836 || n = 3840 || n = 3842 || (n > 3844 && n < 3851) || n = 3852 || n = 3854 || n = 3858 || n = 3859 || n = 3862 || n = 3864 || (n > 3865 && n < 3870) || n = 3872 || n = 3876 || (n > 3877 && n < 3881) || (n > 3881 && n < 3886) || n = 3888 || n = 3890 || n = 3899 || n = 3904)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3835 || n = 3853 || n = 3871 || n = 3889)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3837)
	; 8 bytes
	ldy #202 ;hides 'dex'
	jsr @nearby_rts
	dey
	Jne *-5
.elseif(n = 3838 || n = 3874 || n = 3892 || n = 3897)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3839 || n = 3875 || n = 3893 || n = 3898)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3841 || n = 3861 || n = 3881 || n = 3901)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3843 || n = 3860 || n = 3877)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3844 || n = 3865 || n = 3886)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3851)
	; 8 bytes
	ldx #42 ;hides 'rol a'
	delay_n 11
	inx
	Jne *-5
.elseif(n = 3855 || n = 3873 || n = 3891 || n = 3896)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3856)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
.elseif(n = 3857 || n = 3905)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3863)
	; 8 bytes
	ldx #168 ;hides 'tay'
	delay_n 16
	dex
	Jne *-5
.elseif(n = 3870)
	; 8 bytes
	ldx #194 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 3887)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3894)
	; 7 bytes
	php
	lax #15
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3900)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $68 ;hides 'pla'
	pha
	dex
	Jne *-5
.elseif(n = 3902)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pla
	dex
	Jne *-5
.elseif(n = 3903)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3906)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	brk
	inx
	Jne *-5
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
; 3907 to 3984
.if(n = 3907 || n = 3925 || n = 3943 || n = 3961 || n = 3979)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3908 || (n > 3911 && n < 3916) || n = 3918 || n = 3920 || n = 3922 || n = 3924 || (n > 3929 && n < 3935) || n = 3936 || n = 3938 || n = 3940 || n = 3942 || n = 3944 || (n > 3949 && n < 3953) || (n > 3953 && n < 3959) || n = 3960 || n = 3966 || n = 3968 || (n > 3970 && n < 3977) || n = 3978 || n = 3980 || n = 3984)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3909 || n = 3927 || n = 3963)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3910 || n = 3946 || n = 3964 || n = 3982)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3911 || n = 3928 || n = 3945 || n = 3962)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3916)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 3917 || n = 3939 || n = 3983)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 3919)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 3921 || n = 3941 || n = 3981)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 3923)
	; 8 bytes
	ldx #218 ;hides 'nop'
	delay_n 11
	dex
	Jne *-5
.elseif(n = 3926)
	; 8 bytes
	ldx #232 ;hides 'inx'
	delay_n 8
	dex
	dex
	Jne *-5
.elseif(n = 3929 || n = 3947 || n = 3965)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3935)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	nop
	pla
	pha
	inx
	Jne *-5
.elseif(n = 3937)
	; 8 bytes
	ldx #232 ;hides 'inx'
	brk
	inx
	inx
	Jne *-5
.elseif(n = 3948)
	; 8 bytes
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 3949)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 3953)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3959)
	; 7 bytes
	ldx #58 ;hides 'nop'
	brk
	inx
	Jne *-4
.elseif(n = 3967)
	; 8 bytes
	ldx #248 ;hides 'sed'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 3969)
	; 8 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 3970)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 3977)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 10
	dex
	Jne *-5
.endif
.else
.if((n >= 3985) && (n < 4024))
; 3985 to 4023
.if(n = 3985)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 3986 || n = 3987 || n = 3989 || n = 3990 || n = 3993 || n = 3995 || n = 3998 || n = 4004 || n = 4006 || n = 4007 || n = 4020 || n = 4022 || n = 4023)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 3988)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $6868 ;hides two 'pla's
	inx
	Jne *-5
.elseif(n = 3991)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $68 ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 3992)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	.byte $AE,0,0 ;ldx abs; hides 'brk'
	dey
	Jne *-5
.elseif(n = 3994)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	delay_n 8
	inx
	Jne *-5
.elseif(n = 3996 || n = 4013)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 3997 || n = 4015)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 3999)
	; 7 bytes
	ldx #56 ;hides 'sec'
	brk
	inx
	Jne *-4
.elseif(n = 4000)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	delay_n 8
	inx
	Jne *-5
.elseif(n = 4001 || n = 4021)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4002 || n = 4018)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4003)
	; 8 bytes
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 4005)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	lda $6868 ;hides two 'pla's
	inx
	Jne *-5
.elseif(n = 4008)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	nop $68 ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 4009)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	lda $68,X ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 4010)
	; 8 bytes
	ldx #201 ;hides 'cmp #imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 4011)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	delay_n 8
	inx
	Jne *-5
.elseif(n = 4012)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4014)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	lda $68,X ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 4016)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	nop
	delay_n 8
	inx
	Jne *-5
.elseif(n = 4017)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4019)
	; 8 bytes
	pla
	delay_n (n-4)
.endif
.else
; 4024 to 4062
.if(n = 4024 || n = 4026 || n = 4029 || n = 4034 || n = 4038 || n = 4040 || (n > 4043 && n < 4047) || n = 4048 || n = 4052 || (n > 4055 && n < 4059) || n = 4060)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 4025)
	; 8 bytes
	ldy #182 ;hides 'ldx zp,y'
	iny ;first loop only
	brk
	dey
	Jne *-5
.elseif(n = 4027)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 4028)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 8
	inx
	Jne *-5
.elseif(n = 4030 || n = 4047)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4031)
	; 8 bytes
	ldx #168 ;hides 'tay'
	delay_n 17
	dex
	Jne *-5
.elseif(n = 4032)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	delay_n 8
	inx
	Jne *-5
.elseif(n = 4033 || n = 4051)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4035 || n = 4053)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4036 || n = 4042 || n = 4062)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4037 || n = 4043 || n = 4055)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 4039)
	; 7 bytes
	ldy #202 ;hides 'dex'
	brk
	dey
	Jne *-4
.elseif(n = 4041 || n = 4061)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4049)
	; 8 bytes
	ldx #(n-1)/16
	delay_n 11
	dex
	Jne *-4
.elseif(n = 4050)
	; 8 bytes
	ldy #203 ;hides 'sbx #imm'
	.byte $AE,0,0 ;ldx abs; hides 'brk'
	dey
	Jne *-5
.elseif(n = 4054)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4059)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	brk
	Jne *-4
.endif
.endif
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
; 4063 to 4140
.if(n = 4063 || n = 4073 || n = 4109 || n = 4127)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 4064 || n = 4081 || n = 4098 || n = 4115 || n = 4132)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4065)
	; 8 bytes
	ldx #42 ;hides 'rol a'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif((n > 4065 && n < 4069) || n = 4070 || n = 4074 || (n > 4075 && n < 4079) || n = 4080 || (n > 4081 && n < 4087) || n = 4088 || n = 4092 || n = 4094 || n = 4095 || n = 4099 || n = 4100 || n = 4102 || n = 4104 || n = 4106 || (n > 4109 && n < 4115) || (n > 4117 && n < 4121) || n = 4122 || n = 4124 || n = 4128 || n = 4129 || n = 4131 || (n > 4132 && n < 4137) || n = 4140)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 4069 || n = 4087 || n = 4105 || n = 4123)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4071 || n = 4089 || n = 4107 || n = 4125)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4072 || n = 4090 || n = 4108)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4075 || n = 4096 || n = 4117 || n = 4138)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4079)
	; 8 bytes
	ldy #170 ;hides 'tax'
	delay_n 17
	dey
	Jne *-5
.elseif(n = 4091)
	; 8 bytes
	ldy #186 ;hides 'tsx'
	nop
	brk
	dey
	Jne *-5
.elseif(n = 4093)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 4097)
	; 8 bytes
	ldx #0
	delay_n 11
	dex
	Jne *-4
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
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 4126)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 4130)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	delay_n 8
	dex
	Jne *-5
.elseif(n = 4137)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 4139)
	; 8 bytes
	ldx #26 ;hides 'nop'
	delay_n 11
	inx
	Jne *-5
.endif
.else
; 4141 to 4218
.if(n = 4141 || n = 4159 || n = 4177 || n = 4195 || n = 4213)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4142 || n = 4146 || n = 4148 || n = 4150 || n = 4152 || n = 4156 || n = 4157 || n = 4160 || n = 4165 || (n > 4166 && n < 4175) || n = 4176 || n = 4178 || n = 4182 || n = 4184 || (n > 4185 && n < 4195) || n = 4196 || n = 4202 || (n > 4203 && n < 4211) || n = 4212 || n = 4214 || n = 4218)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 4143 || n = 4153 || n = 4179 || n = 4197 || n = 4215)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4144)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop $68 ;hides 'pla'
	pla
	dex
	Jne *-5
.elseif(n = 4145)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	lda $68,X ;hides 'pla'
	pla
	dex
	Jne *-5
.elseif(n = 4147)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	delay_n 8
	dex
	Jne *-5
.elseif(n = 4149 || n = 4166 || n = 4183 || n = 4200 || n = 4217)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4151)
	; 7 bytes
	php
	lax #16
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 4154 || n = 4162 || n = 4198 || n = 4216)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4155 || n = 4163 || n = 4199)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 4158)
	; 8 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 4161 || n = 4181 || n = 4201)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4164)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	delay_n 8
	dex
	Jne *-5
.elseif(n = 4175)
	; 8 bytes
	ldx #24 ;hides 'clc'
	delay_n 11
	inx
	Jne *-5
.elseif(n = 4180)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4185)
	; 8 bytes
	ldx #74 ;hides 'lsr a'
	delay_n 16
	inx
	Jne *-5
.elseif(n = 4203)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 4211)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 11
	dex
	Jne *-5
.endif
.endif
.else
; 4219 to 4375
.if(n = 4219 || n = 4220 || n = 4223 || n = 4224 || (n > 4225 && n < 4231) || n = 4232 || (n > 4235 && n < 4241) || n = 4242 || n = 4244 || n = 4245 || n = 4248 || (n > 4253 && n < 4261) || n = 4263 || (n > 4271 && n < 4277) || n = 4278 || n = 4280 || n = 4284 || n = 4286 || (n > 4291 && n < 4301) || n = 4304 || (n > 4307 && n < 4313) || (n > 4313 && n < 4319) || n = 4320 || n = 4326 || (n > 4327 && n < 4335) || n = 4337 || n = 4338 || n = 4340 || (n > 4343 && n < 4348) || (n > 4348 && n < 4353) || n = 4354 || n = 4356 || n = 4358 || (n > 4363 && n < 4367) || n = 4368 || (n > 4369 && n < 4375))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 4221 || n = 4241 || n = 4261 || n = 4281 || n = 4301 || n = 4341 || n = 4361)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4222 || n = 4243 || n = 4264 || n = 4306 || n = 4327 || n = 4348)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4225 || n = 4247 || n = 4291 || n = 4313 || n = 4335)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 4231 || n = 4249 || n = 4267 || n = 4285 || n = 4303 || n = 4321 || n = 4339 || n = 4357 || n = 4375)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4233 || n = 4269 || n = 4287 || n = 4305 || n = 4323)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4234 || n = 4251 || n = 4268 || n = 4302 || n = 4336)
	; 8 bytes
	ldx #(n-1)/17
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4235 || n = 4253 || n = 4266 || n = 4271 || n = 4283 || n = 4289 || n = 4307 || n = 4325 || n = 4343 || n = 4363)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 4246)
	; 8 bytes
	ldy #136 ;hides 'dey'
	delay_n 26
	Jne *-5
.elseif(n = 4250)
	; 8 bytes
	ldx #43 ;hides 'anc #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 4252 || n = 4265 || n = 4270 || n = 4282 || n = 4288 || n = 4322 || n = 4324 || n = 4342 || n = 4360 || n = 4362)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4262)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 16
	Jne *-4
.elseif(n = 4277)
	; 8 bytes
	ldy #186 ;hides 'tsx'
	delay_n 16
	dey
	Jne *-5
.elseif(n = 4279)
	; 7 bytes
	ldx #42 ;hides 'rol a'
	brk
	inx
	Jne *-4
.elseif(n = 4290)
	; 8 bytes
	ldx #41 ;hides 'and #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 4319)
	; 7 bytes
	ldx #216 ;hides 'cld'
	brk
	dex
	Jne *-4
.elseif(n = 4353)
	; 8 bytes
	ldx #0
	jsr @nearby_rts
	dex
	Jne *-4
.elseif(n = 4355)
	; 8 bytes
	ldx #58 ;hides 'nop'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 4359)
	; 7 bytes
	ldx #218 ;hides 'nop'
	brk
	dex
	Jne *-4
.elseif(n = 4367)
	; 8 bytes
	ldx #74 ;hides 'lsr a'
	delay_n 17
	inx
	Jne *-5
.elseif(n = 4369)
	; 8 bytes
	ldx #26 ;hides 'nop'
	jsr @nearby_rts
	inx
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
; 4376 to 4531
.if(n = 4376 || n = 4380 || (n > 4381 && n < 4390) || n = 4391 || n = 4392 || n = 4394 || n = 4398 || n = 4400 || (n > 4401 && n < 4407) || n = 4409 || (n > 4415 && n < 4421) || n = 4422 || (n > 4423 && n < 4427) || n = 4428 || n = 4430 || (n > 4433 && n < 4441) || n = 4442 || n = 4444 || n = 4446 || n = 4448 || n = 4452 || (n > 4454 && n < 4461) || n = 4462 || n = 4464 || n = 4466 || (n > 4469 && n < 4474) || n = 4475 || (n > 4476 && n < 4481) || n = 4482 || n = 4484 || n = 4488 || (n > 4489 && n < 4495) || (n > 4495 && n < 4499) || n = 4500 || n = 4502 || (n > 4505 && n < 4509) || (n > 4511 && n < 4516) || n = 4517 || n = 4518 || n = 4520 || (n > 4523 && n < 4532))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 4377 || n = 4395 || n = 4410 || n = 4413 || n = 4431 || n = 4449 || n = 4467 || n = 4485 || n = 4503)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4378 || n = 4396 || n = 4414 || n = 4450 || n = 4468 || n = 4486 || n = 4504 || n = 4522)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4379 || n = 4423 || n = 4489 || n = 4511)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 4381 || n = 4401 || n = 4421 || n = 4441 || n = 4461 || n = 4481 || n = 4521)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4390 || n = 4432 || n = 4453 || n = 4474 || n = 4495 || n = 4516)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4393 || n = 4411 || n = 4429 || n = 4447 || n = 4465 || n = 4483 || n = 4501 || n = 4519)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4397 || n = 4412 || n = 4415 || n = 4433 || n = 4451 || n = 4469 || n = 4505 || n = 4523)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 4399)
	; 8 bytes
	ldx #56 ;hides 'sec'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 4407)
	; 8 bytes
	ldx #24 ;hides 'clc'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 4408)
	; 7 bytes
	php
	lax #17
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 4427)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	delay_n 11
	inx
	Jne *-5
.elseif(n = 4443)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 4445)
	; 8 bytes
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4454)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 4463)
	; 8 bytes
	ldx #248 ;hides 'sed'
	delay_n 11
	dex
	Jne *-5
.elseif(n = 4476)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 4487)
	; 8 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 4499)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 11
	dex
	Jne *-5
.elseif(n = 4509)
	; 8 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 4510)
	; 8 bytes
	ldx #226 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.endif
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if(n = 4532 || (n > 4533 && n < 4537) || n = 4538 || (n > 4541 && n < 4553) || n = 4554 || n = 4556 || n = 4560 || (n > 4561 && n < 4573) || n = 4574 || n = 4589 || n = 4590 || n = 4592 || n = 4596 || n = 4597 || n = 4607 || n = 4608)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 4533 || n = 4577)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 4537 || n = 4555 || n = 4573 || n = 4591)
	; 7 bytes
	ldx #(n-1)/18
	brk
	dex
	Jne *-3
.elseif(n = 4539 || n = 4557 || n = 4575 || n = 4582 || n = 4593 || n = 4600)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4540 || n = 4576 || n = 4583 || n = 4594 || n = 4601)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4541 || n = 4561 || n = 4581)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4553)
	; 8 bytes
	ldx #58 ;hides 'nop'
	delay_n 16
	inx
	Jne *-5
.elseif(n = 4558 || n = 4579)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4559 || n = 4584 || n = 4595 || n = 4602)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 4578)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 4580)
	; 6 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 4585 || n = 4603)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4586)
	; 8 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 4587 || n = 4605)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4588)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 4598)
	; 6 bytes
	ldx #0 ;hides 'brk imm'
	nop
	dex
	Jne *-3
.elseif(n = 4599)
	; 7 bytes
	ldx #26 ;hides 'nop'
	brk
	inx
	Jne *-4
.elseif(n = 4604)
	; 8 bytes
	delay_n (n-6)
	inc @zptemp,x
.elseif(n = 4606)
	; 8 bytes
	delay_n (n-8)
	slo (@ptrtemp,x)
.elseif(n = 4609)
	; 7 bytes
	ldx #0
	brk
	dex
	Jne *-3
.endif
.else
; 4610 to 4687
.if(n = 4610 || (n > 4613 && n < 4620) || (n > 4621 && n < 4639) || n = 4640 || n = 4644 || (n > 4645 && n < 4652) || (n > 4652 && n < 4661) || n = 4662 || n = 4664 || n = 4666 || (n > 4669 && n < 4673) || (n > 4674 && n < 4679) || n = 4680 || n = 4686)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 4611 || n = 4667)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4612 || n = 4668 || n = 4682)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4613 || n = 4669 || n = 4683)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 4620)
	; 8 bytes
	ldx #37 ;hides 'and zp'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 4621 || n = 4641 || n = 4661 || n = 4681)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4639)
	; 7 bytes
	ldx #24 ;hides 'clc'
	brk
	inx
	Jne *-4
.elseif(n = 4642 || n = 4684)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4643 || n = 4687)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 4645)
	; 8 bytes
	ldy #202 ;hides 'dex'
	delay_n 16
	dey
	Jne *-5
.elseif(n = 4652)
	; 8 bytes
	ldx #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 4663)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 4665)
	; 7 bytes
	php
	lax #18
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 4673)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	jsr @nearby_rts
	inx
	Jne *-5
.elseif(n = 4674)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 4679)
	; 7 bytes
	ldx #234 ;hides 'nop'
	brk
	dex
	Jne *-4
.elseif(n = 4685)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	brk
	dex
	Jne *-5
.endif
.endif
.endif
.else
; 4688 to 5000
.if((n > 4687 && n < 4701) || (n > 4701 && n < 4705) || n = 4706 || n = 4708 || n = 4710 || (n > 4711 && n < 4721) || (n > 4721 && n < 4726) || (n > 4726 && n < 4731) || (n > 4731 && n < 4741) || (n > 4741 && n < 4747) || n = 4748 || n = 4750 || n = 4752 || (n > 4753 && n < 4761) || (n > 4761 && n < 4768) || (n > 4768 && n < 4775) || (n > 4775 && n < 4781) || (n > 4781 && n < 4789) || (n > 4789 && n < 4795) || n = 4796 || n = 4798 || n = 4800 || (n > 4801 && n < 4810) || (n > 4810 && n < 4819) || n = 4820 || (n > 4821 && n < 4831) || (n > 4831 && n < 4841) || (n > 4841 && n < 4847) || (n > 4847 && n < 4852) || (n > 4852 && n < 4861) || n = 4862 || (n > 4863 && n < 4871) || n = 4872 || (n > 4873 && n < 4881) || (n > 4881 && n < 4885) || (n > 4885 && n < 4890) || (n > 4890 && n < 4894) || (n > 4894 && n < 4901) || (n > 4901 && n < 4907) || (n > 4907 && n < 4915) || (n > 4915 && n < 4919) || n = 4920 || n = 4927 || n = 4928 || (n > 4930 && n < 4936) || (n > 4936 && n < 4941) || (n > 4941 && n < 4951) || (n > 4951 && n < 4957) || n = 4958 || n = 4960 || (n > 4963 && n < 4967) || (n > 4967 && n < 4973) || (n > 4973 && n < 4978) || n = 4979 || n = 4980 || (n > 4981 && n < 4995) || (n > 4995 && n < 4999) || n = 5000)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 4701 || n = 4721 || n = 4741 || n = 4761 || n = 4781 || n = 4801 || n = 4821 || n = 4841 || n = 4861 || n = 4881 || n = 4901 || n = 4921 || n = 4941 || n = 4961 || n = 4981)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 4705 || n = 4726 || n = 4747 || n = 4768 || n = 4789 || n = 4810 || n = 4831 || n = 4852 || n = 4873 || n = 4894 || n = 4915 || n = 4936 || n = 4957 || n = 4978)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 4707)
	; 8 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 4709 || n = 4731 || n = 4753 || n = 4775 || n = 4797 || n = 4819 || n = 4863 || n = 4885 || n = 4907 || n = 4929 || n = 4951 || n = 4973 || n = 4995)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 4711)
	; 8 bytes
	ldx #248 ;hides 'sed'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4749)
	; 8 bytes
	ldx #250 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
.elseif(n = 4751)
	; 8 bytes
	ldx #216 ;hides 'cld'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 4795)
	; 8 bytes
	ldx #218 ;hides 'nop'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 4799)
	; 8 bytes
	ldx #56 ;hides 'sec'
	delay_n 17
	inx
	Jne *-5
.elseif(n = 4847)
	; 8 bytes
	ldy #202 ;hides 'dex'
	delay_n 17
	dey
	Jne *-5
.elseif(n = 4871)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 19
	Jne *-5
.elseif(n = 4890)
	; 8 bytes
	ldx #11 ;hides 'anc #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 4919)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	brk
	inx
	Jne *-4
.elseif(n = 4922)
	; 7 bytes
	php
	lax #19
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 4923 || n = 4926 || n = 4963)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 4924)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4925 || n = 4962)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4930)
	; 8 bytes
	ldx #9 ;hides 'ora #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
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
.elseif(n = 4999)
	; 7 bytes
	ldx #250 ;hides 'nop'
	brk
	dex
	Jne *-4
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
; 5001 to 5156
.if(n = 5001 || n = 5021 || n = 5041 || n = 5061 || n = 5081 || n = 5101)
	; 8 bytes
	ldx #(n-1)/20
	nop
	brk
	dex
	Jne *-4
.elseif(n = 5002 || n = 5093 || n = 5113)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5003 || n = 5094 || n = 5114)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif((n > 5003 && n < 5013) || (n > 5013 && n < 5017) || n = 5018 || n = 5019 || (n > 5021 && n < 5039) || n = 5040 || (n > 5041 && n < 5059) || n = 5060 || (n > 5062 && n < 5074) || (n > 5074 && n < 5081) || n = 5082 || (n > 5083 && n < 5090) || n = 5091 || (n > 5094 && n < 5101) || n = 5102 || (n > 5105 && n < 5109) || (n > 5114 && n < 5121) || (n > 5121 && n < 5125) || n = 5126 || (n > 5127 && n < 5135) || (n > 5135 && n < 5146) || n = 5148 || (n > 5149 && n < 5157))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 5013)
	; 8 bytes
	ldx #218 ;hides 'nop'
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5017 || n = 5039 || n = 5105 || n = 5127 || n = 5149)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 5020 || n = 5062 || n = 5083 || n = 5104 || n = 5125 || n = 5146)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5059)
	; 8 bytes
	ldx #26 ;hides 'nop'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 5074)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	brk
	Jne *-5
.elseif(n = 5090)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 5092 || n = 5111 || n = 5112)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5103)
	; 8 bytes
	ldx #24 ;hides 'clc'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 5109)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 5110)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	nop
	nop
	dex
	Jne *-4
.elseif(n = 5121)
	; 8 bytes
	ldx #0
	nop
	brk
	dex
	Jne *-4
.elseif(n = 5135)
	; 8 bytes
	ldx #42 ;hides 'rol a'
	delay_n 17
	inx
	Jne *-5
.elseif(n = 5147)
	; 8 bytes
	ldx #234 ;hides 'nop'
	nop
	brk
	dex
	Jne *-5
.endif
.else
; 5157 to 5312
.if(n = 5157 || (n > 5158 && n < 5167) || n = 5168 || n = 5170 || (n > 5171 && n < 5179) || (n > 5183 && n < 5188) || n = 5189 || n = 5190 || n = 5192 || (n > 5193 && n < 5209) || (n > 5209 && n < 5213) || n = 5214 || (n > 5215 && n < 5230) || (n > 5231 && n < 5237) || (n > 5237 && n < 5250) || (n > 5251 && n < 5259) || (n > 5260 && n < 5271) || (n > 5272 && n < 5281) || (n > 5281 && n < 5289) || n = 5290 || n = 5291 || (n > 5293 && n < 5303) || (n > 5303 && n < 5313))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 5158)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 5167 || n = 5188 || n = 5209 || n = 5230 || n = 5251 || n = 5272 || n = 5293)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5169)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 5171 || n = 5193 || n = 5215 || n = 5237 || n = 5259 || n = 5281 || n = 5303)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 5179)
	; 7 bytes
	php
	lax #20
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 5180)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 5181)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5182)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5183)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 5191)
	; 8 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 5213)
	; 8 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 5231)
	; 8 bytes
	ldx #218 ;hides 'nop'
	delay_n 17
	dex
	Jne *-5
.elseif(n = 5250)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	brk
	inx
	Jne *-5
.elseif(n = 5260)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	inx
	Jne *-5
.elseif(n = 5271)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 5289)
	; 8 bytes
	ldx #26 ;hides 'nop'
	delay_n 16
	inx
	Jne *-5
.elseif(n = 5292)
	; 8 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	brk
	inx
	Jne *-5
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
; 5313 to 5468
.if(n = 5313 || (n > 5314 && n < 5325) || (n > 5325 && n < 5335) || (n > 5335 && n < 5345) || n = 5346 || (n > 5349 && n < 5356) || (n > 5356 && n < 5364) || (n > 5370 && n < 5377) || (n > 5377 && n < 5381) || (n > 5381 && n < 5389) || n = 5390 || (n > 5391 && n < 5411) || n = 5412 || (n > 5413 && n < 5435) || n = 5437 || (n > 5440 && n < 5455) || n = 5456 || (n > 5457 && n < 5469))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 5314 || n = 5335)
	; 8 bytes
	ldx #(n-1)/21
	delay_n 16
	dex
	Jne *-4
.elseif(n = 5325 || n = 5391 || n = 5413 || n = 5435 || n = 5457)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 5345)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	pha
	dex
	Jne *-4
.elseif(n = 5347 || n = 5368 || n = 5438)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5348 || n = 5369 || n = 5439)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5349 || n = 5370 || n = 5440)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 5356)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 5364)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	cmp $48 ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 5365)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	lda $48,X ;hides 'pha'
	dex
	Jne *-4
.elseif(n = 5366)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	nop
	pha
	dex
	Jne *-4
.elseif(n = 5367)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	nop
	brk
	dex
	Jne *-5
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
.elseif(n = 5389)
	; 8 bytes
	ldx #244 ;hides 'nop zp,x'
	inx ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 5411)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	nop
	brk
	inx
	Jne *-5
.elseif(n = 5436)
	; 7 bytes
	php
	lax #21
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 5455)
	; 8 bytes
	ldx #248 ;hides 'sed'
	nop
	brk
	dex
	Jne *-5
.endif
.else
; 5469 to 5625
.if((n > 5468 && n < 5479) || (n > 5479 && n < 5499) || n = 5500 || (n > 5501 && n < 5519) || (n > 5519 && n < 5523) || (n > 5523 && n < 5545) || (n > 5545 && n < 5567) || (n > 5567 && n < 5589) || (n > 5589 && n < 5600) || n = 5601 || (n > 5604 && n < 5611) || (n > 5611 && n < 5615) || (n > 5615 && n < 5619))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 5479 || n = 5501 || n = 5523 || n = 5545 || n = 5567 || n = 5589 || n = 5611)
	; 8 bytes
	ldx #(n-1)/22
	delay_n 17
	dex
	Jne *-4
.elseif(n = 5499)
	; 8 bytes
	ldx #250 ;hides 'nop'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 5519)
	; 8 bytes
	ldx #26 ;hides 'nop'
	delay_n 17
	inx
	Jne *-5
.elseif(n = 5600)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	pla
	dex
	Jne *-4
.elseif(n = 5602 || n = 5624)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5603 || n = 5625)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5604 || n = 5623)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 5615)
	; 8 bytes
	ldx #234 ;hides 'nop'
	delay_n 17
	dex
	Jne *-5
.elseif(n = 5619)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	lda $68 ;hides 'pla'
	inx
	Jne *-4
.elseif(n = 5620)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	lda $68,X ;hides 'pla'
	dex
	Jne *-4
.elseif(n = 5621)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	nop
	inx
	Jne *-5
.elseif(n = 5622)
	; 7 bytes
	ldx #0 ;hides 'brk imm'
	nop
	pla
	dex
	Jne *-4
.endif
.endif
.endif
.else
; 5626 to 6250
.if(n = 5626 || n = 5697 || n = 5954 || n = 6211)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif((n > 5626 && n < 5633) || (n > 5633 && n < 5657) || (n > 5657 && n < 5693) || n = 5694 || (n > 5697 && n < 5703) || (n > 5703 && n < 5749) || (n > 5749 && n < 5855) || (n > 5855 && n < 5875) || (n > 5878 && n < 5903) || (n > 5903 && n < 5950) || (n > 5954 && n < 5999) || (n > 5999 && n < 6110) || (n > 6110 && n < 6130) || (n > 6134 && n < 6207) || n = 6208 || (n > 6211 && n < 6251))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 5633)
	; 8 bytes
	ldx #0
	delay_n 17
	dex
	Jne *-4
.elseif(n = 5657)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	delay_n 16
	inx
	Jne *-5
.elseif(n = 5693)
	; 7 bytes
	php
	lax #22
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 5695 || n = 5952 || n = 6209)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5696 || n = 5953 || n = 6210)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5703)
	; 8 bytes
	ldx #248 ;hides 'sed'
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5749)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 16
	dex
	Jne *-5
.elseif(n = 5855)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 5875)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	.byte $CD,$E6,@zptemp ;cmp abs; hides 'inc @zptemp'
	dex
	Jne *-5
.elseif(n = 5876)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	cmp $48 ;hides 'pha'
	nop
	dex
	Jne *-5
.elseif(n = 5877)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	pha
	inx
	Jne *-5
.elseif(n = 5878)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	inc @zptemp
	dex
	Jne *-5
.elseif(n = 5903)
	; 8 bytes
	ldx #10 ;hides 'asl a'
	delay_n 17
	inx
	Jne *-5
.elseif(n = 5950)
	; 7 bytes
	php
	lax #23
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 5951)
	; 8 bytes
	ldx #248 ;hides 'sed'
	delay_n 17
	dex
	Jne *-5
.elseif(n = 5999)
	; 8 bytes
	ldx #250 ;hides 'nop'
	delay_n 17
	dex
	Jne *-5
.elseif(n = 6110)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 6
	dex
	Jne *-5
.elseif(n = 6130)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	cmp $4848 ;hides two 'pha's
	inx
	Jne *-5
.elseif(n = 6131)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	lda $68 ;hides 'pla'
	nop
	inx
	Jne *-5
.elseif(n = 6132)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	cmp $48 ;hides 'pha'
	pha
	dex
	Jne *-5
.elseif(n = 6133)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	lda $EA ;hides 'nop'
	pla
	inx
	Jne *-5
.elseif(n = 6134)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 6
	dex
	Jne *-5
.elseif(n = 6207)
	; 7 bytes
	php
	lax #24
	jsr delay_256x_a_30_clocks
	plp
.endif
.endif
.else
; 6251 to 7500
.if((n > 6250 && n < 6292) || (n > 6292 && n < 6365) || (n > 6365 && n < 6385) || n = 6386 || (n > 6390 && n < 6464) || n = 6465 || (n > 6468 && n < 6620) || (n > 6620 && n < 6640) || n = 6641 || n = 6642 || n = 6645 || (n > 6646 && n < 6721) || n = 6722 || (n > 6725 && n < 6978) || n = 6979 || (n > 6982 && n < 7235) || n = 7236 || (n > 7239 && n < 7492) || n = 7493 || (n > 7496 && n < 7501))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 6292)
	; 8 bytes
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
.elseif(n = 6365)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
.elseif(n = 6385)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	inx
	Jne *-5
.elseif(n = 6387)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	lda $68 ;hides 'pla'
	pha
	inx
	Jne *-5
.elseif(n = 6388)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	lda $48 ;hides 'pha'
	pla
	inx
	Jne *-5
.elseif(n = 6389)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	lda $48,X ;hides 'pha'
	pla
	dex
	Jne *-5
.elseif(n = 6390)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	delay_n 9
	dex
	Jne *-5
.elseif(n = 6464)
	; 7 bytes
	php
	lax #25
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 6466 || n = 6723 || n = 6980 || n = 7237 || n = 7494)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 6467 || n = 6724 || n = 6981 || n = 7238 || n = 7495)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 6468 || n = 6725 || n = 6982 || n = 7239 || n = 7496)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 6620)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 8
	dex
	Jne *-5
.elseif(n = 6640)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	lda $6868 ;hides two 'pla's
	dex
	Jne *-5
.elseif(n = 6643)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	lda $68 ;hides 'pla'
	pla
	inx
	Jne *-5
.elseif(n = 6644)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	lda $68,X ;hides 'pla'
	pla
	dex
	Jne *-5
.elseif(n = 6646)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	delay_n 8
	dex
	Jne *-5
.elseif(n = 6721)
	; 7 bytes
	php
	lax #26
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 6978)
	; 7 bytes
	php
	lax #27
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 7235)
	; 7 bytes
	php
	lax #28
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 7492)
	; 7 bytes
	php
	lax #29
	jsr delay_256x_a_30_clocks
	plp
.endif
.endif
.else
; 7501 to 10000
.if((n > 7500 && n < 7749) || n = 7750 || (n > 7753 && n < 7895) || (n > 7895 && n < 7915) || (n > 7915 && n < 7926) || (n > 7926 && n < 8006) || n = 8007 || (n > 8010 && n < 8263) || n = 8264 || (n > 8267 && n < 8520) || n = 8521 || (n > 8524 && n < 8777) || n = 8778 || (n > 8781 && n < 9034) || n = 9035 || (n > 9038 && n < 9291) || n = 9292 || (n > 9295 && n < 9548) || n = 9549 || (n > 9552 && n < 9805) || n = 9806 || (n > 9809 && n < 10001))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 7749)
	; 7 bytes
	php
	lax #30
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 7751 || n = 8008 || n = 8265 || n = 8522 || n = 8779 || n = 9036 || n = 9293 || n = 9550 || n = 9807)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 7752 || n = 8009 || n = 8266 || n = 8523 || n = 8780 || n = 9037 || n = 9294 || n = 9551 || n = 9808)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 7753 || n = 8010 || n = 8267 || n = 8524 || n = 8781 || n = 9038 || n = 9295 || n = 9552 || n = 9809)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 7895)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	brk
	dex
	Jne *-5
.elseif(n = 7915)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dex
	Jne *-5
.elseif(n = 7926)
	; 8 bytes
	ldx #0 ;hides 'brk imm'
	nop
	brk
	dex
	Jne *-5
.elseif(n = 8006)
	; 7 bytes
	php
	lax #31
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 8263)
	; 7 bytes
	php
	lax #32
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 8520)
	; 7 bytes
	php
	lax #33
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 8777)
	; 7 bytes
	php
	lax #34
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 9034)
	; 7 bytes
	php
	lax #35
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 9291)
	; 7 bytes
	php
	lax #36
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 9548)
	; 7 bytes
	php
	lax #37
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 9805)
	; 7 bytes
	php
	lax #38
	jsr delay_256x_a_30_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10062) || n = 10063 || (n > 10066 && n < 10319) || n = 10320 || (n > 10323 && n < 10576) || n = 10577 || (n > 10580 && n < 10833) || n = 10834 || (n > 10837 && n < 11090) || n = 11091 || (n > 11094 && n < 11347) || n = 11348 || (n > 11351 && n < 11604) || n = 11605 || (n > 11608 && n < 11861) || n = 11862 || (n > 11865 && n < 12118) || n = 12119 || (n > 12122 && n < 12375) || n = 12376 || (n > 12379 && n < 12632) || n = 12633 || (n > 12636 && n < 12889) || n = 12890 || (n > 12893 && n < 13146) || n = 13147 || (n > 13150 && n < 13403) || n = 13404 || (n > 13407 && n < 13660) || n = 13661 || (n > 13664 && n < 13917) || n = 13918 || (n > 13921 && n < 14174) || n = 14175 || (n > 14178 && n < 14431) || n = 14432 || (n > 14435 && n < 14688) || n = 14689 || (n > 14692 && n < 14945) || n = 14946 || (n > 14949 && n < 15001))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 10062)
	; 7 bytes
	php
	lax #39
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 10064 || n = 10321 || n = 10578 || n = 10835 || n = 11092 || n = 11349 || n = 11606 || n = 11863 || n = 12120 || n = 12377 || n = 12634 || n = 12891 || n = 13148 || n = 13405 || n = 13662 || n = 13919 || n = 14176 || n = 14433 || n = 14690 || n = 14947)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 10065 || n = 10322 || n = 10579 || n = 10836 || n = 11093 || n = 11350 || n = 11607 || n = 11864 || n = 12121 || n = 12378 || n = 12635 || n = 12892 || n = 13149 || n = 13406 || n = 13663 || n = 13920 || n = 14177 || n = 14434 || n = 14691 || n = 14948)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10066 || n = 10323 || n = 10580 || n = 10837 || n = 11094 || n = 11351 || n = 11608 || n = 11865 || n = 12122 || n = 12379 || n = 12636 || n = 12893 || n = 13150 || n = 13407 || n = 13664 || n = 13921 || n = 14178 || n = 14435 || n = 14692 || n = 14949)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 10319)
	; 7 bytes
	php
	lax #40
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 10576)
	; 7 bytes
	php
	lax #41
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 10833)
	; 7 bytes
	php
	lax #42
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 11090)
	; 7 bytes
	php
	lax #43
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 11347)
	; 7 bytes
	php
	lax #44
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 11604)
	; 7 bytes
	php
	lax #45
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 11861)
	; 7 bytes
	php
	lax #46
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 12118)
	; 7 bytes
	php
	lax #47
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 12375)
	; 7 bytes
	php
	lax #48
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 12632)
	; 7 bytes
	php
	lax #49
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 12889)
	; 7 bytes
	php
	lax #50
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 13146)
	; 7 bytes
	php
	lax #51
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 13403)
	; 7 bytes
	php
	lax #52
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 13660)
	; 7 bytes
	php
	lax #53
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 13917)
	; 7 bytes
	php
	lax #54
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 14174)
	; 7 bytes
	php
	lax #55
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 14431)
	; 7 bytes
	php
	lax #56
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 14688)
	; 7 bytes
	php
	lax #57
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 14945)
	; 7 bytes
	php
	lax #58
	jsr delay_256x_a_30_clocks
	plp
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15202) || n = 15203 || (n > 15206 && n < 15459) || n = 15460 || (n > 15463 && n < 15716) || n = 15717 || (n > 15720 && n < 15973) || n = 15974 || (n > 15977 && n < 16230) || n = 16231 || (n > 16234 && n < 16487) || n = 16488 || (n > 16491 && n < 16744) || n = 16745 || (n > 16748 && n < 17001) || n = 17002 || (n > 17005 && n < 17258) || n = 17259 || (n > 17262 && n < 17515) || n = 17516 || (n > 17519 && n < 17772) || n = 17773 || (n > 17776 && n < 18029) || n = 18030 || (n > 18033 && n < 18286) || n = 18287 || (n > 18290 && n < 18543) || n = 18544 || (n > 18547 && n < 18800) || n = 18801 || (n > 18804 && n < 19057) || n = 19058 || (n > 19061 && n < 19314) || n = 19315 || (n > 19318 && n < 19571) || n = 19572 || (n > 19575 && n < 19828) || n = 19829 || (n > 19832 && n < 20001))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 15202)
	; 7 bytes
	php
	lax #59
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 15204 || n = 15461 || n = 15718 || n = 15975 || n = 16232 || n = 16489 || n = 16746 || n = 17003 || n = 17260 || n = 17517 || n = 17774 || n = 18031 || n = 18288 || n = 18545 || n = 18802 || n = 19059 || n = 19316 || n = 19573 || n = 19830)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 15205 || n = 15462 || n = 15719 || n = 15976 || n = 16233 || n = 16490 || n = 16747 || n = 17004 || n = 17261 || n = 17518 || n = 17775 || n = 18032 || n = 18289 || n = 18546 || n = 18803 || n = 19060 || n = 19317 || n = 19574 || n = 19831)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15206 || n = 15463 || n = 15720 || n = 15977 || n = 16234 || n = 16491 || n = 16748 || n = 17005 || n = 17262 || n = 17519 || n = 17776 || n = 18033 || n = 18290 || n = 18547 || n = 18804 || n = 19061 || n = 19318 || n = 19575 || n = 19832)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 15459)
	; 7 bytes
	php
	lax #60
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 15716)
	; 7 bytes
	php
	lax #61
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 15973)
	; 7 bytes
	php
	lax #62
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 16230)
	; 7 bytes
	php
	lax #63
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 16487)
	; 7 bytes
	php
	lax #64
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 16744)
	; 7 bytes
	php
	lax #65
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 17001)
	; 7 bytes
	php
	lax #66
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 17258)
	; 7 bytes
	php
	lax #67
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 17515)
	; 7 bytes
	php
	lax #68
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 17772)
	; 7 bytes
	php
	lax #69
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 18029)
	; 7 bytes
	php
	lax #70
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 18286)
	; 7 bytes
	php
	lax #71
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 18543)
	; 7 bytes
	php
	lax #72
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 18800)
	; 7 bytes
	php
	lax #73
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 19057)
	; 7 bytes
	php
	lax #74
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 19314)
	; 7 bytes
	php
	lax #75
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 19571)
	; 7 bytes
	php
	lax #76
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 19828)
	; 7 bytes
	php
	lax #77
	jsr delay_256x_a_30_clocks
	plp
.endif
.endif
.endif

.endif
	rest
.endmacro
