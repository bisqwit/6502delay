; delay_n, delays n cycles with small code
; Will preserve A, X, Y, S, and P
;
; Uses JMP (unrelocatable code).
;
; This version of delay_n does not change any bytes in the $100 page.
;
; Generated code is:
; -313 bytes or less when n <= 624
; -626 bytes or less when n <= 1250
; -938 bytes or less when n <= 1874
; -1251 bytes or less when n <= 2500
; -1563 bytes or less when n <= 3124
; -1876 bytes or less when n <= 3750
; -2188 bytes or less when n <= 4374
; -2501 bytes or less when n <= 5000
; -2813 bytes or less when n <= 5624
; -3126 bytes or less when n <= 6250
; -3438 bytes or less when n <= 6874
; -3751 bytes or less when n <= 7500
; -4063 bytes or less when n <= 8124
; -4376 bytes or less when n <= 8750
; -4688 bytes or less when n <= 9374
; -5001 bytes or less when n <= 10000
; -5314 bytes or less when n <= 10626
; -5626 bytes or less when n <= 11250
; -5939 bytes or less when n <= 11876
; -6251 bytes or less when n <= 12500
; -6564 bytes or less when n <= 13126
; -6876 bytes or less when n <= 13750
; -7189 bytes or less when n <= 14376
; -7501 bytes or less when n <= 15000
; -7814 bytes or less when n <= 15626
; -8126 bytes or less when n <= 16250
; -8439 bytes or less when n <= 16876
; -8751 bytes or less when n <= 17500
; -9064 bytes or less when n <= 18126
; -9376 bytes or less when n <= 18750
; -9689 bytes or less when n <= 19376
; -10001 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=3.7, max=6
; - When 11 <= n <= 20: min=6, avg=8.5, max=11
; - When 21 <= n <= 30: min=11, avg=13.5, max=16
; - When 31 <= n <= 50: min=16, avg=21.0, max=26
; - When 51 <= n <= 75: min=26, avg=32.3, max=39
; - When 76 <= n <= 100: min=38, avg=44.7, max=51
; - When 101 <= n <= 200: min=51, avg=76.0, max=101
; - When 201 <= n <= 300: min=101, avg=126.0, max=151
; - When 301 <= n <= 500: min=151, avg=201.0, max=251
; - When 501 <= n <= 750: min=251, avg=313.5, max=376
; - When 751 <= n <= 1000: min=376, avg=438.5, max=501
; - When 1001 <= n <= 2000: min=501, avg=751.0, max=1001
; - When 2001 <= n <= 3000: min=1001, avg=1251.0, max=1501
; - When 3001 <= n <= 5000: min=1501, avg=2001.0, max=2501
; - When 5001 <= n <= 7500: min=2501, avg=3126.0, max=3751
; - When 7501 <= n <= 10000: min=3751, avg=4376.0, max=5001
; - When 10001 <= n <= 20000: min=5001, avg=7501.0, max=10001
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
	; 3 bytes
	jmp *+3
.elseif((n > 3 && n < 8) || n = 9)
	; 2..6 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 2 = 0) && ((n > 7 && n < 20001)))
	; 4..10000 bytes
	.repeat n/2
	 nop
	.endrepeat
.elseif(n = 11)
	; 7 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13)
	; 8 bytes
	delay_n (n-10)
	delay_n 10
.elseif(n = 15)
	; 9 bytes
	delay_n (n-12)
	delay_n 12
.elseif(n = 17)
	; 10 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 19)
	; 11 bytes
	delay_n (n-16)
	delay_n 16
.elseif(n = 21)
	; 12 bytes
	delay_n (n-18)
	delay_n 18
.elseif(n = 23)
	; 13 bytes
	delay_n (n-20)
	delay_n 20
.elseif((n .mod 2 = 1) && ((n > 24 && n < 20000)))
	; 14..10001 bytes
	jmp *+3
	delay_n (n-3)
.endif

.endif
	rest
.endmacro
