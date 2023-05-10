var1: data 10
var2: data 15
var3: data 20

ldc var1        ; load value of var1 into the accumulator           load var 1 into A
ldnl 0          ; load non local

ldc var2        ; load var2 into A and move var 1 into A
ldnl 0

add             ; add both values
ldc var3        ; load var 3 in A
ldnl 0
add
stl 0           ; store local
HALT
