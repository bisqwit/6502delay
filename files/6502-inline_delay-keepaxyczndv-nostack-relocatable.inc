; delay_n, delays n cycles with small code
; Will preserve A, X, Y, S, and P
;
; This version of delay_n does not change any bytes in the $100 page.
;
; Generated code is:
; -313 bytes or less when n <= 627
; -625 bytes or less when n <= 1251
; -938 bytes or less when n <= 1877
; -1250 bytes or less when n <= 2501
; -1563 bytes or less when n <= 3127
; -1875 bytes or less when n <= 3751
; -2188 bytes or less when n <= 4377
; -2500 bytes or less when n <= 5001
; -2813 bytes or less when n <= 5627
; -3125 bytes or less when n <= 6251
; -3438 bytes or less when n <= 6877
; -3750 bytes or less when n <= 7501
; -4063 bytes or less when n <= 8127
; -4375 bytes or less when n <= 8751
; -4688 bytes or less when n <= 9377
; -5000 bytes or less when n <= 10001
; -5313 bytes or less when n <= 10627
; -5625 bytes or less when n <= 11251
; -5938 bytes or less when n <= 11877
; -6250 bytes or less when n <= 12501
; -6563 bytes or less when n <= 13127
; -6875 bytes or less when n <= 13751
; -7188 bytes or less when n <= 14377
; -7500 bytes or less when n <= 15001
; -7813 bytes or less when n <= 15627
; -8125 bytes or less when n <= 16251
; -8438 bytes or less when n <= 16877
; -8750 bytes or less when n <= 17501
; -9063 bytes or less when n <= 18127
; -9375 bytes or less when n <= 18751
; -9688 bytes or less when n <= 19377
; -10000 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=0, avg=1.7, max=5
; - When 11 <= n <= 20: min=0, avg=4.0, max=10
; - When 21 <= n <= 30: min=0, avg=6.5, max=15
; - When 31 <= n <= 50: min=0, avg=10.2, max=25
; - When 51 <= n <= 75: min=0, avg=15.1, max=37
; - When 76 <= n <= 100: min=0, avg=22.9, max=50
; - When 101 <= n <= 200: min=0, avg=37.8, max=100
; - When 201 <= n <= 300: min=0, avg=62.8, max=150
; - When 301 <= n <= 500: min=0, avg=100.2, max=250
; - When 501 <= n <= 750: min=0, avg=156.5, max=375
; - When 751 <= n <= 1000: min=0, avg=219.0, max=500
; - When 1001 <= n <= 2000: min=0, avg=375.2, max=1000
; - When 2001 <= n <= 3000: min=0, avg=625.2, max=1500
; - When 3001 <= n <= 5000: min=0, avg=1000.2, max=2500
; - When 5001 <= n <= 7500: min=0, avg=1562.8, max=3750
; - When 7501 <= n <= 10000: min=0, avg=2187.8, max=5000
; - When 10001 <= n <= 20000: min=0, avg=3750.2, max=10000
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
.elseif((n .mod 2 = 1) && ((n > 2 && n < 20000)))
.error .sprintf("delay_n: INTERNAL ERROR: UNABLE TO DEVISE DELAY CODE FOR %d CYCLES", n)
.elseif(n = 4 || n = 6)
	; 2..3 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 2 = 0) && ((n > 7 && n < 20001)))
	; 4..10000 bytes
	.repeat n/2
	 nop
	.endrepeat
.endif

.endif
	rest
.endmacro
