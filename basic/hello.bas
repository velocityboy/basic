1 INPUT "WHAT'S YOUR NAME", NAME$
2 PRINT "HELLO, "; NAME$; ", HOW ARE YOU?"

5 LET A = 2
7 PRINT A
10 PRINT "*","HELLO, WORLD"
12 LET A$ = "*****"
14 PRINT A,A$
17 IF A < 1 THEN 30 ELSE 70
20 PRINT 3.14159
30 PRINT 1+2*3
40 print 1+2+3+4
50 print 1+"A"
60 PRINT 1 / 2
70 PRINT 1 - 2
80 GOSUB 1000
90 GOTO 2000

1000 PRINT "IN GOSUB"
1010 RETURN


2000 FOR J = 1 TO 10 STEP 2
2010 PRINT J
2020 NEXT J
2025 PRINT "DOWN"
2030 FOR J = 10 TO 1 STEP -3
2040 PRINT J
2050 NEXT
2060 PRINT "ABS(-2) = "; ABS(-2)
2065 REM
2066 REM TEST THE BUILT-IN MATH FUNCTIONS
2067 REM
2070 LET PI = 3.141592
2080 PRINT "SIN(PI/2) = "; SIN(PI/2)
2100 PRINT "BYE"
