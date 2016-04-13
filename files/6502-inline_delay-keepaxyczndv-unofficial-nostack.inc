; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will preserve A, X, Y, S, and P
;
; This version of delay_n does not change any bytes in the $100 page.
;
; Generated code is:
; -313 bytes or less when n <= 626
; -625 bytes or less when n <= 1250
; -938 bytes or less when n <= 1876
; -1250 bytes or less when n <= 2500
; -1563 bytes or less when n <= 3126
; -1875 bytes or less when n <= 3750
; -2188 bytes or less when n <= 4376
; -2500 bytes or less when n <= 5000
; -2813 bytes or less when n <= 5626
; -3125 bytes or less when n <= 6250
; -3438 bytes or less when n <= 6876
; -3750 bytes or less when n <= 7500
; -4063 bytes or less when n <= 8126
; -4375 bytes or less when n <= 8750
; -4688 bytes or less when n <= 9376
; -5000 bytes or less when n <= 10000
; -5313 bytes or less when n <= 10626
; -5625 bytes or less when n <= 11250
; -5938 bytes or less when n <= 11876
; -6250 bytes or less when n <= 12500
; -6563 bytes or less when n <= 13126
; -6875 bytes or less when n <= 13750
; -7188 bytes or less when n <= 14376
; -7500 bytes or less when n <= 15000
; -7813 bytes or less when n <= 15626
; -8125 bytes or less when n <= 16250
; -8438 bytes or less when n <= 16876
; -8750 bytes or less when n <= 17500
; -9063 bytes or less when n <= 18126
; -9375 bytes or less when n <= 18750
; -9688 bytes or less when n <= 19376
; -10000 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=3.2, max=5
; - When 11 <= n <= 20: min=6, avg=8.0, max=10
; - When 21 <= n <= 30: min=11, avg=13.0, max=15
; - When 31 <= n <= 50: min=16, avg=20.5, max=25
; - When 51 <= n <= 75: min=26, avg=31.8, max=38
; - When 76 <= n <= 100: min=38, avg=44.2, max=50
; - When 101 <= n <= 200: min=51, avg=75.5, max=100
; - When 201 <= n <= 300: min=101, avg=125.5, max=150
; - When 301 <= n <= 500: min=151, avg=200.5, max=250
; - When 501 <= n <= 750: min=251, avg=313.0, max=375
; - When 751 <= n <= 1000: min=376, avg=438.0, max=500
; - When 1001 <= n <= 2000: min=501, avg=750.5, max=1000
; - When 2001 <= n <= 3000: min=1001, avg=1250.5, max=1500
; - When 3001 <= n <= 5000: min=1501, avg=2000.5, max=2500
; - When 5001 <= n <= 7500: min=2501, avg=3125.5, max=3750
; - When 7501 <= n <= 10000: min=3751, avg=4375.5, max=5000
; - When 10001 <= n <= 20000: min=5001, avg=7500.5, max=10000
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
	nop $04
.elseif(n = 4 || n = 6)
	; 2..3 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 2 = 1) && ((n > 4 && n < 20000)))
	; 3..10000 bytes
	delay_n (n-3)
	nop $04
.elseif((n .mod 2 = 0) && ((n > 7 && n < 20001)))
	; 4..10000 bytes
	.repeat n/2
	 nop
	.endrepeat
.endif

.endif
	rest
.endmacro
