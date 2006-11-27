save  %sp, -104, %sp
mov  %i0, %l5
sethi  %hi(0x40031800), %o1
sll  %i0, 0x10, %i0
srl  %i0, 0x10, %i0
lduh  [ %o1 + 0x248 ], %o0
cmp  %o0, %i0
bcc  0x40006a28 <K6502_Step__FUs+18536>
mov  %o1, %l4
sethi  %hi(0x40031800), %o2
ld  [ %o2 + 0x3d0 ], %o1	! 0x40031bd0 <nes_pc>
ldub  [ %o1 ], %o0
mov  %o0, %o3
inc  %o1
mov  %o2, %i0
cmp  %o3, 0xfe
bgu  0x400069f4 <K6502_Step__FUs+18484>
st  %o1, [ %o2 + 0x3d0 ]
sethi  %hi(0x40006800), %o0
or  %o0, 0x23c, %o0	! 0x40006a3c <K6502_Step__FUs+18556>
sll  %o3, 2, %o1
ld  [ %o0 + %o1 ], %o2
jmp  %o2
nop 
sethi  %hi(0x40031800), %l2
ld  [ %i0 + 0x3d0 ], %o1
ld  [ %l2 + 0x3d4 ], %o0
sub  %o1, %o0, %o1
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
sethi  %hi(0x40031800), %l1
ldub  [ %l1 + 0x3d8 ], %o1	! 0x40031bd8 <SP>
or  %i0, 0x3d0, %o7
lduh  [ %o7 + 2 ], %o0
add  %o1, 0xff, %g2
sethi  %hi(0x40031800), %l0
sethi  %hi(0x40032c00), %o5
ldub  [ %l0 + 0x3d9 ], %g3
or  %o5, 0x60, %o5
or  %o1, 0x100, %o1
srl  %o0, 8, %o0
stb  %o0, [ %o5 + %o1 ]
ldub  [ %o7 + 3 ], %o1
or  %g2, 0x100, %o0
add  %g2, -1, %o3
stb  %o1, [ %o5 + %o0 ]
or  %g3, 0x10, %g3
and  %o3, 0xff, %o3
add  %g2, 0x1fe, %g2
stb  %g2, [ %l1 + 0x3d8 ]
add  %o3, 0x100, %o3
or  %g3, 4, %o4
stb  %g3, [ %o5 + %o3 ]
and  %o4, 0xf7, %o4
sethi  %hi(0x1c00), %o2
sethi  %hi(0x40033400), %o1
stb  %o4, [ %l0 + 0x3d9 ]
ld  [ %o1 + 0x74 ], %o3
or  %o2, 0x3ff, %o0
ldub  [ %o3 + %o0 ], %o1
or  %o2, 0x3fe, %o2
ldub  [ %o3 + %o2 ], %o0
sll  %o1, 8, %o1
or  %o0, %o1, %o0
st  %o0, [ %i0 + 0x3d0 ]
lduh  [ %o7 + 2 ], %o1
sll  %o1, 0x10, %o1
srl  %o1, 0x1d, %o2
sethi  %hi(0x40033400), %o0
or  %o0, 0x78, %o0	! 0x40033478 <memmap_tbl>
sll  %o2, 2, %o2
ld  [ %o0 + %o2 ], %o5
sethi  %hi(0x40031800), %o4
lduh  [ %l4 + 0x248 ], %o3
ld  [ %o4 + 0x24c ], %o0
srl  %o1, 0x10, %o1
add  %o5, %o1, %o1
add  %o3, 7, %o3
add  %o0, 7, %o0
st  %o1, [ %i0 + 0x3d0 ]
st  %o0, [ %o4 + 0x24c ]
st  %o5, [ %l2 + 0x3d4 ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o3, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o4
sethi  %hi(0x40031800), %o0
ldub  [ %o4 ], %o1
ldub  [ %o0 + 0x3db ], %o3
add  %o3, %o1, %o3
sethi  %hi(0x40032c00), %o2
or  %o2, 0x60, %o2	! 0x40032c60 <RAM>
add  %o3, 1, %o0
ldub  [ %o2 + %o0 ], %o1
ldub  [ %o2 + %o3 ], %o0
sll  %o1, 8, %o1
or  %o0, %o1, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
or  %o1, 0x250, %o1	! 0x40031a50 <readmem_tbl>
sll  %o2, 2, %o2
inc  %o4
ld  [ %o1 + %o2 ], %o3
st  %o4, [ %i0 + 0x3d0 ]
call  %o3
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3da ], %o4	! 0x40031bda <A>
or  %o4, %o0, %o4
sethi  %hi(0x40031800), %o0
sethi  %hi(0x40031800), %g2
or  %o0, 0x3e0, %o0
and  %o4, 0xff, %o2
ldub  [ %o0 + %o2 ], %o3
ldub  [ %g2 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %o5
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o5 + 0x24c ], %o0
and  %o1, 0x7d, %o1
or  %o1, %o3, %o1
add  %o2, 6, %o2
add  %o0, 6, %o0
stb  %o1, [ %g2 + 0x3d9 ]
b  0x4000388c <K6502_Step__FUs+5836>
st  %o0, [ %o5 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %l5, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o5
sethi  %hi(0x40032c00), %o0
ldub  [ %o5 ], %l3
or  %o0, 0x60, %o0
ldub  [ %l3 + %o0 ], %o1
sethi  %hi(0x40031800), %o7
ldub  [ %o7 + 0x3da ], %o4	! 0x40031bda <A>
sethi  %hi(0x40031800), %o0
or  %o4, %o1, %o4
sethi  %hi(0x40031800), %g3
or  %o0, 0x3e0, %o0
ldub  [ %o0 + %o4 ], %o3
ldub  [ %g3 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %g2
lduh  [ %l4 + 0x248 ], %o2
ld  [ %g2 + 0x24c ], %o0
and  %o1, 0x7d, %o1
or  %o1, %o3, %o1
add  %o2, 3, %o2
b  0x400036f4 <K6502_Step__FUs+5428>
add  %o0, 3, %o0
sethi  %hi(0x40031800), %l0
ldub  [ %l0 + 0x3d9 ], %g3	! 0x40031bd9 <F>
and  %g3, 0x7c, %g3
stb  %g3, [ %l0 + 0x3d9 ]
ld  [ %i0 + 0x3d0 ], %o7
ldub  [ %o7 ], %l3
sethi  %hi(0x40032c00), %g2
or  %g2, 0x60, %g2	! 0x40032c60 <RAM>
mov  %l3, %l2
sethi  %hi(0x40031c00), %o2
ldub  [ %g2 + %l2 ], %o1
or  %o2, 0xe0, %o2
sll  %o1, 1, %o1
add  %o2, 1, %o0
ldub  [ %o0 + %o1 ], %o3
sethi  %hi(0x40031800), %l1
ldub  [ %o2 + %o1 ], %o5
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l1 + 0x24c ], %o0
stb  %o5, [ %g2 + %l2 ]
or  %g3, %o3, %g3
add  %o4, 5, %o4
add  %o0, 5, %o0
inc  %o7
st  %o7, [ %i0 + 0x3d0 ]
b  0x40004214 <K6502_Step__FUs+8276>
stb  %g3, [ %l0 + 0x3d9 ]
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3d8 ], %o4	! 0x40031bd8 <SP>
sethi  %hi(0x40031800), %o7
ldub  [ %o7 + 0x3d9 ], %o2	! 0x40031bd9 <F>
sethi  %hi(0x40031800), %g2
and  %o4, 0xff, %o3
sethi  %hi(0x40032c00), %o0
lduh  [ %l4 + 0x248 ], %o5
ld  [ %g2 + 0x24c ], %o1
or  %o2, 0x10, %o2
or  %o0, 0x60, %o0
add  %o3, 0x100, %o3
stb  %o2, [ %o0 + %o3 ]
add  %o5, 3, %o5
add  %o1, 3, %o1
add  %o4, 0xff, %o4
stb  %o4, [ %g3 + 0x3d8 ]
st  %o1, [ %g2 + 0x24c ]
stb  %o2, [ %o7 + 0x3d9 ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o5, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o4
sethi  %hi(0x40031800), %o7
ldub  [ %o4 ], %o1
ldub  [ %o7 + 0x3da ], %o5
sethi  %hi(0x40031800), %o0
b  0x40003378 <K6502_Step__FUs+4536>
or  %o5, %o1, %o5
sethi  %hi(0x40031800), %l0
sethi  %hi(0x40031c00), %o5
b  0x400033cc <K6502_Step__FUs+4620>
or  %o5, 0xe0, %o5	! 0x40031ce0 <g_ASLTable>
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %o0
sll  %o0, 8, %o0
or  %l3, %o0, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
sethi  %hi(0x40031800), %o2
inc  %o1
or  %o2, 0x250, %o2
sll  %o3, 2, %o3
ld  [ %o2 + %o3 ], %o4
st  %o1, [ %i0 + 0x3d0 ]
call  %o4
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3da ], %o4	! 0x40031bda <A>
or  %o4, %o0, %o4
sethi  %hi(0x40031800), %o0
sethi  %hi(0x40031800), %g2
or  %o0, 0x3e0, %o0
and  %o4, 0xff, %o2
ldub  [ %o0 + %o2 ], %o3
ldub  [ %g2 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %o5
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o5 + 0x24c ], %o0
and  %o1, 0x7d, %o1
or  %o1, %o3, %o1
add  %o2, 4, %o2
add  %o0, 4, %o0
stb  %o1, [ %g2 + 0x3d9 ]
b  0x4000388c <K6502_Step__FUs+5836>
st  %o0, [ %o5 + 0x24c ]
sethi  %hi(0x40031800), %l2
ldub  [ %l2 + 0x3d9 ], %l0	! 0x40031bd9 <F>
and  %l0, 0x7c, %l0
stb  %l0, [ %l2 + 0x3d9 ]
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %g2
sll  %g2, 8, %g2
sethi  %hi(0x40032c00), %o7
or  %l3, %g2, %g2
or  %o7, 0x60, %o7
sethi  %hi(0x40031c00), %o5
ldub  [ %o7 + %g2 ], %o2
or  %o5, 0xe0, %o5
sll  %o2, 1, %o2
add  %o5, 1, %o0
ldub  [ %o0 + %o2 ], %o3
sethi  %hi(0x40031800), %l1
ldub  [ %o5 + %o2 ], %o0
lduh  [ %l4 + 0x248 ], %g3
ld  [ %l1 + 0x24c ], %o4
stb  %o0, [ %o7 + %g2 ]
or  %l0, %o3, %l0
inc  %o1
add  %g3, 6, %g3
add  %o4, 6, %o4
b  0x40003920 <K6502_Step__FUs+5984>
st  %o1, [ %i0 + 0x3d0 ]
sethi  %hi(0x40031800), %o0
ldub  [ %o0 + 0x3d9 ], %o1	! 0x40031bd9 <F>
btst  0x80, %o1
be  0x4000639c <K6502_Step__FUs+16860>
sethi  %hi(0x40031800), %o3
ld  [ %i0 + 0x3d0 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o3 + 0x24c ], %o0
inc  %o1
add  %o2, 2, %o2
b  0x400069e4 <K6502_Step__FUs+18468>
add  %o0, 2, %o0
ld  [ %i0 + 0x3d0 ], %g2
ldub  [ %g2 ], %l3
mov  %l3, %o5
sethi  %hi(0x40032c00), %o2
or  %o2, 0x60, %o2	! 0x40032c60 <RAM>
add  %o5, 1, %o0
ldub  [ %o2 + %o0 ], %o3
ldub  [ %o2 + %o5 ], %o1
sll  %o3, 8, %o3
sethi  %hi(0x40031800), %o4
or  %o1, %o3, %o1
ldub  [ %o4 + 0x3dc ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
sethi  %hi(0x40031800), %l0
sll  %o0, 0x10, %o0
cmp  %g0, %o1
addx  %g0, 0, %o5
lduh  [ %l4 + 0x248 ], %o3
ld  [ %l0 + 0x24c ], %o4
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o4, %o5, %o4
add  %o3, %o5, %o3
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %o5
inc  %g2
st  %g2, [ %i0 + 0x3d0 ]
sth  %o3, [ %l4 + 0x248 ]
st  %o4, [ %l0 + 0x24c ]
call  %o5
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g2
ldub  [ %g2 + 0x3da ], %o4	! 0x40031bda <A>
b  0x40003654 <K6502_Step__FUs+5268>
or  %o4, %o0, %o4
sethi  %hi(0x40031800), %o0
ld  [ %i0 + 0x3d0 ], %o5
ldub  [ %o0 + 0x3db ], %o1
ldub  [ %o5 ], %o2
sethi  %hi(0x40032c00), %o0
add  %o1, %o2, %o1
or  %o0, 0x60, %o0
ldub  [ %o0 + %o1 ], %o2
sethi  %hi(0x40031800), %o7
ldub  [ %o7 + 0x3da ], %o4	! 0x40031bda <A>
sethi  %hi(0x40031800), %o0
b  0x400036c8 <K6502_Step__FUs+5384>
or  %o4, %o2, %o4
sethi  %hi(0x40031800), %l2
ldub  [ %l2 + 0x3d9 ], %g3	! 0x40031bd9 <F>
and  %g3, 0x7c, %g3
stb  %g3, [ %l2 + 0x3d9 ]
ld  [ %i0 + 0x3d0 ], %l0
sethi  %hi(0x40031800), %o0
ldub  [ %l0 ], %o1
ldub  [ %o0 + 0x3db ], %o7
sethi  %hi(0x40032c00), %g2
add  %o7, %o1, %o7
or  %g2, 0x60, %g2
sethi  %hi(0x40031c00), %o2
ldub  [ %g2 + %o7 ], %o1
or  %o2, 0xe0, %o2
sll  %o1, 1, %o1
add  %o2, 1, %o0
ldub  [ %o0 + %o1 ], %o3
sethi  %hi(0x40031800), %l1
ldub  [ %o2 + %o1 ], %o5
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l1 + 0x24c ], %o0
stb  %o5, [ %g2 + %o7 ]
or  %g3, %o3, %g3
add  %o4, 6, %o4
add  %o0, 6, %o0
inc  %l0
st  %l0, [ %i0 + 0x3d0 ]
b  0x40004214 <K6502_Step__FUs+8276>
stb  %g3, [ %l2 + 0x3d9 ]
sethi  %hi(0x40031800), %o3
sethi  %hi(0x40031800), %o4
ldub  [ %o3 + 0x3d9 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o4 + 0x24c ], %o0
b  0x40006678 <K6502_Step__FUs+17592>
and  %o1, 0xfe, %o1
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o3 ], %l3
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o3 ], %o1
sll  %o1, 8, %o1
sethi  %hi(0x40031800), %o2
or  %l3, %o1, %o1
ldub  [ %o2 + 0x3dc ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
sethi  %hi(0x40031800), %l0
sll  %o0, 0x10, %o0
cmp  %g0, %o1
addx  %g0, 0, %g2
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l0 + 0x24c ], %o5
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o5, %g2, %o5
add  %o4, %g2, %o4
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %g2
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
sth  %o4, [ %l4 + 0x248 ]
st  %o5, [ %l0 + 0x24c ]
call  %g2
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3da ], %o4	! 0x40031bda <A>
b  0x40003854 <K6502_Step__FUs+5780>
or  %o4, %o0, %o4
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o3 ], %l3
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o3 ], %o1
sll  %o1, 8, %o1
sethi  %hi(0x40031800), %o2
or  %l3, %o1, %o1
ldub  [ %o2 + 0x3db ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
sethi  %hi(0x40031800), %l0
sll  %o0, 0x10, %o0
cmp  %g0, %o1
addx  %g0, 0, %g2
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l0 + 0x24c ], %o5
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o5, %g2, %o5
add  %o4, %g2, %o4
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %g2
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
sth  %o4, [ %l4 + 0x248 ]
st  %o5, [ %l0 + 0x24c ]
call  %g2
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3da ], %o4	! 0x40031bda <A>
b  0x40003854 <K6502_Step__FUs+5780>
or  %o4, %o0, %o4
sethi  %hi(0x40031800), %l2
ldub  [ %l2 + 0x3d9 ], %l0	! 0x40031bd9 <F>
and  %l0, 0x7c, %l0
stb  %l0, [ %l2 + 0x3d9 ]
ld  [ %i0 + 0x3d0 ], %o5
ldub  [ %o5 ], %l3
inc  %o5
st  %o5, [ %i0 + 0x3d0 ]
ldub  [ %o5 ], %o1
sethi  %hi(0x40031800), %o2
sll  %o1, 8, %o1
ldub  [ %o2 + 0x3db ], %o0
or  %l3, %o1, %o1
add  %o1, %o0, %o1
sll  %o1, 0x10, %o1
sethi  %hi(0x40032c00), %o7
or  %o7, 0x60, %o7	! 0x40032c60 <RAM>
srl  %o1, 0x10, %o1
sethi  %hi(0x40031c00), %g2
ldub  [ %o7 + %o1 ], %o2
b  0x400038ec <K6502_Step__FUs+5932>
or  %g2, 0xe0, %g2
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
sethi  %hi(0x40031800), %l2
st  %o1, [ %i0 + 0x3d0 ]
ld  [ %l2 + 0x3d4 ], %o0
ldub  [ %o1 ], %o2
sub  %o1, %o0, %o0
st  %o0, [ %i0 + 0x3d0 ]
or  %i0, 0x3d0, %o0
sll  %o2, 8, %o2
lduh  [ %o0 + 2 ], %g2
ldub  [ %o0 + 3 ], %l1
or  %l3, %o2, %o2
st  %o2, [ %i0 + 0x3d0 ]
lduh  [ %o0 + 2 ], %o3
sll  %o3, 0x10, %o3
srl  %o3, 0x1d, %o2
sethi  %hi(0x40033400), %o0
sethi  %hi(0x40031800), %l0
or  %o0, 0x78, %o0
sll  %o2, 2, %o2
ld  [ %o0 + %o2 ], %g3
ldub  [ %l0 + 0x3d8 ], %o1
sethi  %hi(0x40031800), %o7
sethi  %hi(0x40032c00), %o4
lduh  [ %l4 + 0x248 ], %o5
ld  [ %o7 + 0x24c ], %o0
or  %o1, 0x100, %o2
or  %o4, 0x60, %o4
srl  %g2, 8, %g2
stb  %g2, [ %o4 + %o2 ]
add  %o1, 0xff, %o1
or  %o1, 0x100, %o2
srl  %o3, 0x10, %o3
stb  %l1, [ %o4 + %o2 ]
add  %g3, %o3, %o3
add  %o5, 6, %o5
add  %o0, 6, %o0
add  %o1, 0xff, %o1
stb  %o1, [ %l0 + 0x3d8 ]
st  %o3, [ %i0 + 0x3d0 ]
st  %o0, [ %o7 + 0x24c ]
st  %g3, [ %l2 + 0x3d4 ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o5, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o4
sethi  %hi(0x40031800), %o0
ldub  [ %o4 ], %o1
ldub  [ %o0 + 0x3db ], %o3
add  %o3, %o1, %o3
sethi  %hi(0x40032c00), %o2
or  %o2, 0x60, %o2	! 0x40032c60 <RAM>
add  %o3, 1, %o0
ldub  [ %o2 + %o0 ], %o1
ldub  [ %o2 + %o3 ], %o0
sll  %o1, 8, %o1
or  %o0, %o1, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
or  %o1, 0x250, %o1	! 0x40031a50 <readmem_tbl>
sll  %o2, 2, %o2
inc  %o4
ld  [ %o1 + %o2 ], %o3
st  %o4, [ %i0 + 0x3d0 ]
call  %o3
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3da ], %o4	! 0x40031bda <A>
sethi  %hi(0x40031800), %o1
and  %o4, %o0, %o4
sethi  %hi(0x40031800), %g2
or  %o1, 0x3e0, %o1
ldub  [ %o1 + %o4 ], %o3
ldub  [ %g2 + 0x3d9 ], %o2
sethi  %hi(0x40031800), %o5
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o5 + 0x24c ], %o0
and  %o2, 0x7d, %o2
or  %o2, %o3, %o2
add  %o1, 6, %o1
add  %o0, 6, %o0
stb  %o2, [ %g2 + 0x3d9 ]
st  %o0, [ %o5 + 0x24c ]
b  0x40006a0c <K6502_Step__FUs+18508>
stb  %o4, [ %g3 + 0x3da ]
ld  [ %i0 + 0x3d0 ], %o3
sethi  %hi(0x40032c00), %o0
ldub  [ %o3 ], %l3
sethi  %hi(0x40031800), %o4
or  %o0, 0x60, %o0
ldub  [ %l3 + %o0 ], %g3
ldub  [ %o4 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %o2
and  %o1, 0x3d, %o1
ldub  [ %o2 + 0x3da ], %o0
inc  %o3
or  %o4, 0x3d9, %g2
mov  %o1, %o5
and  %g3, 0xc0, %o2
st  %o3, [ %i0 + 0x3d0 ]
btst  %o0, %g3
bne  0x40002abc <K6502_Step__FUs+2300>
stb  %o1, [ %o4 + 0x3d9 ]
or  %o5, 2, %o0
b  0x40002ac0 <K6502_Step__FUs+2304>
or  %o0, %o2, %o3
or  %o5, %o2, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %g2 ]
add  %o1, 3, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 3, %o0
ld  [ %i0 + 0x3d0 ], %o5
sethi  %hi(0x40032c00), %o0
ldub  [ %o5 ], %l3
or  %o0, 0x60, %o0
ldub  [ %l3 + %o0 ], %o1
sethi  %hi(0x40031800), %o7
ldub  [ %o7 + 0x3da ], %o4	! 0x40031bda <A>
sethi  %hi(0x40031800), %o0
and  %o4, %o1, %o4
sethi  %hi(0x40031800), %g3
or  %o0, 0x3e0, %o0
ldub  [ %o0 + %o4 ], %o3
ldub  [ %g3 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %g2
lduh  [ %l4 + 0x248 ], %o2
ld  [ %g2 + 0x24c ], %o0
and  %o1, 0x7d, %o1
or  %o1, %o3, %o1
add  %o2, 3, %o2
b  0x400036f4 <K6502_Step__FUs+5428>
add  %o0, 3, %o0
sethi  %hi(0x40031800), %l1
ldub  [ %l1 + 0x3d9 ], %o0	! 0x40031bd9 <F>
and  %o0, 0x7c, %o7
stb  %o7, [ %l1 + 0x3d9 ]
ld  [ %i0 + 0x3d0 ], %g3
ldub  [ %g3 ], %l3
sethi  %hi(0x40032c00), %g2
or  %g2, 0x60, %g2	! 0x40032c60 <RAM>
mov  %l3, %l2
ldub  [ %g2 + %l2 ], %o1
and  %o0, 1, %o0
sll  %o0, 9, %o0
sethi  %hi(0x40032000), %o2
sll  %o1, 1, %o1
add  %o1, %o0, %o1
b  0x40003ba8 <K6502_Step__FUs+6632>
or  %o2, 0xe0, %o2
sethi  %hi(0x40031800), %g2
ldub  [ %g2 + 0x3d8 ], %o4	! 0x40031bd8 <SP>
inc  %o4
sethi  %hi(0x40032c00), %o0
or  %o0, 0x60, %o0	! 0x40032c60 <RAM>
or  %o4, 0x100, %o1
ldub  [ %o0 + %o1 ], %o2
sethi  %hi(0x40031800), %o5
lduh  [ %l4 + 0x248 ], %o3
ld  [ %o5 + 0x24c ], %o1
or  %o2, 0x20, %o2
add  %o3, 4, %o3
add  %o1, 4, %o1
sethi  %hi(0x40031800), %o0
stb  %o2, [ %o0 + 0x3d9 ]	! 0x40031bd9 <F>
st  %o1, [ %o5 + 0x24c ]
stb  %o4, [ %g2 + 0x3d8 ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o3, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o4
sethi  %hi(0x40031800), %o7
ldub  [ %o4 ], %o1
ldub  [ %o7 + 0x3da ], %o5
sethi  %hi(0x40031800), %o0
b  0x40003378 <K6502_Step__FUs+4536>
and  %o5, %o1, %o5
sethi  %hi(0x40031800), %l0
ldub  [ %l0 + 0x3d9 ], %o4	! 0x40031bd9 <F>
sethi  %hi(0x40031800), %o7
and  %o4, 1, %o0
ldub  [ %o7 + 0x3da ], %o1
sll  %o0, 9, %o0
sethi  %hi(0x40032000), %o2
sll  %o1, 1, %o1
add  %o1, %o0, %o1
b  0x40003d18 <K6502_Step__FUs+7000>
or  %o2, 0xe0, %o2
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %o0
sll  %o0, 8, %o0
or  %l3, %o0, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
sethi  %hi(0x40031800), %o2
inc  %o1
or  %o2, 0x250, %o2
sll  %o3, 2, %o3
ld  [ %o2 + %o3 ], %o4
st  %o1, [ %i0 + 0x3d0 ]
call  %o4
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o3
mov  %o0, %g3
ldub  [ %o3 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %o2
and  %o1, 0x3d, %o1
ldub  [ %o2 + 0x3da ], %o0
or  %o3, 0x3d9, %o5
mov  %o1, %o4
and  %g3, 0xc0, %o2
btst  %o0, %g3
bne  0x40002c88 <K6502_Step__FUs+2760>
stb  %o1, [ %o3 + 0x3d9 ]
or  %o4, 2, %o0
b  0x40002c8c <K6502_Step__FUs+2764>
or  %o0, %o2, %o3
or  %o4, %o2, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %o5 ]
add  %o1, 4, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 4, %o0
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %o0
sll  %o0, 8, %o0
or  %l3, %o0, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
sethi  %hi(0x40031800), %o2
inc  %o1
or  %o2, 0x250, %o2
sll  %o3, 2, %o3
ld  [ %o2 + %o3 ], %o4
st  %o1, [ %i0 + 0x3d0 ]
call  %o4
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3da ], %o4	! 0x40031bda <A>
sethi  %hi(0x40031800), %o1
and  %o4, %o0, %o4
sethi  %hi(0x40031800), %g2
or  %o1, 0x3e0, %o1
ldub  [ %o1 + %o4 ], %o3
ldub  [ %g2 + 0x3d9 ], %o2
sethi  %hi(0x40031800), %o5
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o5 + 0x24c ], %o0
and  %o2, 0x7d, %o2
or  %o2, %o3, %o2
add  %o1, 4, %o1
add  %o0, 4, %o0
stb  %o2, [ %g2 + 0x3d9 ]
st  %o0, [ %o5 + 0x24c ]
b  0x40006a0c <K6502_Step__FUs+18508>
stb  %o4, [ %g3 + 0x3da ]
sethi  %hi(0x40031800), %l2
ldub  [ %l2 + 0x3d9 ], %o2	! 0x40031bd9 <F>
and  %o2, 0x7c, %l0
stb  %l0, [ %l2 + 0x3d9 ]
ld  [ %i0 + 0x3d0 ], %o4
ldub  [ %o4 ], %l3
inc  %o4
st  %o4, [ %i0 + 0x3d0 ]
ldub  [ %o4 ], %g3
sll  %g3, 8, %g3
sethi  %hi(0x40032c00), %o7
or  %l3, %g3, %g3
or  %o7, 0x60, %o7
ldub  [ %o7 + %g3 ], %o1
and  %o2, 1, %o2
sethi  %hi(0x40032000), %g2
b  0x40003f34 <K6502_Step__FUs+7540>
or  %g2, 0xe0, %g2	! 0x400320e0 <g_ROLTable>
sethi  %hi(0x40031800), %o0
ldub  [ %o0 + 0x3d9 ], %o1	! 0x40031bd9 <F>
btst  0x80, %o1
bne  0x400063a0 <K6502_Step__FUs+16864>
ld  [ %i0 + 0x3d0 ], %o2
sethi  %hi(0x40031800), %o3
ld  [ %i0 + 0x3d0 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o3 + 0x24c ], %o0
inc  %o1
add  %o2, 2, %o2
b  0x400069e4 <K6502_Step__FUs+18468>
add  %o0, 2, %o0
ld  [ %i0 + 0x3d0 ], %g2
ldub  [ %g2 ], %l3
mov  %l3, %o5
sethi  %hi(0x40032c00), %o2
or  %o2, 0x60, %o2	! 0x40032c60 <RAM>
add  %o5, 1, %o0
ldub  [ %o2 + %o0 ], %o3
ldub  [ %o2 + %o5 ], %o1
sll  %o3, 8, %o3
sethi  %hi(0x40031800), %o4
or  %o1, %o3, %o1
ldub  [ %o4 + 0x3dc ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
sethi  %hi(0x40031800), %l0
sll  %o0, 0x10, %o0
cmp  %g0, %o1
addx  %g0, 0, %o5
lduh  [ %l4 + 0x248 ], %o3
ld  [ %l0 + 0x24c ], %o4
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o4, %o5, %o4
add  %o3, %o5, %o3
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %o5
inc  %g2
st  %g2, [ %i0 + 0x3d0 ]
sth  %o3, [ %l4 + 0x248 ]
st  %o4, [ %l0 + 0x24c ]
call  %o5
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g2
ldub  [ %g2 + 0x3da ], %o3	! 0x40031bda <A>
sethi  %hi(0x40031800), %o1
and  %o3, %o0, %o3
sethi  %hi(0x40031800), %o5
or  %o1, 0x3e0, %o1
ldub  [ %o1 + %o3 ], %o4
ldub  [ %o5 + 0x3d9 ], %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %l0 + 0x24c ], %o0
and  %o2, 0x7d, %o2
or  %o2, %o4, %o2
add  %o1, 5, %o1
add  %o0, 5, %o0
stb  %o2, [ %o5 + 0x3d9 ]
st  %o0, [ %l0 + 0x24c ]
b  0x40006a0c <K6502_Step__FUs+18508>
stb  %o3, [ %g2 + 0x3da ]
sethi  %hi(0x40031800), %o0
ld  [ %i0 + 0x3d0 ], %o5
ldub  [ %o0 + 0x3db ], %o1
ldub  [ %o5 ], %o2
sethi  %hi(0x40032c00), %o0
add  %o1, %o2, %o1
or  %o0, 0x60, %o0
ldub  [ %o0 + %o1 ], %o2
sethi  %hi(0x40031800), %o7
ldub  [ %o7 + 0x3da ], %o4	! 0x40031bda <A>
sethi  %hi(0x40031800), %o0
b  0x400036c8 <K6502_Step__FUs+5384>
and  %o4, %o2, %o4
sethi  %hi(0x40031800), %l2
ldub  [ %l2 + 0x3d9 ], %o3	! 0x40031bd9 <F>
and  %o3, 0x7c, %l0
stb  %l0, [ %l2 + 0x3d9 ]
ld  [ %i0 + 0x3d0 ], %o7
sethi  %hi(0x40031800), %o0
ldub  [ %o7 ], %o1
ldub  [ %o0 + 0x3db ], %g3
sethi  %hi(0x40032c00), %g2
add  %g3, %o1, %g3
or  %g2, 0x60, %g2
ldub  [ %g2 + %g3 ], %o1
and  %o3, 1, %o3
sethi  %hi(0x40032000), %o2
b  0x400041d4 <K6502_Step__FUs+8212>
or  %o2, 0xe0, %o2	! 0x400320e0 <g_ROLTable>
sethi  %hi(0x40031800), %o3
sethi  %hi(0x40031800), %o4
ldub  [ %o3 + 0x3d9 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o4 + 0x24c ], %o0
b  0x40006678 <K6502_Step__FUs+17592>
or  %o1, 1, %o1
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o3 ], %l3
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o3 ], %o1
sll  %o1, 8, %o1
sethi  %hi(0x40031800), %o2
or  %l3, %o1, %o1
ldub  [ %o2 + 0x3dc ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
sethi  %hi(0x40031800), %l0
sll  %o0, 0x10, %o0
cmp  %g0, %o1
addx  %g0, 0, %g2
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l0 + 0x24c ], %o5
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o5, %g2, %o5
add  %o4, %g2, %o4
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %g2
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
sth  %o4, [ %l4 + 0x248 ]
st  %o5, [ %l0 + 0x24c ]
call  %g2
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3da ], %o3	! 0x40031bda <A>
sethi  %hi(0x40031800), %o1
and  %o3, %o0, %o3
sethi  %hi(0x40031800), %o5
or  %o1, 0x3e0, %o1
ldub  [ %o1 + %o3 ], %o4
ldub  [ %o5 + 0x3d9 ], %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %l0 + 0x24c ], %o0
and  %o2, 0x7d, %o2
or  %o2, %o4, %o2
add  %o1, 4, %o1
add  %o0, 4, %o0
stb  %o2, [ %o5 + 0x3d9 ]
st  %o0, [ %l0 + 0x24c ]
b  0x40006a0c <K6502_Step__FUs+18508>
stb  %o3, [ %g3 + 0x3da ]
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o3 ], %l3
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o3 ], %o1
sll  %o1, 8, %o1
sethi  %hi(0x40031800), %o2
or  %l3, %o1, %o1
ldub  [ %o2 + 0x3db ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
sethi  %hi(0x40031800), %l0
sll  %o0, 0x10, %o0
cmp  %g0, %o1
addx  %g0, 0, %g2
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l0 + 0x24c ], %o5
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o5, %g2, %o5
add  %o4, %g2, %o4
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %g2
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
sth  %o4, [ %l4 + 0x248 ]
st  %o5, [ %l0 + 0x24c ]
call  %g2
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3da ], %o3	! 0x40031bda <A>
sethi  %hi(0x40031800), %o1
and  %o3, %o0, %o3
sethi  %hi(0x40031800), %o5
or  %o1, 0x3e0, %o1
ldub  [ %o1 + %o3 ], %o4
ldub  [ %o5 + 0x3d9 ], %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %l0 + 0x24c ], %o0
and  %o2, 0x7d, %o2
or  %o2, %o4, %o2
add  %o1, 4, %o1
add  %o0, 4, %o0
stb  %o2, [ %o5 + 0x3d9 ]
st  %o0, [ %l0 + 0x24c ]
b  0x40006a0c <K6502_Step__FUs+18508>
stb  %o3, [ %g3 + 0x3da ]
sethi  %hi(0x40031800), %l2
ldub  [ %l2 + 0x3d9 ], %o3	! 0x40031bd9 <F>
and  %o3, 0x7c, %l1
stb  %l1, [ %l2 + 0x3d9 ]
ld  [ %i0 + 0x3d0 ], %o5
ldub  [ %o5 ], %l3
inc  %o5
st  %o5, [ %i0 + 0x3d0 ]
ldub  [ %o5 ], %o2
sethi  %hi(0x40031800), %o1
sll  %o2, 8, %o2
ldub  [ %o1 + 0x3db ], %o0
or  %l3, %o2, %o2
add  %o2, %o0, %o2
sll  %o2, 0x10, %o2
sethi  %hi(0x40032c00), %o7
or  %o7, 0x60, %o7	! 0x40032c60 <RAM>
srl  %o2, 0x10, %o2
ldub  [ %o7 + %o2 ], %o1
and  %o3, 1, %o3
sethi  %hi(0x40032000), %g3
b  0x400044cc <K6502_Step__FUs+8972>
or  %g3, 0xe0, %g3	! 0x400320e0 <g_ROLTable>
sethi  %hi(0x40031800), %l1
ldub  [ %l1 + 0x3d8 ], %o3	! 0x40031bd8 <SP>
inc  %o3
add  %o3, 2, %l0
add  %o3, 1, %o1
and  %l0, 0xff, %o0
sethi  %hi(0x40032c00), %o5
or  %o5, 0x60, %o5	! 0x40032c60 <RAM>
add  %o0, 0x100, %o0
and  %o1, 0xff, %o1
ldub  [ %o5 + %o0 ], %o2
add  %o1, 0x100, %o1
ldub  [ %o5 + %o1 ], %o0
sll  %o2, 8, %o2
add  %o0, %o2, %o0
st  %o0, [ %i0 + 0x3d0 ]
or  %i0, 0x3d0, %o1
lduh  [ %o1 + 2 ], %o2
sll  %o2, 0x10, %o2
srl  %o2, 0x1d, %o1
sethi  %hi(0x40033400), %o0
or  %o0, 0x78, %o0	! 0x40033478 <memmap_tbl>
sll  %o1, 2, %o1
ld  [ %o0 + %o1 ], %o7
sethi  %hi(0x40031800), %g3
or  %o3, 0x100, %o3
ldub  [ %o5 + %o3 ], %o4
lduh  [ %l4 + 0x248 ], %g2
ld  [ %g3 + 0x24c ], %o1
srl  %o2, 0x10, %o2
add  %o7, %o2, %o2
or  %o4, 0x20, %o4
add  %g2, 6, %g2
add  %o1, 6, %o1
sethi  %hi(0x40031800), %o0
sethi  %hi(0x40031800), %o3
stb  %o4, [ %o0 + 0x3d9 ]
st  %o2, [ %i0 + 0x3d0 ]
st  %o1, [ %g3 + 0x24c ]
stb  %l0, [ %l1 + 0x3d8 ]
st  %o7, [ %o3 + 0x3d4 ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %g2, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o4
sethi  %hi(0x40031800), %o0
ldub  [ %o4 ], %o1
ldub  [ %o0 + 0x3db ], %o3
add  %o3, %o1, %o3
sethi  %hi(0x40032c00), %o2
or  %o2, 0x60, %o2	! 0x40032c60 <RAM>
add  %o3, 1, %o0
ldub  [ %o2 + %o0 ], %o1
ldub  [ %o2 + %o3 ], %o0
sll  %o1, 8, %o1
or  %o0, %o1, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
or  %o1, 0x250, %o1	! 0x40031a50 <readmem_tbl>
sll  %o2, 2, %o2
inc  %o4
ld  [ %o1 + %o2 ], %o3
st  %o4, [ %i0 + 0x3d0 ]
call  %o3
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3da ], %o4	! 0x40031bda <A>
xor  %o4, %o0, %o4
sethi  %hi(0x40031800), %o0
sethi  %hi(0x40031800), %g2
or  %o0, 0x3e0, %o0
and  %o4, 0xff, %o2
ldub  [ %o0 + %o2 ], %o3
ldub  [ %g2 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %o5
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o5 + 0x24c ], %o0
and  %o1, 0x7d, %o1
or  %o1, %o3, %o1
add  %o2, 6, %o2
add  %o0, 6, %o0
stb  %o1, [ %g2 + 0x3d9 ]
b  0x4000388c <K6502_Step__FUs+5836>
st  %o0, [ %o5 + 0x24c ]
ld  [ %i0 + 0x3d0 ], %o5
sethi  %hi(0x40032c00), %o0
ldub  [ %o5 ], %l3
or  %o0, 0x60, %o0
ldub  [ %l3 + %o0 ], %o1
sethi  %hi(0x40031800), %o7
ldub  [ %o7 + 0x3da ], %o4	! 0x40031bda <A>
sethi  %hi(0x40031800), %o0
xor  %o4, %o1, %o4
sethi  %hi(0x40031800), %g3
or  %o0, 0x3e0, %o0
ldub  [ %o0 + %o4 ], %o3
ldub  [ %g3 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %g2
lduh  [ %l4 + 0x248 ], %o2
ld  [ %g2 + 0x24c ], %o0
and  %o1, 0x7d, %o1
or  %o1, %o3, %o1
add  %o2, 3, %o2
b  0x400036f4 <K6502_Step__FUs+5428>
add  %o0, 3, %o0
sethi  %hi(0x40031800), %l0
ldub  [ %l0 + 0x3d9 ], %g3	! 0x40031bd9 <F>
and  %g3, 0x7c, %g3
stb  %g3, [ %l0 + 0x3d9 ]
ld  [ %i0 + 0x3d0 ], %o7
ldub  [ %o7 ], %l3
sethi  %hi(0x40032c00), %g2
or  %g2, 0x60, %g2	! 0x40032c60 <RAM>
mov  %l3, %l2
sethi  %hi(0x40031c00), %o2
ldub  [ %g2 + %l2 ], %o1
or  %o2, 0x2e0, %o2
sll  %o1, 1, %o1
add  %o2, 1, %o0
ldub  [ %o0 + %o1 ], %o3
sethi  %hi(0x40031800), %l1
ldub  [ %o2 + %o1 ], %o5
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l1 + 0x24c ], %o0
stb  %o5, [ %g2 + %l2 ]
or  %g3, %o3, %g3
add  %o4, 5, %o4
add  %o0, 5, %o0
inc  %o7
st  %o7, [ %i0 + 0x3d0 ]
b  0x40004214 <K6502_Step__FUs+8276>
stb  %g3, [ %l0 + 0x3d9 ]
sethi  %hi(0x40031800), %o7
ldub  [ %o7 + 0x3d8 ], %o4	! 0x40031bd8 <SP>
sethi  %hi(0x40031800), %g3
and  %o4, 0xff, %o2
sethi  %hi(0x40032c00), %o0
sethi  %hi(0x40031800), %o1
lduh  [ %l4 + 0x248 ], %g2
ld  [ %g3 + 0x24c ], %o3
or  %o0, 0x60, %o0
add  %o2, 0x100, %o2
ldub  [ %o1 + 0x3da ], %o5
stb  %o5, [ %o0 + %o2 ]
add  %g2, 3, %g2
add  %o3, 3, %o3
add  %o4, 0xff, %o4
stb  %o4, [ %o7 + 0x3d8 ]
st  %o3, [ %g3 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %g2, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o4
sethi  %hi(0x40031800), %o7
ldub  [ %o4 ], %o1
ldub  [ %o7 + 0x3da ], %o5
sethi  %hi(0x40031800), %o0
xor  %o5, %o1, %o5
sethi  %hi(0x40031800), %g3
or  %o0, 0x3e0, %o0
ldub  [ %o0 + %o5 ], %o3
ldub  [ %g3 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %g2
lduh  [ %l4 + 0x248 ], %o2
ld  [ %g2 + 0x24c ], %o0
and  %o1, 0x7d, %o1
or  %o1, %o3, %o1
add  %o2, 2, %o2
add  %o0, 2, %o0
inc  %o4
st  %o4, [ %i0 + 0x3d0 ]
stb  %o1, [ %g3 + 0x3d9 ]
st  %o0, [ %g2 + 0x24c ]
stb  %o5, [ %o7 + 0x3da ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %l0
sethi  %hi(0x40031c00), %o5
or  %o5, 0x2e0, %o5	! 0x40031ee0 <g_LSRTable>
ldub  [ %l0 + 0x3da ], %o2
sethi  %hi(0x40031800), %o7
sll  %o2, 1, %o2
add  %o5, 1, %o0
ldub  [ %o0 + %o2 ], %o3
ldub  [ %o7 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %g3
lduh  [ %l4 + 0x248 ], %o4
ld  [ %g3 + 0x24c ], %o0
and  %o1, 0x7c, %o1
ldub  [ %o5 + %o2 ], %g2
or  %o1, %o3, %o1
add  %o4, 2, %o4
add  %o0, 2, %o0
stb  %o1, [ %o7 + 0x3d9 ]
stb  %g2, [ %l0 + 0x3da ]
st  %o0, [ %g3 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o4, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %o0
sll  %o0, 8, %o0
or  %l3, %o0, %o0
st  %o0, [ %i0 + 0x3d0 ]
or  %i0, 0x3d0, %o1
lduh  [ %o1 + 2 ], %o2
sll  %o2, 0x10, %o2
srl  %o2, 0x1d, %o1
sethi  %hi(0x40033400), %o0
or  %o0, 0x78, %o0	! 0x40033478 <memmap_tbl>
sll  %o1, 2, %o1
ld  [ %o0 + %o1 ], %o5
sethi  %hi(0x40031800), %o4
lduh  [ %l4 + 0x248 ], %o3
ld  [ %o4 + 0x24c ], %o0
srl  %o2, 0x10, %o2
add  %o5, %o2, %o2
add  %o3, 3, %o3
add  %o0, 3, %o0
sethi  %hi(0x40031800), %o1
st  %o2, [ %i0 + 0x3d0 ]
b  0x400039b8 <K6502_Step__FUs+6136>
st  %o0, [ %o4 + 0x24c ]
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %o0
sll  %o0, 8, %o0
or  %l3, %o0, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
sethi  %hi(0x40031800), %o2
inc  %o1
or  %o2, 0x250, %o2
sll  %o3, 2, %o3
ld  [ %o2 + %o3 ], %o4
st  %o1, [ %i0 + 0x3d0 ]
call  %o4
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3da ], %o4	! 0x40031bda <A>
xor  %o4, %o0, %o4
sethi  %hi(0x40031800), %o0
sethi  %hi(0x40031800), %g2
or  %o0, 0x3e0, %o0
and  %o4, 0xff, %o2
ldub  [ %o0 + %o2 ], %o3
ldub  [ %g2 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %o5
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o5 + 0x24c ], %o0
and  %o1, 0x7d, %o1
or  %o1, %o3, %o1
add  %o2, 4, %o2
add  %o0, 4, %o0
stb  %o1, [ %g2 + 0x3d9 ]
b  0x4000388c <K6502_Step__FUs+5836>
st  %o0, [ %o5 + 0x24c ]
sethi  %hi(0x40031800), %l2
ldub  [ %l2 + 0x3d9 ], %l0	! 0x40031bd9 <F>
and  %l0, 0x7c, %l0
stb  %l0, [ %l2 + 0x3d9 ]
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %g2
sll  %g2, 8, %g2
sethi  %hi(0x40032c00), %o7
or  %l3, %g2, %g2
or  %o7, 0x60, %o7
sethi  %hi(0x40031c00), %o5
ldub  [ %o7 + %g2 ], %o2
or  %o5, 0x2e0, %o5
sll  %o2, 1, %o2
add  %o5, 1, %o0
ldub  [ %o0 + %o2 ], %o3
sethi  %hi(0x40031800), %l1
ldub  [ %o5 + %o2 ], %o0
lduh  [ %l4 + 0x248 ], %g3
ld  [ %l1 + 0x24c ], %o4
stb  %o0, [ %o7 + %g2 ]
or  %l0, %o3, %l0
inc  %o1
add  %g3, 6, %g3
add  %o4, 6, %o4
b  0x40003920 <K6502_Step__FUs+5984>
st  %o1, [ %i0 + 0x3d0 ]
sethi  %hi(0x40031800), %o0
ldub  [ %o0 + 0x3d9 ], %o1	! 0x40031bd9 <F>
btst  0x40, %o1
be  0x4000639c <K6502_Step__FUs+16860>
sethi  %hi(0x40031800), %o3
ld  [ %i0 + 0x3d0 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o3 + 0x24c ], %o0
inc  %o1
add  %o2, 2, %o2
b  0x400069e4 <K6502_Step__FUs+18468>
add  %o0, 2, %o0
ld  [ %i0 + 0x3d0 ], %g2
ldub  [ %g2 ], %l3
mov  %l3, %o5
sethi  %hi(0x40032c00), %o2
or  %o2, 0x60, %o2	! 0x40032c60 <RAM>
add  %o5, 1, %o0
ldub  [ %o2 + %o0 ], %o3
ldub  [ %o2 + %o5 ], %o1
sll  %o3, 8, %o3
sethi  %hi(0x40031800), %o4
or  %o1, %o3, %o1
ldub  [ %o4 + 0x3dc ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
sethi  %hi(0x40031800), %l0
sll  %o0, 0x10, %o0
cmp  %g0, %o1
addx  %g0, 0, %o5
lduh  [ %l4 + 0x248 ], %o3
ld  [ %l0 + 0x24c ], %o4
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o4, %o5, %o4
add  %o3, %o5, %o3
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %o5
inc  %g2
st  %g2, [ %i0 + 0x3d0 ]
sth  %o3, [ %l4 + 0x248 ]
st  %o4, [ %l0 + 0x24c ]
call  %o5
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g2
ldub  [ %g2 + 0x3da ], %o4	! 0x40031bda <A>
xor  %o4, %o0, %o4
sethi  %hi(0x40031800), %o0
sethi  %hi(0x40031800), %o5
or  %o0, 0x3e0, %o0
and  %o4, 0xff, %o2
ldub  [ %o0 + %o2 ], %o3
ldub  [ %o5 + 0x3d9 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %l0 + 0x24c ], %o0
and  %o1, 0x7d, %o1
or  %o1, %o3, %o1
add  %o2, 5, %o2
add  %o0, 5, %o0
stb  %o1, [ %o5 + 0x3d9 ]
st  %o0, [ %l0 + 0x24c ]
stb  %o4, [ %g2 + 0x3da ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %o0
ld  [ %i0 + 0x3d0 ], %o5
ldub  [ %o0 + 0x3db ], %o1
ldub  [ %o5 ], %o2
sethi  %hi(0x40032c00), %o0
add  %o1, %o2, %o1
or  %o0, 0x60, %o0
ldub  [ %o0 + %o1 ], %o2
sethi  %hi(0x40031800), %o7
ldub  [ %o7 + 0x3da ], %o4	! 0x40031bda <A>
sethi  %hi(0x40031800), %o0
xor  %o4, %o2, %o4
sethi  %hi(0x40031800), %g3
or  %o0, 0x3e0, %o0
ldub  [ %o0 + %o4 ], %o3
ldub  [ %g3 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %g2
lduh  [ %l4 + 0x248 ], %o2
ld  [ %g2 + 0x24c ], %o0
and  %o1, 0x7d, %o1
or  %o1, %o3, %o1
add  %o2, 4, %o2
add  %o0, 4, %o0
inc  %o5
st  %o5, [ %i0 + 0x3d0 ]
stb  %o1, [ %g3 + 0x3d9 ]
st  %o0, [ %g2 + 0x24c ]
stb  %o4, [ %o7 + 0x3da ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %l2
ldub  [ %l2 + 0x3d9 ], %g3	! 0x40031bd9 <F>
and  %g3, 0x7c, %g3
stb  %g3, [ %l2 + 0x3d9 ]
ld  [ %i0 + 0x3d0 ], %l0
sethi  %hi(0x40031800), %o0
ldub  [ %l0 ], %o1
ldub  [ %o0 + 0x3db ], %o7
sethi  %hi(0x40032c00), %g2
add  %o7, %o1, %o7
or  %g2, 0x60, %g2
sethi  %hi(0x40031c00), %o2
ldub  [ %g2 + %o7 ], %o1
or  %o2, 0x2e0, %o2
sll  %o1, 1, %o1
add  %o2, 1, %o0
ldub  [ %o0 + %o1 ], %o3
sethi  %hi(0x40031800), %l1
ldub  [ %o2 + %o1 ], %o5
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l1 + 0x24c ], %o0
stb  %o5, [ %g2 + %o7 ]
or  %g3, %o3, %g3
add  %o4, 6, %o4
add  %o0, 6, %o0
inc  %l0
st  %l0, [ %i0 + 0x3d0 ]
b  0x40004214 <K6502_Step__FUs+8276>
stb  %g3, [ %l2 + 0x3d9 ]
sethi  %hi(0x40031800), %o3
sethi  %hi(0x40031800), %o4
ldub  [ %o3 + 0x3d9 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o4 + 0x24c ], %o0
b  0x40006678 <K6502_Step__FUs+17592>
and  %o1, 0xfb, %o1
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o3 ], %l3
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o3 ], %o1
sll  %o1, 8, %o1
sethi  %hi(0x40031800), %o2
or  %l3, %o1, %o1
b  0x400037ec <K6502_Step__FUs+5676>
ldub  [ %o2 + 0x3dc ], %o0
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o3 ], %l3
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o3 ], %o1
sll  %o1, 8, %o1
sethi  %hi(0x40031800), %o2
or  %l3, %o1, %o1
ldub  [ %o2 + 0x3db ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
sethi  %hi(0x40031800), %l0
sll  %o0, 0x10, %o0
cmp  %g0, %o1
addx  %g0, 0, %g2
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l0 + 0x24c ], %o5
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o5, %g2, %o5
add  %o4, %g2, %o4
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %g2
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
sth  %o4, [ %l4 + 0x248 ]
st  %o5, [ %l0 + 0x24c ]
call  %g2
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3da ], %o4	! 0x40031bda <A>
xor  %o4, %o0, %o4
sethi  %hi(0x40031800), %o0
sethi  %hi(0x40031800), %o5
or  %o0, 0x3e0, %o0
and  %o4, 0xff, %o2
ldub  [ %o0 + %o2 ], %o3
ldub  [ %o5 + 0x3d9 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %l0 + 0x24c ], %o0
and  %o1, 0x7d, %o1
or  %o1, %o3, %o1
add  %o2, 4, %o2
add  %o0, 4, %o0
stb  %o1, [ %o5 + 0x3d9 ]
st  %o0, [ %l0 + 0x24c ]
stb  %o4, [ %g3 + 0x3da ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %l2
ldub  [ %l2 + 0x3d9 ], %l0	! 0x40031bd9 <F>
and  %l0, 0x7c, %l0
stb  %l0, [ %l2 + 0x3d9 ]
ld  [ %i0 + 0x3d0 ], %o5
ldub  [ %o5 ], %l3
inc  %o5
st  %o5, [ %i0 + 0x3d0 ]
ldub  [ %o5 ], %o1
sethi  %hi(0x40031800), %o2
sll  %o1, 8, %o1
ldub  [ %o2 + 0x3db ], %o0
or  %l3, %o1, %o1
add  %o1, %o0, %o1
sll  %o1, 0x10, %o1
sethi  %hi(0x40032c00), %o7
or  %o7, 0x60, %o7	! 0x40032c60 <RAM>
srl  %o1, 0x10, %o1
sethi  %hi(0x40031c00), %g2
ldub  [ %o7 + %o1 ], %o2
or  %g2, 0x2e0, %g2
sll  %o2, 1, %o2
add  %g2, 1, %o0
ldub  [ %o0 + %o2 ], %o3
sethi  %hi(0x40031800), %l1
ldub  [ %g2 + %o2 ], %o0
lduh  [ %l4 + 0x248 ], %g3
ld  [ %l1 + 0x24c ], %o4
stb  %o0, [ %o7 + %o1 ]
or  %l0, %o3, %l0
inc  %o5
add  %g3, 7, %g3
add  %o4, 7, %o4
st  %o5, [ %i0 + 0x3d0 ]
stb  %l0, [ %l2 + 0x3d9 ]
st  %o4, [ %l1 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %g3, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3d8 ], %o3	! 0x40031bd8 <SP>
inc  %o3
add  %o3, 1, %g2
and  %g2, 0xff, %o0
sethi  %hi(0x40032c00), %o1
or  %o1, 0x60, %o1	! 0x40032c60 <RAM>
add  %o0, 0x100, %o0
ldub  [ %o1 + %o0 ], %o2
or  %o3, 0x100, %o3
ldub  [ %o1 + %o3 ], %o0
sll  %o2, 8, %o2
add  %o0, %o2, %o0
inc  %o0
st  %o0, [ %i0 + 0x3d0 ]
or  %i0, 0x3d0, %o1
lduh  [ %o1 + 2 ], %o2
sll  %o2, 0x10, %o2
srl  %o2, 0x1d, %o1
sethi  %hi(0x40033400), %o0
or  %o0, 0x78, %o0	! 0x40033478 <memmap_tbl>
sll  %o1, 2, %o1
ld  [ %o0 + %o1 ], %o5
sethi  %hi(0x40031800), %o4
lduh  [ %l4 + 0x248 ], %o3
ld  [ %o4 + 0x24c ], %o0
srl  %o2, 0x10, %o2
add  %o5, %o2, %o2
add  %o3, 6, %o3
add  %o0, 6, %o0
sethi  %hi(0x40031800), %o1
st  %o2, [ %i0 + 0x3d0 ]
st  %o0, [ %o4 + 0x24c ]
stb  %g2, [ %g3 + 0x3d8 ]
st  %o5, [ %o1 + 0x3d4 ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o3, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o4
sethi  %hi(0x40031800), %o0
ldub  [ %o4 ], %o1
ldub  [ %o0 + 0x3db ], %o3
add  %o3, %o1, %o3
sethi  %hi(0x40032c00), %o2
or  %o2, 0x60, %o2	! 0x40032c60 <RAM>
add  %o3, 1, %o0
ldub  [ %o2 + %o0 ], %o1
ldub  [ %o2 + %o3 ], %o0
sll  %o1, 8, %o1
or  %o0, %o1, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
or  %o1, 0x250, %o1	! 0x40031a50 <readmem_tbl>
sll  %o2, 2, %o2
inc  %o4
ld  [ %o1 + %o2 ], %o3
st  %o4, [ %i0 + 0x3d0 ]
call  %o3
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o4
ldub  [ %o4 + 0x3d9 ], %o1	! 0x40031bd9 <F>
mov  %o0, %g3
sethi  %hi(0x40031800), %o5
ldub  [ %o5 + 0x3da ], %o2	! 0x40031bda <A>
and  %o1, 0x3c, %l0
and  %g3, 0xff, %o0
add  %o0, %o2, %o0
and  %o1, 1, %o1
add  %o1, %o0, %o7
mov  %o7, %l3
xor  %o2, %g3, %o3
xor  %o2, %l3, %o2
sethi  %hi(0x40031800), %o1
and  %o3, 0xff, %o3
and  %o2, 0xff, %o2
or  %o1, 0x3e0, %o1
and  %l3, 0xff, %o0
andn  %o2, %o3, %o3
ldub  [ %o1 + %o0 ], %o1
or  %o4, 0x3d9, %l1
stb  %l0, [ %o4 + 0x3d9 ]
btst  0x80, %o3
be  0x40003a84 <K6502_Step__FUs+6340>
mov  %o5, %g2
or  %o1, 0x40, %o1
cmp  %o7, 0xff
bleu  0x40003a94 <K6502_Step__FUs+6356>
mov  %l0, %o0
or  %o0, 1, %o0
or  %o0, %o1, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %l1 ]
add  %o1, 6, %o1
add  %o0, 6, %o0
b  0x40006a08 <K6502_Step__FUs+18504>
stb  %l3, [ %g2 + 0x3da ]
ld  [ %i0 + 0x3d0 ], %o3
sethi  %hi(0x40032c00), %o0
ldub  [ %o3 ], %l3
or  %o0, 0x60, %o0
ldub  [ %l3 + %o0 ], %g3
sethi  %hi(0x40031800), %o4
ldub  [ %o4 + 0x3d9 ], %o0	! 0x40031bd9 <F>
sethi  %hi(0x40031800), %o5
ldub  [ %o5 + 0x3da ], %o1	! 0x40031bda <A>
and  %o0, 0x3c, %l0
add  %o1, %g3, %o2
and  %o0, 1, %o0
add  %o0, %o2, %o7
mov  %o7, %l3
xor  %o1, %g3, %o2
xor  %o1, %l3, %o1
and  %o1, 0xff, %o1
and  %o2, 0xff, %o2
andn  %o1, %o2, %o2
sethi  %hi(0x40031800), %o0
inc  %o3
or  %o0, 0x3e0, %o0
and  %l3, 0xff, %o1
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o0 + %o1 ], %o1
or  %o4, 0x3d9, %l1
stb  %l0, [ %o4 + 0x3d9 ]
btst  0x80, %o2
be  0x40003b34 <K6502_Step__FUs+6516>
mov  %o5, %g2
or  %o1, 0x40, %o1
cmp  %o7, 0xff
bleu  0x40003b44 <K6502_Step__FUs+6532>
mov  %l0, %o0
or  %o0, 1, %o0
or  %o0, %o1, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %l1 ]
add  %o1, 3, %o1
add  %o0, 3, %o0
b  0x40006a08 <K6502_Step__FUs+18504>
stb  %l3, [ %g2 + 0x3da ]
sethi  %hi(0x40031800), %l1
ldub  [ %l1 + 0x3d9 ], %o0	! 0x40031bd9 <F>
and  %o0, 0x7c, %o7
stb  %o7, [ %l1 + 0x3d9 ]
ld  [ %i0 + 0x3d0 ], %g3
ldub  [ %g3 ], %l3
sethi  %hi(0x40032c00), %g2
or  %g2, 0x60, %g2	! 0x40032c60 <RAM>
mov  %l3, %l2
ldub  [ %g2 + %l2 ], %o1
and  %o0, 1, %o0
sll  %o0, 9, %o0
sethi  %hi(0x40032400), %o2
sll  %o1, 1, %o1
add  %o1, %o0, %o1
or  %o2, 0xe0, %o2
add  %o2, 1, %o0
ldub  [ %o0 + %o1 ], %o3
sethi  %hi(0x40031800), %l0
ldub  [ %o2 + %o1 ], %o5
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l0 + 0x24c ], %o0
stb  %o5, [ %g2 + %l2 ]
or  %o7, %o3, %o7
add  %o4, 5, %o4
add  %o0, 5, %o0
inc  %g3
st  %g3, [ %i0 + 0x3d0 ]
stb  %o7, [ %l1 + 0x3d9 ]
st  %o0, [ %l0 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o4, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %o7
ldub  [ %o7 + 0x3d8 ], %o4	! 0x40031bd8 <SP>
inc  %o4
sethi  %hi(0x40032c00), %o0
or  %o0, 0x60, %o0	! 0x40032c60 <RAM>
or  %o4, 0x100, %o1
ldub  [ %o0 + %o1 ], %g3
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40031800), %o0
or  %o0, 0x3e0, %o0	! 0x40031be0 <g_byTestTable>
mov  %g3, %o2
ldub  [ %g2 + 0x3d9 ], %o1
ldub  [ %o0 + %o2 ], %o3
sethi  %hi(0x40031800), %o5
and  %o1, 0x7d, %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o5 + 0x24c ], %o0
or  %o1, %o3, %o1
add  %o2, 4, %o2
add  %o0, 4, %o0
sethi  %hi(0x40031800), %o3
stb  %o1, [ %g2 + 0x3d9 ]
st  %o0, [ %o5 + 0x24c ]
b  0x40005068 <K6502_Step__FUs+11944>
stb  %o4, [ %o7 + 0x3d8 ]
ld  [ %i0 + 0x3d0 ], %o4
sethi  %hi(0x40031800), %o5
ldub  [ %o4 ], %g3
ldub  [ %o5 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %g2
ldub  [ %g2 + 0x3da ], %o2	! 0x40031bda <A>
and  %o1, 0x3c, %l0
and  %g3, 0xff, %o0
add  %o0, %o2, %o0
and  %o1, 1, %o1
add  %o1, %o0, %o7
mov  %o7, %l3
xor  %o2, %g3, %o3
xor  %o2, %l3, %o2
sethi  %hi(0x40031800), %o0
inc  %o4
and  %o3, 0xff, %o3
and  %o2, 0xff, %o2
or  %o0, 0x3e0, %o0
and  %l3, 0xff, %o1
st  %o4, [ %i0 + 0x3d0 ]
andn  %o2, %o3, %o3
ldub  [ %o0 + %o1 ], %o1
or  %o5, 0x3d9, %l1
btst  0x80, %o3
be  0x40003cbc <K6502_Step__FUs+6908>
stb  %l0, [ %o5 + 0x3d9 ]
or  %o1, 0x40, %o1
cmp  %o7, 0xff
bleu  0x40003ccc <K6502_Step__FUs+6924>
mov  %l0, %o0
or  %o0, 1, %o0
or  %o0, %o1, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %l1 ]
add  %o1, 2, %o1
add  %o0, 2, %o0
b  0x40006a08 <K6502_Step__FUs+18504>
stb  %l3, [ %g2 + 0x3da ]
sethi  %hi(0x40031800), %l0
ldub  [ %l0 + 0x3d9 ], %o4	! 0x40031bd9 <F>
sethi  %hi(0x40031800), %o7
and  %o4, 1, %o0
ldub  [ %o7 + 0x3da ], %o1
sll  %o0, 9, %o0
sethi  %hi(0x40032400), %o2
sll  %o1, 1, %o1
add  %o1, %o0, %o1
or  %o2, 0xe0, %o2
add  %o2, 1, %o0
ldub  [ %o0 + %o1 ], %o5
sethi  %hi(0x40031800), %g3
lduh  [ %l4 + 0x248 ], %o3
ld  [ %g3 + 0x24c ], %o0
and  %o4, 0x7c, %o4
ldub  [ %o2 + %o1 ], %g2
or  %o4, %o5, %o4
add  %o3, 2, %o3
add  %o0, 2, %o0
stb  %o4, [ %l0 + 0x3d9 ]
stb  %g2, [ %o7 + 0x3da ]
st  %o0, [ %g3 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o3, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o0
ldub  [ %o0 ], %l3
inc  %o0
st  %o0, [ %i0 + 0x3d0 ]
ldub  [ %o0 ], %l0
sll  %l0, 8, %l0
or  %l3, %l0, %l0
sll  %l0, 0x10, %o0
srl  %o0, 0x1d, %l3
sethi  %hi(0x40031800), %l2
or  %l2, 0x250, %l2	! 0x40031a50 <readmem_tbl>
sll  %l3, 2, %l3
ld  [ %l2 + %l3 ], %o1
call  %o1
srl  %o0, 0x10, %o0
inc  %l0
mov  %o0, %l1
sll  %l0, 0x10, %l0
ld  [ %l2 + %l3 ], %o1
call  %o1
srl  %l0, 0x10, %o0
and  %o0, 0xff, %o0
sll  %o0, 8, %o0
and  %l1, 0xff, %l1
or  %l1, %o0, %l1
st  %l1, [ %i0 + 0x3d0 ]
or  %i0, 0x3d0, %o0
lduh  [ %o0 + 2 ], %o1
sll  %o1, 0x10, %o1
srl  %o1, 0x1d, %o2
sethi  %hi(0x40033400), %o0
or  %o0, 0x78, %o0	! 0x40033478 <memmap_tbl>
sll  %o2, 2, %o2
ld  [ %o0 + %o2 ], %o5
sethi  %hi(0x40031800), %o4
lduh  [ %l4 + 0x248 ], %o3
ld  [ %o4 + 0x24c ], %o0
srl  %o1, 0x10, %o1
add  %o5, %o1, %o1
add  %o3, 5, %o3
add  %o0, 5, %o0
sethi  %hi(0x40031800), %o2
st  %o1, [ %i0 + 0x3d0 ]
st  %o0, [ %o4 + 0x24c ]
st  %o5, [ %o2 + 0x3d4 ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o3, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %o0
sll  %o0, 8, %o0
or  %l3, %o0, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
sethi  %hi(0x40031800), %o2
inc  %o1
or  %o2, 0x250, %o2
sll  %o3, 2, %o3
ld  [ %o2 + %o3 ], %o4
st  %o1, [ %i0 + 0x3d0 ]
call  %o4
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o4
ldub  [ %o4 + 0x3d9 ], %o1	! 0x40031bd9 <F>
mov  %o0, %g3
sethi  %hi(0x40031800), %o5
ldub  [ %o5 + 0x3da ], %o2	! 0x40031bda <A>
and  %o1, 0x3c, %l0
and  %g3, 0xff, %o0
add  %o0, %o2, %o0
and  %o1, 1, %o1
add  %o1, %o0, %o7
mov  %o7, %l3
xor  %o2, %g3, %o3
xor  %o2, %l3, %o2
sethi  %hi(0x40031800), %o1
and  %o3, 0xff, %o3
and  %o2, 0xff, %o2
or  %o1, 0x3e0, %o1
and  %l3, 0xff, %o0
andn  %o2, %o3, %o3
ldub  [ %o1 + %o0 ], %o1
or  %o4, 0x3d9, %l1
stb  %l0, [ %o4 + 0x3d9 ]
btst  0x80, %o3
be  0x40003ebc <K6502_Step__FUs+7420>
mov  %o5, %g2
or  %o1, 0x40, %o1
cmp  %o7, 0xff
bleu  0x40003ecc <K6502_Step__FUs+7436>
mov  %l0, %o0
or  %o0, 1, %o0
or  %o0, %o1, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %l1 ]
add  %o1, 4, %o1
add  %o0, 4, %o0
b  0x40006a08 <K6502_Step__FUs+18504>
stb  %l3, [ %g2 + 0x3da ]
sethi  %hi(0x40031800), %l2
ldub  [ %l2 + 0x3d9 ], %o2	! 0x40031bd9 <F>
and  %o2, 0x7c, %l0
stb  %l0, [ %l2 + 0x3d9 ]
ld  [ %i0 + 0x3d0 ], %o4
ldub  [ %o4 ], %l3
inc  %o4
st  %o4, [ %i0 + 0x3d0 ]
ldub  [ %o4 ], %g3
sll  %g3, 8, %g3
sethi  %hi(0x40032c00), %o7
or  %l3, %g3, %g3
or  %o7, 0x60, %o7
ldub  [ %o7 + %g3 ], %o1
and  %o2, 1, %o2
sethi  %hi(0x40032400), %g2
or  %g2, 0xe0, %g2	! 0x400324e0 <g_RORTable>
sll  %o2, 9, %o2
sll  %o1, 1, %o1
add  %o1, %o2, %o1
add  %g2, 1, %o0
ldub  [ %o0 + %o1 ], %o2
sethi  %hi(0x40031800), %l1
ldub  [ %g2 + %o1 ], %o0
lduh  [ %l4 + 0x248 ], %o5
ld  [ %l1 + 0x24c ], %o3
stb  %o0, [ %o7 + %g3 ]
or  %l0, %o2, %l0
inc  %o4
add  %o5, 6, %o5
add  %o3, 6, %o3
st  %o4, [ %i0 + 0x3d0 ]
stb  %l0, [ %l2 + 0x3d9 ]
st  %o3, [ %l1 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o5, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %o0
ldub  [ %o0 + 0x3d9 ], %o1	! 0x40031bd9 <F>
btst  0x40, %o1
bne  0x400063a0 <K6502_Step__FUs+16864>
ld  [ %i0 + 0x3d0 ], %o2
sethi  %hi(0x40031800), %o3
ld  [ %i0 + 0x3d0 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o3 + 0x24c ], %o0
inc  %o1
add  %o2, 2, %o2
b  0x400069e4 <K6502_Step__FUs+18468>
add  %o0, 2, %o0
ld  [ %i0 + 0x3d0 ], %g2
ldub  [ %g2 ], %l3
mov  %l3, %o5
sethi  %hi(0x40032c00), %o2
or  %o2, 0x60, %o2	! 0x40032c60 <RAM>
add  %o5, 1, %o0
ldub  [ %o2 + %o0 ], %o3
ldub  [ %o2 + %o5 ], %o1
sll  %o3, 8, %o3
sethi  %hi(0x40031800), %o4
or  %o1, %o3, %o1
ldub  [ %o4 + 0x3dc ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
cmp  %g0, %o1
sll  %o0, 0x10, %o0
sethi  %hi(0x40031800), %l0
addx  %g0, 0, %o5
lduh  [ %l4 + 0x248 ], %o3
ld  [ %l0 + 0x24c ], %o4
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o4, %o5, %o4
add  %o3, %o5, %o3
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %o5
inc  %g2
st  %g2, [ %i0 + 0x3d0 ]
sth  %o3, [ %l4 + 0x248 ]
st  %o4, [ %l0 + 0x24c ]
call  %o5
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o4
ldub  [ %o4 + 0x3d9 ], %o1	! 0x40031bd9 <F>
mov  %o0, %g3
sethi  %hi(0x40031800), %o5
ldub  [ %o5 + 0x3da ], %o2	! 0x40031bda <A>
and  %o1, 0x3c, %l1
and  %g3, 0xff, %o0
add  %o0, %o2, %o0
and  %o1, 1, %o1
add  %o1, %o0, %o7
mov  %o7, %l3
xor  %o2, %g3, %o3
xor  %o2, %l3, %o2
sethi  %hi(0x40031800), %o1
and  %o3, 0xff, %o3
and  %o2, 0xff, %o2
or  %o1, 0x3e0, %o1
and  %l3, 0xff, %o0
andn  %o2, %o3, %o3
ldub  [ %o1 + %o0 ], %o1
or  %o4, 0x3d9, %l2
stb  %l1, [ %o4 + 0x3d9 ]
btst  0x80, %o3
be  0x400040a8 <K6502_Step__FUs+7912>
mov  %o5, %g2
or  %o1, 0x40, %o1
cmp  %o7, 0xff
bleu  0x400040b8 <K6502_Step__FUs+7928>
mov  %l1, %o0
or  %o0, 1, %o0
or  %o0, %o1, %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %l0 + 0x24c ], %o0
stb  %o2, [ %l2 ]
add  %o1, 5, %o1
add  %o0, 5, %o0
stb  %l3, [ %g2 + 0x3da ]
b  0x40006a0c <K6502_Step__FUs+18508>
st  %o0, [ %l0 + 0x24c ]
sethi  %hi(0x40031800), %o0
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o0 + 0x3db ], %o2
ldub  [ %o3 ], %o1
sethi  %hi(0x40032c00), %o0
add  %o2, %o1, %o2
or  %o0, 0x60, %o0
ldub  [ %o0 + %o2 ], %g3
sethi  %hi(0x40031800), %o4
ldub  [ %o4 + 0x3d9 ], %o0	! 0x40031bd9 <F>
sethi  %hi(0x40031800), %o5
ldub  [ %o5 + 0x3da ], %o1	! 0x40031bda <A>
and  %o0, 0x3c, %l0
add  %o1, %g3, %o2
and  %o0, 1, %o0
add  %o0, %o2, %o7
mov  %o7, %l3
xor  %o1, %g3, %o2
xor  %o1, %l3, %o1
and  %o1, 0xff, %o1
and  %o2, 0xff, %o2
andn  %o1, %o2, %o2
sethi  %hi(0x40031800), %o0
inc  %o3
or  %o0, 0x3e0, %o0
and  %l3, 0xff, %o1
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o0 + %o1 ], %o1
or  %o4, 0x3d9, %l1
stb  %l0, [ %o4 + 0x3d9 ]
btst  0x80, %o2
be  0x40004164 <K6502_Step__FUs+8100>
mov  %o5, %g2
or  %o1, 0x40, %o1
cmp  %o7, 0xff
bleu  0x40004174 <K6502_Step__FUs+8116>
mov  %l0, %o0
or  %o0, 1, %o0
or  %o0, %o1, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %l1 ]
add  %o1, 4, %o1
add  %o0, 4, %o0
b  0x40006a08 <K6502_Step__FUs+18504>
stb  %l3, [ %g2 + 0x3da ]
sethi  %hi(0x40031800), %l2
ldub  [ %l2 + 0x3d9 ], %o3	! 0x40031bd9 <F>
and  %o3, 0x7c, %l0
stb  %l0, [ %l2 + 0x3d9 ]
ld  [ %i0 + 0x3d0 ], %o7
sethi  %hi(0x40031800), %o0
ldub  [ %o7 ], %o1
ldub  [ %o0 + 0x3db ], %g3
sethi  %hi(0x40032c00), %g2
add  %g3, %o1, %g3
or  %g2, 0x60, %g2
ldub  [ %g2 + %g3 ], %o1
and  %o3, 1, %o3
sethi  %hi(0x40032400), %o2
or  %o2, 0xe0, %o2	! 0x400324e0 <g_RORTable>
sll  %o3, 9, %o3
sll  %o1, 1, %o1
add  %o1, %o3, %o1
add  %o2, 1, %o0
ldub  [ %o0 + %o1 ], %o3
sethi  %hi(0x40031800), %l1
ldub  [ %o2 + %o1 ], %o5
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l1 + 0x24c ], %o0
stb  %o5, [ %g2 + %g3 ]
or  %l0, %o3, %l0
add  %o4, 6, %o4
add  %o0, 6, %o0
inc  %o7
st  %o7, [ %i0 + 0x3d0 ]
stb  %l0, [ %l2 + 0x3d9 ]
st  %o0, [ %l1 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o4, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %o3
sethi  %hi(0x40031800), %o4
ldub  [ %o3 + 0x3d9 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o4 + 0x24c ], %o0
b  0x40006678 <K6502_Step__FUs+17592>
or  %o1, 4, %o1
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o3 ], %l3
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o3 ], %o1
sll  %o1, 8, %o1
sethi  %hi(0x40031800), %o2
or  %l3, %o1, %o1
ldub  [ %o2 + 0x3dc ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
cmp  %g0, %o1
sll  %o0, 0x10, %o0
sethi  %hi(0x40031800), %l0
addx  %g0, 0, %g2
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l0 + 0x24c ], %o5
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o5, %g2, %o5
add  %o4, %g2, %o4
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %g2
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
sth  %o4, [ %l4 + 0x248 ]
st  %o5, [ %l0 + 0x24c ]
call  %g2
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o4
ldub  [ %o4 + 0x3d9 ], %o1	! 0x40031bd9 <F>
mov  %o0, %g3
sethi  %hi(0x40031800), %o5
ldub  [ %o5 + 0x3da ], %o2	! 0x40031bda <A>
and  %o1, 0x3c, %l1
and  %g3, 0xff, %o0
add  %o0, %o2, %o0
and  %o1, 1, %o1
add  %o1, %o0, %o7
mov  %o7, %l3
xor  %o2, %g3, %o3
xor  %o2, %l3, %o2
sethi  %hi(0x40031800), %o1
and  %o3, 0xff, %o3
and  %o2, 0xff, %o2
or  %o1, 0x3e0, %o1
and  %l3, 0xff, %o0
andn  %o2, %o3, %o3
ldub  [ %o1 + %o0 ], %o1
or  %o4, 0x3d9, %l2
stb  %l1, [ %o4 + 0x3d9 ]
btst  0x80, %o3
be  0x40004324 <K6502_Step__FUs+8548>
mov  %o5, %g2
or  %o1, 0x40, %o1
cmp  %o7, 0xff
bleu  0x40004334 <K6502_Step__FUs+8564>
mov  %l1, %o0
or  %o0, 1, %o0
or  %o0, %o1, %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %l0 + 0x24c ], %o0
stb  %o2, [ %l2 ]
add  %o1, 4, %o1
add  %o0, 4, %o0
stb  %l3, [ %g2 + 0x3da ]
b  0x40006a0c <K6502_Step__FUs+18508>
st  %o0, [ %l0 + 0x24c ]
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o3 ], %l3
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o3 ], %o1
sll  %o1, 8, %o1
sethi  %hi(0x40031800), %o2
or  %l3, %o1, %o1
ldub  [ %o2 + 0x3db ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
cmp  %g0, %o1
sll  %o0, 0x10, %o0
sethi  %hi(0x40031800), %l0
addx  %g0, 0, %g2
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l0 + 0x24c ], %o5
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o5, %g2, %o5
add  %o4, %g2, %o4
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %g2
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
sth  %o4, [ %l4 + 0x248 ]
st  %o5, [ %l0 + 0x24c ]
call  %g2
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o4
ldub  [ %o4 + 0x3d9 ], %o1	! 0x40031bd9 <F>
mov  %o0, %g3
sethi  %hi(0x40031800), %o5
ldub  [ %o5 + 0x3da ], %o2	! 0x40031bda <A>
and  %o1, 0x3c, %l1
and  %g3, 0xff, %o0
add  %o0, %o2, %o0
and  %o1, 1, %o1
add  %o1, %o0, %o7
mov  %o7, %l3
xor  %o2, %g3, %o3
xor  %o2, %l3, %o2
sethi  %hi(0x40031800), %o1
and  %o3, 0xff, %o3
and  %o2, 0xff, %o2
or  %o1, 0x3e0, %o1
and  %l3, 0xff, %o0
andn  %o2, %o3, %o3
ldub  [ %o1 + %o0 ], %o1
or  %o4, 0x3d9, %l2
stb  %l1, [ %o4 + 0x3d9 ]
btst  0x80, %o3
be  0x40004440 <K6502_Step__FUs+8832>
mov  %o5, %g2
or  %o1, 0x40, %o1
cmp  %o7, 0xff
bleu  0x40004450 <K6502_Step__FUs+8848>
mov  %l1, %o0
or  %o0, 1, %o0
or  %o0, %o1, %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %l0 + 0x24c ], %o0
stb  %o2, [ %l2 ]
add  %o1, 4, %o1
add  %o0, 4, %o0
stb  %l3, [ %g2 + 0x3da ]
b  0x40006a0c <K6502_Step__FUs+18508>
st  %o0, [ %l0 + 0x24c ]
sethi  %hi(0x40031800), %l2
ldub  [ %l2 + 0x3d9 ], %o3	! 0x40031bd9 <F>
and  %o3, 0x7c, %l1
stb  %l1, [ %l2 + 0x3d9 ]
ld  [ %i0 + 0x3d0 ], %o5
ldub  [ %o5 ], %l3
inc  %o5
st  %o5, [ %i0 + 0x3d0 ]
ldub  [ %o5 ], %o2
sethi  %hi(0x40031800), %o1
sll  %o2, 8, %o2
ldub  [ %o1 + 0x3db ], %o0
or  %l3, %o2, %o2
add  %o2, %o0, %o2
sll  %o2, 0x10, %o2
sethi  %hi(0x40032c00), %o7
or  %o7, 0x60, %o7	! 0x40032c60 <RAM>
srl  %o2, 0x10, %o2
ldub  [ %o7 + %o2 ], %o1
and  %o3, 1, %o3
sethi  %hi(0x40032400), %g3
or  %g3, 0xe0, %g3	! 0x400324e0 <g_RORTable>
sll  %o3, 9, %o3
sll  %o1, 1, %o1
add  %o1, %o3, %o1
add  %g3, 1, %o0
ldub  [ %o0 + %o1 ], %o3
sethi  %hi(0x40031800), %l0
ldub  [ %g3 + %o1 ], %o0
lduh  [ %l4 + 0x248 ], %g2
ld  [ %l0 + 0x24c ], %o4
stb  %o0, [ %o7 + %o2 ]
or  %l1, %o3, %l1
inc  %o5
add  %g2, 7, %g2
add  %o4, 7, %o4
st  %o5, [ %i0 + 0x3d0 ]
stb  %l1, [ %l2 + 0x3d9 ]
st  %o4, [ %l0 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %g2, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o5
sethi  %hi(0x40031800), %o0
ldub  [ %o5 ], %o1
ldub  [ %o0 + 0x3db ], %o3
add  %o3, %o1, %o3
sethi  %hi(0x40032c00), %o2
or  %o2, 0x60, %o2	! 0x40032c60 <RAM>
add  %o3, 1, %o0
ldub  [ %o2 + %o0 ], %o1
ldub  [ %o2 + %o3 ], %o0
sll  %o1, 8, %o1
or  %o0, %o1, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
or  %o1, 0x310, %o1	! 0x40031b10 <writemem_tbl>
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %o3
inc  %o5
sethi  %hi(0x40031800), %o4
srl  %o0, 0x10, %o0
ldub  [ %o4 + 0x3da ], %o1
call  %o3
st  %o5, [ %i0 + 0x3d0 ]
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
add  %o1, 6, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 6, %o0
ld  [ %i0 + 0x3d0 ], %o3
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40032c00), %o1
sethi  %hi(0x40031800), %o0
ldub  [ %o3 ], %l3
lduh  [ %l4 + 0x248 ], %o4
ld  [ %g2 + 0x24c ], %o2
or  %o1, 0x60, %o1
b  0x40004604 <K6502_Step__FUs+9284>
ldub  [ %o0 + 0x3dc ], %o5
ld  [ %i0 + 0x3d0 ], %o3
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40032c00), %o1
sethi  %hi(0x40031800), %o0
ldub  [ %o3 ], %l3
lduh  [ %l4 + 0x248 ], %o4
ld  [ %g2 + 0x24c ], %o2
or  %o1, 0x60, %o1
b  0x40004604 <K6502_Step__FUs+9284>
ldub  [ %o0 + 0x3da ], %o5
ld  [ %i0 + 0x3d0 ], %o3
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40032c00), %o1
sethi  %hi(0x40031800), %o0
ldub  [ %o3 ], %l3
lduh  [ %l4 + 0x248 ], %o4
ld  [ %g2 + 0x24c ], %o2
or  %o1, 0x60, %o1
ldub  [ %o0 + 0x3db ], %o5
stb  %o5, [ %l3 + %o1 ]
add  %o4, 3, %o4
add  %o2, 3, %o2
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
st  %o2, [ %g2 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o4, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3dc ], %o4	! 0x40031bdc <Y>
b  0x40005660 <K6502_Step__FUs+13472>
add  %o4, -1, %o4
sethi  %hi(0x40031800), %o0
b  0x400048e0 <K6502_Step__FUs+10016>
ldub  [ %o0 + 0x3db ], %g2	! 0x40031bdb <X>
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %o0
sll  %o0, 8, %o0
or  %l3, %o0, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
inc  %o1
sethi  %hi(0x40031800), %o2
or  %o2, 0x310, %o2	! 0x40031b10 <writemem_tbl>
sll  %o3, 2, %o3
sethi  %hi(0x40031800), %o5
st  %o1, [ %i0 + 0x3d0 ]
ld  [ %o2 + %o3 ], %o4
srl  %o0, 0x10, %o0
call  %o4
ldub  [ %o5 + 0x3dc ], %o1
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
add  %o1, 4, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 4, %o0
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %o0
sll  %o0, 8, %o0
or  %l3, %o0, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
inc  %o1
sethi  %hi(0x40031800), %o2
or  %o2, 0x310, %o2	! 0x40031b10 <writemem_tbl>
sll  %o3, 2, %o3
sethi  %hi(0x40031800), %o5
st  %o1, [ %i0 + 0x3d0 ]
ld  [ %o2 + %o3 ], %o4
srl  %o0, 0x10, %o0
call  %o4
ldub  [ %o5 + 0x3da ], %o1
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
add  %o1, 4, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 4, %o0
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %o0
sll  %o0, 8, %o0
or  %l3, %o0, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
inc  %o1
sethi  %hi(0x40031800), %o2
or  %o2, 0x310, %o2	! 0x40031b10 <writemem_tbl>
sll  %o3, 2, %o3
sethi  %hi(0x40031800), %o5
st  %o1, [ %i0 + 0x3d0 ]
ld  [ %o2 + %o3 ], %o4
srl  %o0, 0x10, %o0
call  %o4
ldub  [ %o5 + 0x3db ], %o1
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
add  %o1, 4, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 4, %o0
sethi  %hi(0x40031800), %o1
ldub  [ %o1 + 0x3d9 ], %o0	! 0x40031bd9 <F>
xor  %o0, 1, %o0
btst  1, %o0
bne  0x400063a0 <K6502_Step__FUs+16864>
ld  [ %i0 + 0x3d0 ], %o2
sethi  %hi(0x40031800), %o3
ld  [ %i0 + 0x3d0 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o3 + 0x24c ], %o0
inc  %o1
add  %o2, 2, %o2
b  0x400069e4 <K6502_Step__FUs+18468>
add  %o0, 2, %o0
ld  [ %i0 + 0x3d0 ], %o5
ldub  [ %o5 ], %l3
mov  %l3, %o4
sethi  %hi(0x40032c00), %o1
or  %o1, 0x60, %o1	! 0x40032c60 <RAM>
add  %o4, 1, %o0
ldub  [ %o1 + %o0 ], %o2
ldub  [ %o1 + %o4 ], %o0
sll  %o2, 8, %o2
sethi  %hi(0x40031800), %o3
ldub  [ %o3 + 0x3dc ], %o1	! 0x40031bdc <Y>
or  %o0, %o2, %o0
add  %o0, %o1, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
or  %o1, 0x310, %o1	! 0x40031b10 <writemem_tbl>
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %o3
inc  %o5
sethi  %hi(0x40031800), %o4
srl  %o0, 0x10, %o0
ldub  [ %o4 + 0x3da ], %o1
call  %o3
st  %o5, [ %i0 + 0x3d0 ]
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
add  %o1, 6, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 6, %o0
ld  [ %i0 + 0x3d0 ], %o5
sethi  %hi(0x40031800), %o0
ldub  [ %o5 ], %o4
ldub  [ %o0 + 0x3db ], %o3
sethi  %hi(0x40031800), %g3
add  %o3, %o4, %o3
sethi  %hi(0x40032c00), %o0
sethi  %hi(0x40031800), %o1
lduh  [ %l4 + 0x248 ], %g2
ld  [ %g3 + 0x24c ], %o2
or  %o0, 0x60, %o0
b  0x400048b8 <K6502_Step__FUs+9976>
ldub  [ %o1 + 0x3dc ], %o4
ld  [ %i0 + 0x3d0 ], %o5
sethi  %hi(0x40031800), %o0
ldub  [ %o5 ], %o4
ldub  [ %o0 + 0x3db ], %o3
sethi  %hi(0x40031800), %g3
add  %o3, %o4, %o3
sethi  %hi(0x40032c00), %o0
sethi  %hi(0x40031800), %o1
lduh  [ %l4 + 0x248 ], %g2
ld  [ %g3 + 0x24c ], %o2
or  %o0, 0x60, %o0
b  0x400048b8 <K6502_Step__FUs+9976>
ldub  [ %o1 + 0x3da ], %o4
ld  [ %i0 + 0x3d0 ], %o5
sethi  %hi(0x40031800), %o0
ldub  [ %o5 ], %o4
ldub  [ %o0 + 0x3dc ], %o3
sethi  %hi(0x40031800), %g3
add  %o3, %o4, %o3
sethi  %hi(0x40032c00), %o0
sethi  %hi(0x40031800), %o1
lduh  [ %l4 + 0x248 ], %g2
ld  [ %g3 + 0x24c ], %o2
or  %o0, 0x60, %o0
ldub  [ %o1 + 0x3db ], %o4
stb  %o4, [ %o0 + %o3 ]
add  %g2, 4, %g2
add  %o2, 4, %o2
inc  %o5
st  %o5, [ %i0 + 0x3d0 ]
st  %o2, [ %g3 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %g2, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %o0
ldub  [ %o0 + 0x3dc ], %g2	! 0x40031bdc <Y>
sethi  %hi(0x40031800), %o5
sethi  %hi(0x40031800), %o0
or  %o0, 0x3e0, %o0	! 0x40031be0 <g_byTestTable>
mov  %g2, %o2
ldub  [ %o5 + 0x3d9 ], %o1
ldub  [ %o0 + %o2 ], %o3
sethi  %hi(0x40031800), %o4
and  %o1, 0x7d, %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o4 + 0x24c ], %o0
or  %o1, %o3, %o1
add  %o2, 2, %o2
add  %o0, 2, %o0
sethi  %hi(0x40031800), %o3
stb  %o1, [ %o5 + 0x3d9 ]
st  %o0, [ %o4 + 0x24c ]
stb  %g2, [ %o3 + 0x3da ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o2
ldub  [ %o2 ], %l3
inc  %o2
st  %o2, [ %i0 + 0x3d0 ]
ldub  [ %o2 ], %o0
sll  %o0, 8, %o0
sethi  %hi(0x40031800), %o3
ldub  [ %o3 + 0x3dc ], %o1	! 0x40031bdc <Y>
or  %l3, %o0, %o0
add  %o0, %o1, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
sethi  %hi(0x40031800), %o1
or  %o1, 0x310, %o1	! 0x40031b10 <writemem_tbl>
sll  %o3, 2, %o3
inc  %o2
sethi  %hi(0x40031800), %o5
ld  [ %o1 + %o3 ], %o4
st  %o2, [ %i0 + 0x3d0 ]
srl  %o0, 0x10, %o0
call  %o4
ldub  [ %o5 + 0x3da ], %o1
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
add  %o1, 5, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 5, %o0
sethi  %hi(0x40031800), %o4
sethi  %hi(0x40031800), %o0
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o4 + 0x24c ], %o1
ldub  [ %o0 + 0x3db ], %o3
add  %o2, 2, %o2
add  %o1, 2, %o1
sethi  %hi(0x40031800), %o0
stb  %o3, [ %o0 + 0x3d8 ]	! 0x40031bd8 <SP>
st  %o1, [ %o4 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o2
ldub  [ %o2 ], %l3
inc  %o2
st  %o2, [ %i0 + 0x3d0 ]
ldub  [ %o2 ], %o0
sll  %o0, 8, %o0
sethi  %hi(0x40031800), %o3
ldub  [ %o3 + 0x3db ], %o1	! 0x40031bdb <X>
or  %l3, %o0, %o0
add  %o0, %o1, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
sethi  %hi(0x40031800), %o1
or  %o1, 0x310, %o1	! 0x40031b10 <writemem_tbl>
sll  %o3, 2, %o3
inc  %o2
sethi  %hi(0x40031800), %o5
ld  [ %o1 + %o3 ], %o4
st  %o2, [ %i0 + 0x3d0 ]
srl  %o0, 0x10, %o0
call  %o4
ldub  [ %o5 + 0x3da ], %o1
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
add  %o1, 5, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 5, %o0
ld  [ %i0 + 0x3d0 ], %o4
ldub  [ %o4 ], %g3
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40031800), %o0
or  %o0, 0x3e0, %o0	! 0x40031be0 <g_byTestTable>
mov  %g3, %o2
ldub  [ %g2 + 0x3d9 ], %o1
ldub  [ %o0 + %o2 ], %o3
sethi  %hi(0x40031800), %o5
and  %o1, 0x7d, %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o5 + 0x24c ], %o0
or  %o1, %o3, %o1
add  %o2, 2, %o2
add  %o0, 2, %o0
sethi  %hi(0x40031800), %o3
b  0x40004fe8 <K6502_Step__FUs+11816>
inc  %o4
ld  [ %i0 + 0x3d0 ], %o4
sethi  %hi(0x40031800), %o0
ldub  [ %o4 ], %o1
ldub  [ %o0 + 0x3db ], %o3
add  %o3, %o1, %o3
sethi  %hi(0x40032c00), %o2
or  %o2, 0x60, %o2	! 0x40032c60 <RAM>
add  %o3, 1, %o0
ldub  [ %o2 + %o0 ], %o1
ldub  [ %o2 + %o3 ], %o0
sll  %o1, 8, %o1
or  %o0, %o1, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
or  %o1, 0x250, %o1	! 0x40031a50 <readmem_tbl>
sll  %o2, 2, %o2
inc  %o4
ld  [ %o1 + %o2 ], %o3
st  %o4, [ %i0 + 0x3d0 ]
call  %o3
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40031800), %o1
or  %o1, 0x3e0, %o1	! 0x40031be0 <g_byTestTable>
and  %o0, 0xff, %o3
ldub  [ %g2 + 0x3d9 ], %o2
ldub  [ %o1 + %o3 ], %o4
sethi  %hi(0x40031800), %o5
and  %o2, 0x7d, %o2
lduh  [ %l4 + 0x248 ], %o3
ld  [ %o5 + 0x24c ], %o1
or  %o2, %o4, %o2
add  %o3, 6, %o3
b  0x40004df0 <K6502_Step__FUs+11312>
add  %o1, 6, %o1
ld  [ %i0 + 0x3d0 ], %o4
ldub  [ %o4 ], %g3
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40031800), %o0
or  %o0, 0x3e0, %o0	! 0x40031be0 <g_byTestTable>
mov  %g3, %o2
ldub  [ %g2 + 0x3d9 ], %o1
ldub  [ %o0 + %o2 ], %o3
sethi  %hi(0x40031800), %o5
and  %o1, 0x7d, %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o5 + 0x24c ], %o0
or  %o1, %o3, %o1
add  %o2, 2, %o2
add  %o0, 2, %o0
sethi  %hi(0x40031800), %o3
b  0x400050d0 <K6502_Step__FUs+12048>
inc  %o4
ld  [ %i0 + 0x3d0 ], %o4
sethi  %hi(0x40032c00), %o0
ldub  [ %o4 ], %l3
or  %o0, 0x60, %o0
ldub  [ %l3 + %o0 ], %g3
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40031800), %o0
or  %o0, 0x3e0, %o0	! 0x40031be0 <g_byTestTable>
mov  %g3, %o2
ldub  [ %g2 + 0x3d9 ], %o1
ldub  [ %o0 + %o2 ], %o3
sethi  %hi(0x40031800), %o5
and  %o1, 0x7d, %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o5 + 0x24c ], %o0
or  %o1, %o3, %o1
add  %o2, 3, %o2
b  0x40004fe0 <K6502_Step__FUs+11808>
add  %o0, 3, %o0
ld  [ %i0 + 0x3d0 ], %o4
sethi  %hi(0x40032c00), %o0
ldub  [ %o4 ], %l3
or  %o0, 0x60, %o0
ldub  [ %l3 + %o0 ], %g3
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40031800), %o0
or  %o0, 0x3e0, %o0	! 0x40031be0 <g_byTestTable>
mov  %g3, %o2
ldub  [ %g2 + 0x3d9 ], %o1
ldub  [ %o0 + %o2 ], %o3
sethi  %hi(0x40031800), %o5
and  %o1, 0x7d, %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o5 + 0x24c ], %o0
or  %o1, %o3, %o1
add  %o2, 3, %o2
b  0x40005054 <K6502_Step__FUs+11924>
add  %o0, 3, %o0
ld  [ %i0 + 0x3d0 ], %o4
sethi  %hi(0x40032c00), %o0
ldub  [ %o4 ], %l3
or  %o0, 0x60, %o0
ldub  [ %l3 + %o0 ], %g3
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40031800), %o0
or  %o0, 0x3e0, %o0	! 0x40031be0 <g_byTestTable>
mov  %g3, %o2
ldub  [ %g2 + 0x3d9 ], %o1
ldub  [ %o0 + %o2 ], %o3
sethi  %hi(0x40031800), %o5
and  %o1, 0x7d, %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o5 + 0x24c ], %o0
or  %o1, %o3, %o1
add  %o2, 3, %o2
b  0x400050c8 <K6502_Step__FUs+12040>
add  %o0, 3, %o0
sethi  %hi(0x40031800), %o0
ldub  [ %o0 + 0x3da ], %g2	! 0x40031bda <A>
sethi  %hi(0x40031800), %o5
sethi  %hi(0x40031800), %o0
or  %o0, 0x3e0, %o0	! 0x40031be0 <g_byTestTable>
mov  %g2, %o2
ldub  [ %o5 + 0x3d9 ], %o1
ldub  [ %o0 + %o2 ], %o3
sethi  %hi(0x40031800), %o4
and  %o1, 0x7d, %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o4 + 0x24c ], %o0
or  %o1, %o3, %o1
add  %o2, 2, %o2
add  %o0, 2, %o0
sethi  %hi(0x40031800), %o3
stb  %o1, [ %o5 + 0x3d9 ]
st  %o0, [ %o4 + 0x24c ]
stb  %g2, [ %o3 + 0x3dc ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o4
ldub  [ %o4 ], %g3
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40031800), %o0
or  %o0, 0x3e0, %o0	! 0x40031be0 <g_byTestTable>
mov  %g3, %o2
ldub  [ %g2 + 0x3d9 ], %o1
ldub  [ %o0 + %o2 ], %o3
sethi  %hi(0x40031800), %o5
and  %o1, 0x7d, %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o5 + 0x24c ], %o0
or  %o1, %o3, %o1
add  %o2, 2, %o2
add  %o0, 2, %o0
sethi  %hi(0x40031800), %o3
b  0x4000505c <K6502_Step__FUs+11932>
inc  %o4
sethi  %hi(0x40031800), %o0
b  0x40005134 <K6502_Step__FUs+12148>
ldub  [ %o0 + 0x3da ], %g2	! 0x40031bda <A>
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %o0
sll  %o0, 8, %o0
or  %l3, %o0, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
sethi  %hi(0x40031800), %o2
inc  %o1
or  %o2, 0x250, %o2
sll  %o3, 2, %o3
ld  [ %o2 + %o3 ], %o4
st  %o1, [ %i0 + 0x3d0 ]
call  %o4
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40031800), %o1
or  %o1, 0x3e0, %o1	! 0x40031be0 <g_byTestTable>
and  %o0, 0xff, %o3
ldub  [ %g2 + 0x3d9 ], %o2
ldub  [ %o1 + %o3 ], %o4
sethi  %hi(0x40031800), %o5
and  %o2, 0x7d, %o2
lduh  [ %l4 + 0x248 ], %o3
ld  [ %o5 + 0x24c ], %o1
or  %o2, %o4, %o2
add  %o3, 4, %o3
add  %o1, 4, %o1
sethi  %hi(0x40031800), %o4
stb  %o2, [ %g2 + 0x3d9 ]
st  %o1, [ %o5 + 0x24c ]
stb  %o0, [ %o4 + 0x3dc ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o3, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %o0
sll  %o0, 8, %o0
or  %l3, %o0, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
sethi  %hi(0x40031800), %o2
inc  %o1
or  %o2, 0x250, %o2
sll  %o3, 2, %o3
ld  [ %o2 + %o3 ], %o4
st  %o1, [ %i0 + 0x3d0 ]
call  %o4
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40031800), %o1
or  %o1, 0x3e0, %o1	! 0x40031be0 <g_byTestTable>
and  %o0, 0xff, %o3
ldub  [ %g2 + 0x3d9 ], %o2
ldub  [ %o1 + %o3 ], %o4
sethi  %hi(0x40031800), %o5
and  %o2, 0x7d, %o2
lduh  [ %l4 + 0x248 ], %o3
ld  [ %o5 + 0x24c ], %o1
or  %o2, %o4, %o2
add  %o3, 4, %o3
add  %o1, 4, %o1
sethi  %hi(0x40031800), %o4
stb  %o2, [ %g2 + 0x3d9 ]
st  %o1, [ %o5 + 0x24c ]
stb  %o0, [ %o4 + 0x3da ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o3, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %o0
sll  %o0, 8, %o0
or  %l3, %o0, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
sethi  %hi(0x40031800), %o2
inc  %o1
or  %o2, 0x250, %o2
sll  %o3, 2, %o3
ld  [ %o2 + %o3 ], %o4
st  %o1, [ %i0 + 0x3d0 ]
call  %o4
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40031800), %o1
or  %o1, 0x3e0, %o1	! 0x40031be0 <g_byTestTable>
and  %o0, 0xff, %o3
ldub  [ %g2 + 0x3d9 ], %o2
ldub  [ %o1 + %o3 ], %o4
sethi  %hi(0x40031800), %o5
and  %o2, 0x7d, %o2
lduh  [ %l4 + 0x248 ], %o3
ld  [ %o5 + 0x24c ], %o1
or  %o2, %o4, %o2
add  %o3, 4, %o3
add  %o1, 4, %o1
sethi  %hi(0x40031800), %o4
stb  %o2, [ %g2 + 0x3d9 ]
st  %o1, [ %o5 + 0x24c ]
stb  %o0, [ %o4 + 0x3db ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o3, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %o0
ldub  [ %o0 + 0x3d9 ], %o1	! 0x40031bd9 <F>
btst  1, %o1
bne  0x400063a0 <K6502_Step__FUs+16864>
ld  [ %i0 + 0x3d0 ], %o2
sethi  %hi(0x40031800), %o3
ld  [ %i0 + 0x3d0 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o3 + 0x24c ], %o0
inc  %o1
add  %o2, 2, %o2
b  0x400069e4 <K6502_Step__FUs+18468>
add  %o0, 2, %o0
ld  [ %i0 + 0x3d0 ], %g2
ldub  [ %g2 ], %l3
mov  %l3, %o5
sethi  %hi(0x40032c00), %o2
or  %o2, 0x60, %o2	! 0x40032c60 <RAM>
add  %o5, 1, %o0
ldub  [ %o2 + %o0 ], %o3
ldub  [ %o2 + %o5 ], %o1
sll  %o3, 8, %o3
sethi  %hi(0x40031800), %o4
or  %o1, %o3, %o1
ldub  [ %o4 + 0x3dc ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
sethi  %hi(0x40031800), %l0
sll  %o0, 0x10, %o0
cmp  %g0, %o1
addx  %g0, 0, %o5
lduh  [ %l4 + 0x248 ], %o3
ld  [ %l0 + 0x24c ], %o4
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o4, %o5, %o4
add  %o3, %o5, %o3
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %o5
inc  %g2
sth  %o3, [ %l4 + 0x248 ]
st  %o4, [ %l0 + 0x24c ]
st  %g2, [ %i0 + 0x3d0 ]
call  %o5
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o5
sethi  %hi(0x40031800), %o1
or  %o1, 0x3e0, %o1	! 0x40031be0 <g_byTestTable>
and  %o0, 0xff, %o2
ldub  [ %o5 + 0x3d9 ], %o3
ldub  [ %o1 + %o2 ], %o4
and  %o3, 0x7d, %o3
lduh  [ %l4 + 0x248 ], %o2
ld  [ %l0 + 0x24c ], %o1
or  %o3, %o4, %o3
add  %o2, 5, %o2
b  0x400052f8 <K6502_Step__FUs+12600>
add  %o1, 5, %o1
sethi  %hi(0x40031800), %o0
ld  [ %i0 + 0x3d0 ], %o4
ldub  [ %o0 + 0x3db ], %o1
ldub  [ %o4 ], %o2
sethi  %hi(0x40032c00), %o0
add  %o1, %o2, %o1
or  %o0, 0x60, %o0
ldub  [ %o0 + %o1 ], %g3
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40031800), %o0
or  %o0, 0x3e0, %o0	! 0x40031be0 <g_byTestTable>
mov  %g3, %o2
ldub  [ %g2 + 0x3d9 ], %o1
ldub  [ %o0 + %o2 ], %o3
sethi  %hi(0x40031800), %o5
and  %o1, 0x7d, %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o5 + 0x24c ], %o0
or  %o1, %o3, %o1
add  %o2, 4, %o2
add  %o0, 4, %o0
inc  %o4
sethi  %hi(0x40031800), %o3
st  %o4, [ %i0 + 0x3d0 ]
stb  %o1, [ %g2 + 0x3d9 ]
st  %o0, [ %o5 + 0x24c ]
stb  %g3, [ %o3 + 0x3dc ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %o0
ld  [ %i0 + 0x3d0 ], %o4
ldub  [ %o0 + 0x3db ], %o1
ldub  [ %o4 ], %o2
sethi  %hi(0x40032c00), %o0
add  %o1, %o2, %o1
or  %o0, 0x60, %o0
ldub  [ %o0 + %o1 ], %g3
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40031800), %o0
or  %o0, 0x3e0, %o0	! 0x40031be0 <g_byTestTable>
mov  %g3, %o2
ldub  [ %g2 + 0x3d9 ], %o1
ldub  [ %o0 + %o2 ], %o3
sethi  %hi(0x40031800), %o5
and  %o1, 0x7d, %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o5 + 0x24c ], %o0
or  %o1, %o3, %o1
add  %o2, 4, %o2
add  %o0, 4, %o0
inc  %o4
sethi  %hi(0x40031800), %o3
st  %o4, [ %i0 + 0x3d0 ]
stb  %o1, [ %g2 + 0x3d9 ]
st  %o0, [ %o5 + 0x24c ]
stb  %g3, [ %o3 + 0x3da ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %o0
ld  [ %i0 + 0x3d0 ], %o4
ldub  [ %o0 + 0x3dc ], %o1
ldub  [ %o4 ], %o2
sethi  %hi(0x40032c00), %o0
add  %o1, %o2, %o1
or  %o0, 0x60, %o0
ldub  [ %o0 + %o1 ], %g3
sethi  %hi(0x40031800), %g2
sethi  %hi(0x40031800), %o0
or  %o0, 0x3e0, %o0	! 0x40031be0 <g_byTestTable>
mov  %g3, %o2
ldub  [ %g2 + 0x3d9 ], %o1
ldub  [ %o0 + %o2 ], %o3
sethi  %hi(0x40031800), %o5
and  %o1, 0x7d, %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o5 + 0x24c ], %o0
or  %o1, %o3, %o1
add  %o2, 4, %o2
add  %o0, 4, %o0
inc  %o4
sethi  %hi(0x40031800), %o3
st  %o4, [ %i0 + 0x3d0 ]
stb  %o1, [ %g2 + 0x3d9 ]
st  %o0, [ %o5 + 0x24c ]
stb  %g3, [ %o3 + 0x3db ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %o3
sethi  %hi(0x40031800), %o4
ldub  [ %o3 + 0x3d9 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o4 + 0x24c ], %o0
b  0x40006678 <K6502_Step__FUs+17592>
and  %o1, 0xbf, %o1
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o3 ], %l3
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o3 ], %o1
sll  %o1, 8, %o1
sethi  %hi(0x40031800), %o2
or  %l3, %o1, %o1
b  0x4000526c <K6502_Step__FUs+12460>
ldub  [ %o2 + 0x3dc ], %o0
sethi  %hi(0x40031800), %o0
ldub  [ %o0 + 0x3d8 ], %g2	! 0x40031bd8 <SP>
sethi  %hi(0x40031800), %o5
sethi  %hi(0x40031800), %o0
or  %o0, 0x3e0, %o0	! 0x40031be0 <g_byTestTable>
mov  %g2, %o2
ldub  [ %o5 + 0x3d9 ], %o1
ldub  [ %o0 + %o2 ], %o3
sethi  %hi(0x40031800), %o4
and  %o1, 0x7d, %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o4 + 0x24c ], %o0
or  %o1, %o3, %o1
add  %o2, 2, %o2
add  %o0, 2, %o0
sethi  %hi(0x40031800), %o3
stb  %o1, [ %o5 + 0x3d9 ]
st  %o0, [ %o4 + 0x24c ]
stb  %g2, [ %o3 + 0x3db ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o3 ], %l3
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o3 ], %o1
sll  %o1, 8, %o1
sethi  %hi(0x40031800), %o2
or  %l3, %o1, %o1
ldub  [ %o2 + 0x3db ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
sethi  %hi(0x40031800), %l0
sll  %o0, 0x10, %o0
cmp  %g0, %o1
addx  %g0, 0, %g2
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l0 + 0x24c ], %o5
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o5, %g2, %o5
add  %o4, %g2, %o4
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %g2
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
sth  %o4, [ %l4 + 0x248 ]
st  %o5, [ %l0 + 0x24c ]
call  %g2
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o5
sethi  %hi(0x40031800), %o1
or  %o1, 0x3e0, %o1	! 0x40031be0 <g_byTestTable>
and  %o0, 0xff, %o2
ldub  [ %o5 + 0x3d9 ], %o3
ldub  [ %o1 + %o2 ], %o4
and  %o3, 0x7d, %o3
lduh  [ %l4 + 0x248 ], %o2
ld  [ %l0 + 0x24c ], %o1
or  %o3, %o4, %o3
add  %o2, 4, %o2
add  %o1, 4, %o1
sethi  %hi(0x40031800), %o4
stb  %o3, [ %o5 + 0x3d9 ]
st  %o1, [ %l0 + 0x24c ]
stb  %o0, [ %o4 + 0x3dc ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o3 ], %l3
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o3 ], %o1
sll  %o1, 8, %o1
sethi  %hi(0x40031800), %o2
or  %l3, %o1, %o1
ldub  [ %o2 + 0x3db ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
sethi  %hi(0x40031800), %l0
sll  %o0, 0x10, %o0
cmp  %g0, %o1
addx  %g0, 0, %g2
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l0 + 0x24c ], %o5
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o5, %g2, %o5
add  %o4, %g2, %o4
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %g2
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
sth  %o4, [ %l4 + 0x248 ]
st  %o5, [ %l0 + 0x24c ]
call  %g2
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o5
sethi  %hi(0x40031800), %o1
or  %o1, 0x3e0, %o1	! 0x40031be0 <g_byTestTable>
and  %o0, 0xff, %o2
ldub  [ %o5 + 0x3d9 ], %o3
ldub  [ %o1 + %o2 ], %o4
and  %o3, 0x7d, %o3
lduh  [ %l4 + 0x248 ], %o2
ld  [ %l0 + 0x24c ], %o1
or  %o3, %o4, %o3
add  %o2, 4, %o2
add  %o1, 4, %o1
sethi  %hi(0x40031800), %o4
stb  %o3, [ %o5 + 0x3d9 ]
st  %o1, [ %l0 + 0x24c ]
stb  %o0, [ %o4 + 0x3da ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o3 ], %l3
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o3 ], %o1
sll  %o1, 8, %o1
sethi  %hi(0x40031800), %o2
or  %l3, %o1, %o1
ldub  [ %o2 + 0x3dc ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
sethi  %hi(0x40031800), %l0
sll  %o0, 0x10, %o0
cmp  %g0, %o1
addx  %g0, 0, %g2
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l0 + 0x24c ], %o5
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o5, %g2, %o5
add  %o4, %g2, %o4
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %g2
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
sth  %o4, [ %l4 + 0x248 ]
st  %o5, [ %l0 + 0x24c ]
call  %g2
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o5
sethi  %hi(0x40031800), %o1
or  %o1, 0x3e0, %o1	! 0x40031be0 <g_byTestTable>
and  %o0, 0xff, %o2
ldub  [ %o5 + 0x3d9 ], %o3
ldub  [ %o1 + %o2 ], %o4
and  %o3, 0x7d, %o3
lduh  [ %l4 + 0x248 ], %o2
ld  [ %l0 + 0x24c ], %o1
or  %o3, %o4, %o3
add  %o2, 4, %o2
add  %o1, 4, %o1
sethi  %hi(0x40031800), %o4
stb  %o3, [ %o5 + 0x3d9 ]
st  %o1, [ %l0 + 0x24c ]
stb  %o0, [ %o4 + 0x3db ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o5
sethi  %hi(0x40031800), %o0
ldub  [ %o5 ], %o1
ldub  [ %o0 + 0x3dc ], %o2
sub  %o2, %o1, %o2
sethi  %hi(0x40031800), %o4
sll  %o2, 0x10, %o3
sethi  %hi(0x40031800), %o0
ldub  [ %o4 + 0x3d9 ], %o1
or  %o0, 0x3e0, %o0
and  %o2, 0xff, %o2
and  %o1, 0x7c, %o1
ldub  [ %o0 + %o2 ], %o2
srl  %o3, 0x10, %o3
inc  %o5
or  %o4, 0x3d9, %g3
mov  %o1, %g2
st  %o5, [ %i0 + 0x3d0 ]
cmp  %o3, 0xff
bgu  0x40005438 <K6502_Step__FUs+12920>
stb  %o1, [ %o4 + 0x3d9 ]
or  %g2, 1, %o0
b  0x4000543c <K6502_Step__FUs+12924>
or  %o0, %o2, %o3
or  %g2, %o2, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
b  0x40006a00 <K6502_Step__FUs+18496>
stb  %o3, [ %g3 ]
ld  [ %i0 + 0x3d0 ], %o4
sethi  %hi(0x40031800), %o0
ldub  [ %o4 ], %o1
ldub  [ %o0 + 0x3db ], %o3
add  %o3, %o1, %o3
sethi  %hi(0x40032c00), %o2
or  %o2, 0x60, %o2	! 0x40032c60 <RAM>
add  %o3, 1, %o0
ldub  [ %o2 + %o0 ], %o1
ldub  [ %o2 + %o3 ], %o0
sll  %o1, 8, %o1
or  %o0, %o1, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
or  %o1, 0x250, %o1	! 0x40031a50 <readmem_tbl>
sll  %o2, 2, %o2
inc  %o4
ld  [ %o1 + %o2 ], %o3
st  %o4, [ %i0 + 0x3d0 ]
call  %o3
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o1
ldub  [ %o1 + 0x3da ], %o2	! 0x40031bda <A>
and  %o0, 0xff, %o0
sub  %o2, %o0, %o2
sethi  %hi(0x40031800), %o4
sll  %o2, 0x10, %o3
sethi  %hi(0x40031800), %o0
ldub  [ %o4 + 0x3d9 ], %o1
or  %o0, 0x3e0, %o0
and  %o2, 0xff, %o2
and  %o1, 0x7c, %o1
srl  %o3, 0x10, %o3
ldub  [ %o0 + %o2 ], %o2
or  %o4, 0x3d9, %g2
mov  %o1, %o5
cmp  %o3, 0xff
bgu  0x400054fc <K6502_Step__FUs+13116>
stb  %o1, [ %o4 + 0x3d9 ]
or  %o5, 1, %o0
b  0x40005500 <K6502_Step__FUs+13120>
or  %o0, %o2, %o3
or  %o5, %o2, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %g2 ]
add  %o1, 6, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 6, %o0
ld  [ %i0 + 0x3d0 ], %o5
sethi  %hi(0x40032c00), %o0
ldub  [ %o5 ], %l3
or  %o0, 0x60, %o0
sethi  %hi(0x40031800), %o1
ldub  [ %l3 + %o0 ], %o2
ldub  [ %o1 + 0x3dc ], %o3
sub  %o3, %o2, %o3
sethi  %hi(0x40031800), %o4
sll  %o3, 0x10, %o2
sethi  %hi(0x40031800), %o0
ldub  [ %o4 + 0x3d9 ], %o1
or  %o0, 0x3e0, %o0
and  %o3, 0xff, %o3
and  %o1, 0x7c, %o1
ldub  [ %o0 + %o3 ], %g2
srl  %o2, 0x10, %o2
inc  %o5
or  %o4, 0x3d9, %o7
mov  %o1, %g3
st  %o5, [ %i0 + 0x3d0 ]
cmp  %o2, 0xff
bgu  0x40005588 <K6502_Step__FUs+13256>
stb  %o1, [ %o4 + 0x3d9 ]
or  %g3, 1, %o0
b  0x4000558c <K6502_Step__FUs+13260>
or  %o0, %g2, %g2
or  %g3, %g2, %g2
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %g2, [ %o7 ]
add  %o1, 3, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 3, %o0
ld  [ %i0 + 0x3d0 ], %o5
sethi  %hi(0x40032c00), %o0
ldub  [ %o5 ], %l3
or  %o0, 0x60, %o0
sethi  %hi(0x40031800), %o1
ldub  [ %l3 + %o0 ], %o2
ldub  [ %o1 + 0x3da ], %o3
sub  %o3, %o2, %o3
sethi  %hi(0x40031800), %o4
sll  %o3, 0x10, %o2
sethi  %hi(0x40031800), %o0
ldub  [ %o4 + 0x3d9 ], %o1
or  %o0, 0x3e0, %o0
and  %o3, 0xff, %o3
and  %o1, 0x7c, %o1
ldub  [ %o0 + %o3 ], %g2
srl  %o2, 0x10, %o2
inc  %o5
or  %o4, 0x3d9, %o7
mov  %o1, %g3
st  %o5, [ %i0 + 0x3d0 ]
cmp  %o2, 0xff
bgu  0x40005614 <K6502_Step__FUs+13396>
stb  %o1, [ %o4 + 0x3d9 ]
or  %g3, 1, %o0
b  0x40005618 <K6502_Step__FUs+13400>
or  %o0, %g2, %g2
or  %g3, %g2, %g2
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %g2, [ %o7 ]
add  %o1, 3, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 3, %o0
ld  [ %i0 + 0x3d0 ], %o5
ldub  [ %o5 ], %l3
sethi  %hi(0x40032c00), %o4
or  %o4, 0x60, %o4	! 0x40032c60 <RAM>
mov  %l3, %l0
ldub  [ %o4 + %l0 ], %g3
b  0x40006020 <K6502_Step__FUs+15968>
add  %g3, -1, %g3
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3dc ], %o4	! 0x40031bdc <Y>
inc  %o4
sethi  %hi(0x40031800), %o0
sethi  %hi(0x40031800), %g2
or  %o0, 0x3e0, %o0
and  %o4, 0xff, %o2
ldub  [ %o0 + %o2 ], %o3
ldub  [ %g2 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %o5
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o5 + 0x24c ], %o0
and  %o1, 0x7d, %o1
or  %o1, %o3, %o1
add  %o2, 2, %o2
add  %o0, 2, %o0
stb  %o1, [ %g2 + 0x3d9 ]
st  %o0, [ %o5 + 0x24c ]
stb  %o4, [ %g3 + 0x3dc ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o5
sethi  %hi(0x40031800), %o0
ldub  [ %o5 ], %o1
ldub  [ %o0 + 0x3da ], %o2
sub  %o2, %o1, %o2
sethi  %hi(0x40031800), %o4
sll  %o2, 0x10, %o3
sethi  %hi(0x40031800), %o0
ldub  [ %o4 + 0x3d9 ], %o1
or  %o0, 0x3e0, %o0
and  %o2, 0xff, %o2
and  %o1, 0x7c, %o1
ldub  [ %o0 + %o2 ], %o2
srl  %o3, 0x10, %o3
inc  %o5
or  %o4, 0x3d9, %g3
mov  %o1, %g2
st  %o5, [ %i0 + 0x3d0 ]
cmp  %o3, 0xff
bgu  0x40005708 <K6502_Step__FUs+13640>
stb  %o1, [ %o4 + 0x3d9 ]
or  %g2, 1, %o0
b  0x4000570c <K6502_Step__FUs+13644>
or  %o0, %o2, %o3
or  %g2, %o2, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
b  0x40006a00 <K6502_Step__FUs+18496>
stb  %o3, [ %g3 ]
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3db ], %o4	! 0x40031bdb <X>
b  0x4000607c <K6502_Step__FUs+16060>
add  %o4, -1, %o4
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %o0
sll  %o0, 8, %o0
or  %l3, %o0, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
sethi  %hi(0x40031800), %o2
inc  %o1
or  %o2, 0x250, %o2
sll  %o3, 2, %o3
ld  [ %o2 + %o3 ], %o4
st  %o1, [ %i0 + 0x3d0 ]
call  %o4
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o1
ldub  [ %o1 + 0x3dc ], %o2	! 0x40031bdc <Y>
and  %o0, 0xff, %o0
sub  %o2, %o0, %o2
sethi  %hi(0x40031800), %o4
sll  %o2, 0x10, %o3
sethi  %hi(0x40031800), %o0
ldub  [ %o4 + 0x3d9 ], %o1
or  %o0, 0x3e0, %o0
and  %o2, 0xff, %o2
and  %o1, 0x7c, %o1
srl  %o3, 0x10, %o3
ldub  [ %o0 + %o2 ], %o2
or  %o4, 0x3d9, %g2
mov  %o1, %o5
cmp  %o3, 0xff
bgu  0x400057c8 <K6502_Step__FUs+13832>
stb  %o1, [ %o4 + 0x3d9 ]
or  %o5, 1, %o0
b  0x400057cc <K6502_Step__FUs+13836>
or  %o0, %o2, %o3
or  %o5, %o2, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %g2 ]
add  %o1, 4, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 4, %o0
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %o0
sll  %o0, 8, %o0
or  %l3, %o0, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
sethi  %hi(0x40031800), %o2
inc  %o1
or  %o2, 0x250, %o2
sll  %o3, 2, %o3
ld  [ %o2 + %o3 ], %o4
st  %o1, [ %i0 + 0x3d0 ]
call  %o4
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o1
ldub  [ %o1 + 0x3da ], %o2	! 0x40031bda <A>
and  %o0, 0xff, %o0
sub  %o2, %o0, %o2
sethi  %hi(0x40031800), %o4
sll  %o2, 0x10, %o3
sethi  %hi(0x40031800), %o0
ldub  [ %o4 + 0x3d9 ], %o1
or  %o0, 0x3e0, %o0
and  %o2, 0xff, %o2
and  %o1, 0x7c, %o1
srl  %o3, 0x10, %o3
ldub  [ %o0 + %o2 ], %o2
or  %o4, 0x3d9, %g2
mov  %o1, %o5
cmp  %o3, 0xff
bgu  0x40005880 <K6502_Step__FUs+14016>
stb  %o1, [ %o4 + 0x3d9 ]
or  %o5, 1, %o0
b  0x40005884 <K6502_Step__FUs+14020>
or  %o0, %o2, %o3
or  %o5, %o2, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %g2 ]
add  %o1, 4, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 4, %o0
ld  [ %i0 + 0x3d0 ], %o2
ldub  [ %o2 ], %l3
inc  %o2
st  %o2, [ %i0 + 0x3d0 ]
ldub  [ %o2 ], %o5
sll  %o5, 8, %o5
sethi  %hi(0x40032c00), %g3
or  %l3, %o5, %o5
or  %g3, 0x60, %g3
ldub  [ %g3 + %o5 ], %g2
add  %g2, -1, %g2
sethi  %hi(0x40031800), %o0
sethi  %hi(0x40031800), %l0
or  %o0, 0x3e0, %o0
and  %g2, 0xff, %o3
ldub  [ %o0 + %o3 ], %o4
ldub  [ %l0 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %o7
inc  %o2
lduh  [ %l4 + 0x248 ], %o3
ld  [ %o7 + 0x24c ], %o0
and  %o1, 0x7d, %o1
st  %o2, [ %i0 + 0x3d0 ]
stb  %g2, [ %g3 + %o5 ]
or  %o1, %o4, %o1
add  %o3, 6, %o3
b  0x40006960 <K6502_Step__FUs+18336>
add  %o0, 6, %o0
sethi  %hi(0x40031800), %o0
ldub  [ %o0 + 0x3d9 ], %o1	! 0x40031bd9 <F>
btst  2, %o1
be  0x4000639c <K6502_Step__FUs+16860>
sethi  %hi(0x40031800), %o3
ld  [ %i0 + 0x3d0 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o3 + 0x24c ], %o0
inc  %o1
add  %o2, 2, %o2
b  0x400069e4 <K6502_Step__FUs+18468>
add  %o0, 2, %o0
ld  [ %i0 + 0x3d0 ], %g2
ldub  [ %g2 ], %l3
mov  %l3, %o5
sethi  %hi(0x40032c00), %o2
or  %o2, 0x60, %o2	! 0x40032c60 <RAM>
add  %o5, 1, %o0
ldub  [ %o2 + %o0 ], %o3
ldub  [ %o2 + %o5 ], %o1
sll  %o3, 8, %o3
sethi  %hi(0x40031800), %o4
or  %o1, %o3, %o1
ldub  [ %o4 + 0x3dc ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
cmp  %g0, %o1
sll  %o0, 0x10, %o0
sethi  %hi(0x40031800), %l0
addx  %g0, 0, %o5
lduh  [ %l4 + 0x248 ], %o3
ld  [ %l0 + 0x24c ], %o4
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o4, %o5, %o4
add  %o3, %o5, %o3
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %o5
inc  %g2
st  %g2, [ %i0 + 0x3d0 ]
sth  %o3, [ %l4 + 0x248 ]
st  %o4, [ %l0 + 0x24c ]
call  %o5
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o1
ldub  [ %o1 + 0x3da ], %o2	! 0x40031bda <A>
and  %o0, 0xff, %o0
sub  %o2, %o0, %o2
sethi  %hi(0x40031800), %o4
sll  %o2, 0x10, %o3
sethi  %hi(0x40031800), %o0
ldub  [ %o4 + 0x3d9 ], %o1
or  %o0, 0x3e0, %o0
and  %o2, 0xff, %o2
and  %o1, 0x7c, %o1
srl  %o3, 0x10, %o3
ldub  [ %o0 + %o2 ], %o2
or  %o4, 0x3d9, %g2
mov  %o1, %o5
cmp  %o3, 0xff
bgu  0x40005a20 <K6502_Step__FUs+14432>
stb  %o1, [ %o4 + 0x3d9 ]
or  %o5, 1, %o0
b  0x40005a24 <K6502_Step__FUs+14436>
or  %o0, %o2, %o2
or  %o5, %o2, %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %l0 + 0x24c ], %o0
stb  %o2, [ %g2 ]
add  %o1, 5, %o1
add  %o0, 5, %o0
b  0x40006a0c <K6502_Step__FUs+18508>
st  %o0, [ %l0 + 0x24c ]
ld  [ %i0 + 0x3d0 ], %g2
sethi  %hi(0x40031800), %o0
ldub  [ %g2 ], %o2
ldub  [ %o0 + 0x3db ], %o1
add  %o1, %o2, %o1
sethi  %hi(0x40032c00), %o0
or  %o0, 0x60, %o0	! 0x40032c60 <RAM>
sethi  %hi(0x40031800), %o2
ldub  [ %o0 + %o1 ], %o3
ldub  [ %o2 + 0x3da ], %o4
sub  %o4, %o3, %o4
sethi  %hi(0x40031800), %o5
sll  %o4, 0x10, %o2
sethi  %hi(0x40031800), %o0
ldub  [ %o5 + 0x3d9 ], %o1
or  %o0, 0x3e0, %o0
and  %o4, 0xff, %o4
and  %o1, 0x7c, %o1
ldub  [ %o0 + %o4 ], %o3
srl  %o2, 0x10, %o2
inc  %g2
or  %o5, 0x3d9, %o7
mov  %o1, %g3
st  %g2, [ %i0 + 0x3d0 ]
cmp  %o2, 0xff
bgu  0x40005ab8 <K6502_Step__FUs+14584>
stb  %o1, [ %o5 + 0x3d9 ]
or  %g3, 1, %o0
b  0x40005abc <K6502_Step__FUs+14588>
or  %o0, %o3, %o3
or  %g3, %o3, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %o7 ]
add  %o1, 4, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 4, %o0
ld  [ %i0 + 0x3d0 ], %g2
sethi  %hi(0x40031800), %o0
ldub  [ %g2 ], %o1
ldub  [ %o0 + 0x3db ], %o5
sethi  %hi(0x40032c00), %o4
add  %o5, %o1, %o5
or  %o4, 0x60, %o4
ldub  [ %o4 + %o5 ], %g3
b  0x40006610 <K6502_Step__FUs+17488>
add  %g3, -1, %o7
sethi  %hi(0x40031800), %o3
sethi  %hi(0x40031800), %o4
ldub  [ %o3 + 0x3d9 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o4 + 0x24c ], %o0
b  0x40006678 <K6502_Step__FUs+17592>
and  %o1, 0xf7, %o1
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o3 ], %l3
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o3 ], %o1
sll  %o1, 8, %o1
sethi  %hi(0x40031800), %o2
or  %l3, %o1, %o1
ldub  [ %o2 + 0x3dc ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
cmp  %g0, %o1
sll  %o0, 0x10, %o0
sethi  %hi(0x40031800), %l0
addx  %g0, 0, %g2
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l0 + 0x24c ], %o5
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o5, %g2, %o5
add  %o4, %g2, %o4
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %g2
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
sth  %o4, [ %l4 + 0x248 ]
st  %o5, [ %l0 + 0x24c ]
call  %g2
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o1
ldub  [ %o1 + 0x3da ], %o2	! 0x40031bda <A>
and  %o0, 0xff, %o0
sub  %o2, %o0, %o2
sethi  %hi(0x40031800), %o4
sll  %o2, 0x10, %o3
sethi  %hi(0x40031800), %o0
ldub  [ %o4 + 0x3d9 ], %o1
or  %o0, 0x3e0, %o0
and  %o2, 0xff, %o2
and  %o1, 0x7c, %o1
srl  %o3, 0x10, %o3
ldub  [ %o0 + %o2 ], %o2
or  %o4, 0x3d9, %g2
mov  %o1, %o5
cmp  %o3, 0xff
bgu  0x40005bf0 <K6502_Step__FUs+14896>
stb  %o1, [ %o4 + 0x3d9 ]
or  %o5, 1, %o0
b  0x40005bf4 <K6502_Step__FUs+14900>
or  %o0, %o2, %o2
or  %o5, %o2, %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %l0 + 0x24c ], %o0
stb  %o2, [ %g2 ]
add  %o1, 4, %o1
add  %o0, 4, %o0
b  0x40006a0c <K6502_Step__FUs+18508>
st  %o0, [ %l0 + 0x24c ]
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o3 ], %l3
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o3 ], %o1
sll  %o1, 8, %o1
sethi  %hi(0x40031800), %o2
or  %l3, %o1, %o1
ldub  [ %o2 + 0x3db ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
cmp  %g0, %o1
sll  %o0, 0x10, %o0
sethi  %hi(0x40031800), %l0
addx  %g0, 0, %g2
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l0 + 0x24c ], %o5
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o5, %g2, %o5
add  %o4, %g2, %o4
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %g2
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
sth  %o4, [ %l4 + 0x248 ]
st  %o5, [ %l0 + 0x24c ]
call  %g2
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o1
ldub  [ %o1 + 0x3da ], %o2	! 0x40031bda <A>
and  %o0, 0xff, %o0
sub  %o2, %o0, %o2
sethi  %hi(0x40031800), %o4
sll  %o2, 0x10, %o3
sethi  %hi(0x40031800), %o0
ldub  [ %o4 + 0x3d9 ], %o1
or  %o0, 0x3e0, %o0
and  %o2, 0xff, %o2
and  %o1, 0x7c, %o1
srl  %o3, 0x10, %o3
ldub  [ %o0 + %o2 ], %o2
or  %o4, 0x3d9, %g2
mov  %o1, %o5
cmp  %o3, 0xff
bgu  0x40005ce4 <K6502_Step__FUs+15140>
stb  %o1, [ %o4 + 0x3d9 ]
or  %o5, 1, %o0
b  0x40005ce8 <K6502_Step__FUs+15144>
or  %o0, %o2, %o2
or  %o5, %o2, %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %l0 + 0x24c ], %o0
stb  %o2, [ %g2 ]
add  %o1, 4, %o1
add  %o0, 4, %o0
b  0x40006a0c <K6502_Step__FUs+18508>
st  %o0, [ %l0 + 0x24c ]
ld  [ %i0 + 0x3d0 ], %o4
ldub  [ %o4 ], %l3
inc  %o4
st  %o4, [ %i0 + 0x3d0 ]
ldub  [ %o4 ], %o2
sethi  %hi(0x40031800), %o1
sll  %o2, 8, %o2
ldub  [ %o1 + 0x3db ], %o0
or  %l3, %o2, %o2
add  %o2, %o0, %o2
sll  %o2, 0x10, %o2
sethi  %hi(0x40032c00), %g3
or  %g3, 0x60, %g3	! 0x40032c60 <RAM>
srl  %o2, 0x10, %o2
ldub  [ %g3 + %o2 ], %g2
b  0x40006920 <K6502_Step__FUs+18272>
add  %g2, -1, %g2
ld  [ %i0 + 0x3d0 ], %o5
sethi  %hi(0x40031800), %o0
ldub  [ %o5 ], %o1
ldub  [ %o0 + 0x3db ], %o2
sub  %o2, %o1, %o2
sethi  %hi(0x40031800), %o4
sll  %o2, 0x10, %o3
sethi  %hi(0x40031800), %o0
ldub  [ %o4 + 0x3d9 ], %o1
or  %o0, 0x3e0, %o0
and  %o2, 0xff, %o2
and  %o1, 0x7c, %o1
ldub  [ %o0 + %o2 ], %o2
srl  %o3, 0x10, %o3
inc  %o5
or  %o4, 0x3d9, %g3
mov  %o1, %g2
st  %o5, [ %i0 + 0x3d0 ]
cmp  %o3, 0xff
bgu  0x40005da8 <K6502_Step__FUs+15336>
stb  %o1, [ %o4 + 0x3d9 ]
or  %g2, 1, %o0
b  0x40005dac <K6502_Step__FUs+15340>
or  %o0, %o2, %o3
or  %g2, %o2, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
b  0x40006a00 <K6502_Step__FUs+18496>
stb  %o3, [ %g3 ]
ld  [ %i0 + 0x3d0 ], %o4
sethi  %hi(0x40031800), %o0
ldub  [ %o4 ], %o1
ldub  [ %o0 + 0x3db ], %o3
add  %o3, %o1, %o3
sethi  %hi(0x40032c00), %o2
or  %o2, 0x60, %o2	! 0x40032c60 <RAM>
add  %o3, 1, %o0
ldub  [ %o2 + %o0 ], %o1
ldub  [ %o2 + %o3 ], %o0
sll  %o1, 8, %o1
or  %o0, %o1, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
or  %o1, 0x250, %o1	! 0x40031a50 <readmem_tbl>
sll  %o2, 2, %o2
inc  %o4
ld  [ %o1 + %o2 ], %o3
st  %o4, [ %i0 + 0x3d0 ]
call  %o3
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o4
ldub  [ %o4 + 0x3d9 ], %o1	! 0x40031bd9 <F>
mov  %o0, %g3
and  %o1, 0x3c, %l0
sethi  %hi(0x40031800), %o5
ldub  [ %o5 + 0x3da ], %o2	! 0x40031bda <A>
and  %g3, 0xff, %o0
xnor  %g0, %o1, %o1
sub  %o2, %o0, %o0
and  %o1, 1, %o1
sub  %o0, %o1, %o7
mov  %o7, %l3
xor  %o2, %g3, %o3
sethi  %hi(0x40031800), %o1
xor  %o2, %l3, %o2
or  %o1, 0x3e0, %o1
and  %l3, 0xff, %o0
and  %o3, %o2, %o3
ldub  [ %o1 + %o0 ], %l1
or  %o4, 0x3d9, %l2
stb  %l0, [ %o4 + 0x3d9 ]
btst  0x80, %o3
be  0x40005e7c <K6502_Step__FUs+15548>
mov  %o5, %g2
or  %l1, 0x40, %l1
sll  %o7, 0x10, %o0
srl  %o0, 0x10, %o0
cmp  %o0, 0xff
bgu  0x40005e9c <K6502_Step__FUs+15580>
mov  %l0, %o1
or  %o1, 1, %o0
b  0x40005ea0 <K6502_Step__FUs+15584>
or  %o0, %l1, %o3
or  %o1, %l1, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %l2 ]
add  %o1, 6, %o1
add  %o0, 6, %o0
b  0x40006a08 <K6502_Step__FUs+18504>
stb  %l3, [ %g2 + 0x3da ]
ld  [ %i0 + 0x3d0 ], %o5
sethi  %hi(0x40032c00), %o0
ldub  [ %o5 ], %l3
or  %o0, 0x60, %o0
sethi  %hi(0x40031800), %o1
ldub  [ %l3 + %o0 ], %o2
ldub  [ %o1 + 0x3db ], %o3
sub  %o3, %o2, %o3
sethi  %hi(0x40031800), %o4
sll  %o3, 0x10, %o2
sethi  %hi(0x40031800), %o0
ldub  [ %o4 + 0x3d9 ], %o1
or  %o0, 0x3e0, %o0
and  %o3, 0xff, %o3
and  %o1, 0x7c, %o1
ldub  [ %o0 + %o3 ], %g2
srl  %o2, 0x10, %o2
inc  %o5
or  %o4, 0x3d9, %o7
mov  %o1, %g3
st  %o5, [ %i0 + 0x3d0 ]
cmp  %o2, 0xff
bgu  0x40005f2c <K6502_Step__FUs+15724>
stb  %o1, [ %o4 + 0x3d9 ]
or  %g3, 1, %o0
b  0x40005f30 <K6502_Step__FUs+15728>
or  %o0, %g2, %g2
or  %g3, %g2, %g2
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %g2, [ %o7 ]
add  %o1, 3, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 3, %o0
sethi  %hi(0x40031800), %o5
ld  [ %i0 + 0x3d0 ], %o4
ldub  [ %o5 + 0x3d9 ], %o1
sethi  %hi(0x40032c00), %o0
ldub  [ %o4 ], %l3
or  %o0, 0x60, %o0
and  %o1, 0x3c, %l0
sethi  %hi(0x40031800), %g2
ldub  [ %l3 + %o0 ], %g3
ldub  [ %g2 + 0x3da ], %o2
xnor  %g0, %o1, %o1
sub  %o2, %g3, %o0
and  %o1, 1, %o1
sub  %o0, %o1, %o7
mov  %o7, %l3
xor  %o2, %g3, %o3
sethi  %hi(0x40031800), %o0
inc  %o4
xor  %o2, %l3, %o2
or  %o0, 0x3e0, %o0
and  %l3, 0xff, %o1
st  %o4, [ %i0 + 0x3d0 ]
and  %o3, %o2, %o3
ldub  [ %o0 + %o1 ], %l1
or  %o5, 0x3d9, %l2
btst  0x80, %o3
be  0x40005fc0 <K6502_Step__FUs+15872>
stb  %l0, [ %o5 + 0x3d9 ]
or  %l1, 0x40, %l1
sll  %o7, 0x10, %o0
srl  %o0, 0x10, %o0
cmp  %o0, 0xff
bgu  0x40005fe0 <K6502_Step__FUs+15904>
mov  %l0, %o1
or  %o1, 1, %o0
b  0x40005fe4 <K6502_Step__FUs+15908>
or  %o0, %l1, %o3
or  %o1, %l1, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %l2 ]
add  %o1, 3, %o1
add  %o0, 3, %o0
b  0x40006a08 <K6502_Step__FUs+18504>
stb  %l3, [ %g2 + 0x3da ]
ld  [ %i0 + 0x3d0 ], %o5
ldub  [ %o5 ], %l3
sethi  %hi(0x40032c00), %o4
or  %o4, 0x60, %o4	! 0x40032c60 <RAM>
mov  %l3, %l0
ldub  [ %o4 + %l0 ], %g3
inc  %g3
sethi  %hi(0x40031800), %o0
sethi  %hi(0x40031800), %o7
or  %o0, 0x3e0, %o0
and  %g3, 0xff, %o2
ldub  [ %o0 + %o2 ], %o3
ldub  [ %o7 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %g2
inc  %o5
lduh  [ %l4 + 0x248 ], %o2
ld  [ %g2 + 0x24c ], %o0
and  %o1, 0x7d, %o1
st  %o5, [ %i0 + 0x3d0 ]
stb  %g3, [ %o4 + %l0 ]
or  %o1, %o3, %o1
add  %o2, 5, %o2
add  %o0, 5, %o0
stb  %o1, [ %o7 + 0x3d9 ]
st  %o0, [ %g2 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %g3
ldub  [ %g3 + 0x3db ], %o4	! 0x40031bdb <X>
inc  %o4
sethi  %hi(0x40031800), %o0
sethi  %hi(0x40031800), %g2
or  %o0, 0x3e0, %o0
and  %o4, 0xff, %o2
ldub  [ %o0 + %o2 ], %o3
ldub  [ %g2 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %o5
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o5 + 0x24c ], %o0
and  %o1, 0x7d, %o1
or  %o1, %o3, %o1
add  %o2, 2, %o2
add  %o0, 2, %o0
stb  %o1, [ %g2 + 0x3d9 ]
st  %o0, [ %o5 + 0x24c ]
stb  %o4, [ %g3 + 0x3db ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %o5
ld  [ %i0 + 0x3d0 ], %o4
ldub  [ %o5 + 0x3d9 ], %o1
ldub  [ %o4 ], %g3
and  %o1, 0x3c, %l0
sethi  %hi(0x40031800), %g2
ldub  [ %g2 + 0x3da ], %o2	! 0x40031bda <A>
and  %g3, 0xff, %o0
xnor  %g0, %o1, %o1
sub  %o2, %o0, %o0
and  %o1, 1, %o1
sub  %o0, %o1, %o7
mov  %o7, %l3
xor  %o2, %g3, %o3
sethi  %hi(0x40031800), %o0
inc  %o4
xor  %o2, %l3, %o2
or  %o0, 0x3e0, %o0
and  %l3, 0xff, %o1
st  %o4, [ %i0 + 0x3d0 ]
and  %o3, %o2, %o3
ldub  [ %o0 + %o1 ], %l1
or  %o5, 0x3d9, %l2
btst  0x80, %o3
be  0x40006130 <K6502_Step__FUs+16240>
stb  %l0, [ %o5 + 0x3d9 ]
or  %l1, 0x40, %l1
sll  %o7, 0x10, %o0
srl  %o0, 0x10, %o0
cmp  %o0, 0xff
bgu  0x40006150 <K6502_Step__FUs+16272>
mov  %l0, %o1
or  %o1, 1, %o0
b  0x40006154 <K6502_Step__FUs+16276>
or  %o0, %l1, %o3
or  %o1, %l1, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %l2 ]
add  %o1, 2, %o1
add  %o0, 2, %o0
b  0x40006a08 <K6502_Step__FUs+18504>
stb  %l3, [ %g2 + 0x3da ]
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %o0
sll  %o0, 8, %o0
or  %l3, %o0, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
sethi  %hi(0x40031800), %o2
inc  %o1
or  %o2, 0x250, %o2
sll  %o3, 2, %o3
ld  [ %o2 + %o3 ], %o4
st  %o1, [ %i0 + 0x3d0 ]
call  %o4
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o1
ldub  [ %o1 + 0x3db ], %o2	! 0x40031bdb <X>
and  %o0, 0xff, %o0
sub  %o2, %o0, %o2
sethi  %hi(0x40031800), %o4
sll  %o2, 0x10, %o3
sethi  %hi(0x40031800), %o0
ldub  [ %o4 + 0x3d9 ], %o1
or  %o0, 0x3e0, %o0
and  %o2, 0xff, %o2
and  %o1, 0x7c, %o1
srl  %o3, 0x10, %o3
ldub  [ %o0 + %o2 ], %o2
or  %o4, 0x3d9, %g2
mov  %o1, %o5
cmp  %o3, 0xff
bgu  0x4000620c <K6502_Step__FUs+16460>
stb  %o1, [ %o4 + 0x3d9 ]
or  %o5, 1, %o0
b  0x40006210 <K6502_Step__FUs+16464>
or  %o0, %o2, %o3
or  %o5, %o2, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %g2 ]
add  %o1, 4, %o1
b  0x40006a08 <K6502_Step__FUs+18504>
add  %o0, 4, %o0
ld  [ %i0 + 0x3d0 ], %o1
ldub  [ %o1 ], %l3
inc  %o1
st  %o1, [ %i0 + 0x3d0 ]
ldub  [ %o1 ], %o0
sll  %o0, 8, %o0
or  %l3, %o0, %o0
sll  %o0, 0x10, %o0
srl  %o0, 0x1d, %o3
sethi  %hi(0x40031800), %o2
inc  %o1
or  %o2, 0x250, %o2
sll  %o3, 2, %o3
ld  [ %o2 + %o3 ], %o4
st  %o1, [ %i0 + 0x3d0 ]
call  %o4
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o5
ldub  [ %o5 + 0x3d9 ], %o1	! 0x40031bd9 <F>
mov  %o0, %g3
and  %o1, 0x3c, %l0
sethi  %hi(0x40031800), %o4
ldub  [ %o4 + 0x3da ], %o2	! 0x40031bda <A>
and  %g3, 0xff, %o0
xnor  %g0, %o1, %o1
sub  %o2, %o0, %o0
and  %o1, 1, %o1
sub  %o0, %o1, %o7
mov  %o7, %l3
xor  %o2, %g3, %o3
sethi  %hi(0x40031800), %o1
xor  %o2, %l3, %o2
or  %o1, 0x3e0, %o1
and  %l3, 0xff, %o0
and  %o3, %o2, %o3
ldub  [ %o1 + %o0 ], %l1
or  %o5, 0x3d9, %l2
stb  %l0, [ %o5 + 0x3d9 ]
btst  0x80, %o3
be  0x400062d4 <K6502_Step__FUs+16660>
mov  %o4, %g2
or  %l1, 0x40, %l1
sll  %o7, 0x10, %o0
srl  %o0, 0x10, %o0
cmp  %o0, 0xff
bgu  0x400062f4 <K6502_Step__FUs+16692>
mov  %l0, %o1
or  %o1, 1, %o0
b  0x400062f8 <K6502_Step__FUs+16696>
or  %o0, %l1, %o3
or  %o1, %l1, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %l2 ]
add  %o1, 4, %o1
add  %o0, 4, %o0
b  0x40006a08 <K6502_Step__FUs+18504>
stb  %l3, [ %g2 + 0x3da ]
ld  [ %i0 + 0x3d0 ], %o2
ldub  [ %o2 ], %l3
inc  %o2
st  %o2, [ %i0 + 0x3d0 ]
ldub  [ %o2 ], %o5
sll  %o5, 8, %o5
sethi  %hi(0x40032c00), %g3
or  %l3, %o5, %o5
or  %g3, 0x60, %g3
ldub  [ %g3 + %o5 ], %g2
inc  %g2
sethi  %hi(0x40031800), %o0
sethi  %hi(0x40031800), %l0
or  %o0, 0x3e0, %o0
and  %g2, 0xff, %o3
ldub  [ %o0 + %o3 ], %o4
ldub  [ %l0 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %o7
inc  %o2
lduh  [ %l4 + 0x248 ], %o3
ld  [ %o7 + 0x24c ], %o0
and  %o1, 0x7d, %o1
st  %o2, [ %i0 + 0x3d0 ]
stb  %g2, [ %g3 + %o5 ]
or  %o1, %o4, %o1
add  %o3, 6, %o3
b  0x40006960 <K6502_Step__FUs+18336>
add  %o0, 6, %o0
sethi  %hi(0x40031800), %o0
ldub  [ %o0 + 0x3d9 ], %o1	! 0x40031bd9 <F>
btst  2, %o1
be  0x400063d8 <K6502_Step__FUs+16920>
sethi  %hi(0x40031800), %o3
ld  [ %i0 + 0x3d0 ], %o2
ldub  [ %o2 ], %o0
sethi  %hi(0x40031800), %o4
sll  %o0, 0x18, %o0
lduh  [ %l4 + 0x248 ], %o3
ld  [ %o4 + 0x24c ], %o1
sra  %o0, 0x18, %o0
inc  %o2
add  %o2, %o0, %o2
add  %o3, 3, %o3
add  %o1, 3, %o1
st  %o2, [ %i0 + 0x3d0 ]
st  %o1, [ %o4 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o3, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o3 + 0x24c ], %o0
inc  %o1
add  %o2, 2, %o2
b  0x400069e4 <K6502_Step__FUs+18468>
add  %o0, 2, %o0
ld  [ %i0 + 0x3d0 ], %g2
ldub  [ %g2 ], %l3
mov  %l3, %o5
sethi  %hi(0x40032c00), %o2
or  %o2, 0x60, %o2	! 0x40032c60 <RAM>
add  %o5, 1, %o0
ldub  [ %o2 + %o0 ], %o3
ldub  [ %o2 + %o5 ], %o1
sll  %o3, 8, %o3
sethi  %hi(0x40031800), %o4
or  %o1, %o3, %o1
ldub  [ %o4 + 0x3dc ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
cmp  %g0, %o1
sll  %o0, 0x10, %o0
sethi  %hi(0x40031800), %l0
addx  %g0, 0, %o5
lduh  [ %l4 + 0x248 ], %o3
ld  [ %l0 + 0x24c ], %o4
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o4, %o5, %o4
add  %o3, %o5, %o3
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %o5
inc  %g2
st  %g2, [ %i0 + 0x3d0 ]
sth  %o3, [ %l4 + 0x248 ]
st  %o4, [ %l0 + 0x24c ]
call  %o5
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o4
ldub  [ %o4 + 0x3d9 ], %o1	! 0x40031bd9 <F>
mov  %o0, %g3
and  %o1, 0x3c, %l2
sethi  %hi(0x40031800), %o5
ldub  [ %o5 + 0x3da ], %o2	! 0x40031bda <A>
and  %g3, 0xff, %o0
xnor  %g0, %o1, %o1
sub  %o2, %o0, %o0
and  %o1, 1, %o1
sub  %o0, %o1, %o7
mov  %o7, %l3
xor  %o2, %g3, %o3
sethi  %hi(0x40031800), %o1
xor  %o2, %l3, %o2
or  %o1, 0x3e0, %o1
and  %l3, 0xff, %o0
and  %o3, %o2, %o3
ldub  [ %o1 + %o0 ], %l1
or  %o4, 0x3d9, %i0
stb  %l2, [ %o4 + 0x3d9 ]
btst  0x80, %o3
be  0x400064e4 <K6502_Step__FUs+17188>
mov  %o5, %g2
or  %l1, 0x40, %l1
sll  %o7, 0x10, %o0
srl  %o0, 0x10, %o0
cmp  %o0, 0xff
bgu  0x40006504 <K6502_Step__FUs+17220>
mov  %l2, %o1
or  %o1, 1, %o0
b  0x40006508 <K6502_Step__FUs+17224>
or  %o0, %l1, %o2
or  %o1, %l1, %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %l0 + 0x24c ], %o0
stb  %o2, [ %i0 ]
add  %o1, 5, %o1
add  %o0, 5, %o0
stb  %l3, [ %g2 + 0x3da ]
b  0x40006a0c <K6502_Step__FUs+18508>
st  %o0, [ %l0 + 0x24c ]
ld  [ %i0 + 0x3d0 ], %o4
sethi  %hi(0x40031800), %o0
ldub  [ %o4 ], %o1
ldub  [ %o0 + 0x3db ], %o3
sethi  %hi(0x40031800), %o5
add  %o3, %o1, %o3
ldub  [ %o5 + 0x3d9 ], %o1
sethi  %hi(0x40032c00), %o0
or  %o0, 0x60, %o0	! 0x40032c60 <RAM>
and  %o1, 0x3c, %l0
sethi  %hi(0x40031800), %g2
ldub  [ %o0 + %o3 ], %g3
ldub  [ %g2 + 0x3da ], %o2
xnor  %g0, %o1, %o1
sub  %o2, %g3, %o0
and  %o1, 1, %o1
sub  %o0, %o1, %o7
mov  %o7, %l3
xor  %o2, %g3, %o3
sethi  %hi(0x40031800), %o0
inc  %o4
xor  %o2, %l3, %o2
or  %o0, 0x3e0, %o0
and  %l3, 0xff, %o1
st  %o4, [ %i0 + 0x3d0 ]
and  %o3, %o2, %o3
ldub  [ %o0 + %o1 ], %l1
or  %o5, 0x3d9, %l2
btst  0x80, %o3
be  0x400065a8 <K6502_Step__FUs+17384>
stb  %l0, [ %o5 + 0x3d9 ]
or  %l1, 0x40, %l1
sll  %o7, 0x10, %o0
srl  %o0, 0x10, %o0
cmp  %o0, 0xff
bgu  0x400065c8 <K6502_Step__FUs+17416>
mov  %l0, %o1
or  %o1, 1, %o0
b  0x400065cc <K6502_Step__FUs+17420>
or  %o0, %l1, %o3
or  %o1, %l1, %o3
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
stb  %o3, [ %l2 ]
add  %o1, 4, %o1
add  %o0, 4, %o0
b  0x40006a08 <K6502_Step__FUs+18504>
stb  %l3, [ %g2 + 0x3da ]
ld  [ %i0 + 0x3d0 ], %g2
sethi  %hi(0x40031800), %o0
ldub  [ %g2 ], %o1
ldub  [ %o0 + 0x3db ], %o5
sethi  %hi(0x40032c00), %o4
add  %o5, %o1, %o5
or  %o4, 0x60, %o4
ldub  [ %o4 + %o5 ], %g3
add  %g3, 1, %o7
sethi  %hi(0x40031800), %o0
sethi  %hi(0x40031800), %l0
or  %o0, 0x3e0, %o0
and  %o7, 0xff, %o2
ldub  [ %o0 + %o2 ], %o3
ldub  [ %l0 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %g3
inc  %g2
lduh  [ %l4 + 0x248 ], %o2
ld  [ %g3 + 0x24c ], %o0
and  %o1, 0x7d, %o1
st  %g2, [ %i0 + 0x3d0 ]
stb  %o7, [ %o4 + %o5 ]
or  %o1, %o3, %o1
add  %o2, 6, %o2
add  %o0, 6, %o0
stb  %o1, [ %l0 + 0x3d9 ]
st  %o0, [ %g3 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %o3
sethi  %hi(0x40031800), %o4
ldub  [ %o3 + 0x3d9 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o4 + 0x24c ], %o0
or  %o1, 8, %o1
add  %o2, 2, %o2
add  %o0, 2, %o0
stb  %o1, [ %o3 + 0x3d9 ]
st  %o0, [ %o4 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o3 ], %l3
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o3 ], %o1
sll  %o1, 8, %o1
sethi  %hi(0x40031800), %o2
or  %l3, %o1, %o1
ldub  [ %o2 + 0x3dc ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
cmp  %g0, %o1
sll  %o0, 0x10, %o0
sethi  %hi(0x40031800), %l0
addx  %g0, 0, %g2
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l0 + 0x24c ], %o5
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o5, %g2, %o5
add  %o4, %g2, %o4
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %g2
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
sth  %o4, [ %l4 + 0x248 ]
st  %o5, [ %l0 + 0x24c ]
call  %g2
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o4
ldub  [ %o4 + 0x3d9 ], %o1	! 0x40031bd9 <F>
mov  %o0, %g3
and  %o1, 0x3c, %l2
sethi  %hi(0x40031800), %o5
ldub  [ %o5 + 0x3da ], %o2	! 0x40031bda <A>
and  %g3, 0xff, %o0
xnor  %g0, %o1, %o1
sub  %o2, %o0, %o0
and  %o1, 1, %o1
sub  %o0, %o1, %o7
mov  %o7, %l3
xor  %o2, %g3, %o3
sethi  %hi(0x40031800), %o1
xor  %o2, %l3, %o2
or  %o1, 0x3e0, %o1
and  %l3, 0xff, %o0
and  %o3, %o2, %o3
ldub  [ %o1 + %o0 ], %l1
or  %o4, 0x3d9, %i0
stb  %l2, [ %o4 + 0x3d9 ]
btst  0x80, %o3
be  0x40006774 <K6502_Step__FUs+17844>
mov  %o5, %g2
or  %l1, 0x40, %l1
sll  %o7, 0x10, %o0
srl  %o0, 0x10, %o0
cmp  %o0, 0xff
bgu  0x40006794 <K6502_Step__FUs+17876>
mov  %l2, %o1
or  %o1, 1, %o0
b  0x40006798 <K6502_Step__FUs+17880>
or  %o0, %l1, %o2
or  %o1, %l1, %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %l0 + 0x24c ], %o0
stb  %o2, [ %i0 ]
add  %o1, 4, %o1
add  %o0, 4, %o0
stb  %l3, [ %g2 + 0x3da ]
b  0x40006a0c <K6502_Step__FUs+18508>
st  %o0, [ %l0 + 0x24c ]
ld  [ %i0 + 0x3d0 ], %o3
ldub  [ %o3 ], %l3
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
ldub  [ %o3 ], %o1
sll  %o1, 8, %o1
sethi  %hi(0x40031800), %o2
or  %l3, %o1, %o1
ldub  [ %o2 + 0x3db ], %o0
add  %o1, %o0, %o0
and  %o0, 0x100, %o2
and  %o1, 0x100, %o1
xor  %o1, %o2, %o1
cmp  %g0, %o1
sll  %o0, 0x10, %o0
sethi  %hi(0x40031800), %l0
addx  %g0, 0, %g2
lduh  [ %l4 + 0x248 ], %o4
ld  [ %l0 + 0x24c ], %o5
srl  %o0, 0x1d, %o2
sethi  %hi(0x40031800), %o1
add  %o5, %g2, %o5
add  %o4, %g2, %o4
or  %o1, 0x250, %o1
sll  %o2, 2, %o2
ld  [ %o1 + %o2 ], %g2
inc  %o3
st  %o3, [ %i0 + 0x3d0 ]
sth  %o4, [ %l4 + 0x248 ]
st  %o5, [ %l0 + 0x24c ]
call  %g2
srl  %o0, 0x10, %o0
sethi  %hi(0x40031800), %o4
ldub  [ %o4 + 0x3d9 ], %o1	! 0x40031bd9 <F>
mov  %o0, %g3
and  %o1, 0x3c, %l2
sethi  %hi(0x40031800), %o5
ldub  [ %o5 + 0x3da ], %o2	! 0x40031bda <A>
and  %g3, 0xff, %o0
xnor  %g0, %o1, %o1
sub  %o2, %o0, %o0
and  %o1, 1, %o1
sub  %o0, %o1, %o7
mov  %o7, %l3
xor  %o2, %g3, %o3
sethi  %hi(0x40031800), %o1
xor  %o2, %l3, %o2
or  %o1, 0x3e0, %o1
and  %l3, 0xff, %o0
and  %o3, %o2, %o3
ldub  [ %o1 + %o0 ], %l1
or  %o4, 0x3d9, %i0
stb  %l2, [ %o4 + 0x3d9 ]
btst  0x80, %o3
be  0x4000689c <K6502_Step__FUs+18140>
mov  %o5, %g2
or  %l1, 0x40, %l1
sll  %o7, 0x10, %o0
srl  %o0, 0x10, %o0
cmp  %o0, 0xff
bgu  0x400068bc <K6502_Step__FUs+18172>
mov  %l2, %o1
or  %o1, 1, %o0
b  0x400068c0 <K6502_Step__FUs+18176>
or  %o0, %l1, %o2
or  %o1, %l1, %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %l0 + 0x24c ], %o0
stb  %o2, [ %i0 ]
add  %o1, 4, %o1
add  %o0, 4, %o0
stb  %l3, [ %g2 + 0x3da ]
b  0x40006a0c <K6502_Step__FUs+18508>
st  %o0, [ %l0 + 0x24c ]
ld  [ %i0 + 0x3d0 ], %o4
ldub  [ %o4 ], %l3
inc  %o4
st  %o4, [ %i0 + 0x3d0 ]
ldub  [ %o4 ], %o2
sethi  %hi(0x40031800), %o1
sll  %o2, 8, %o2
ldub  [ %o1 + 0x3db ], %o0
or  %l3, %o2, %o2
add  %o2, %o0, %o2
sll  %o2, 0x10, %o2
sethi  %hi(0x40032c00), %g3
or  %g3, 0x60, %g3	! 0x40032c60 <RAM>
srl  %o2, 0x10, %o2
ldub  [ %g3 + %o2 ], %g2
inc  %g2
sethi  %hi(0x40031800), %o0
sethi  %hi(0x40031800), %l0
or  %o0, 0x3e0, %o0
and  %g2, 0xff, %o3
ldub  [ %o0 + %o3 ], %o5
ldub  [ %l0 + 0x3d9 ], %o1
sethi  %hi(0x40031800), %o7
inc  %o4
lduh  [ %l4 + 0x248 ], %o3
ld  [ %o7 + 0x24c ], %o0
and  %o1, 0x7d, %o1
st  %o4, [ %i0 + 0x3d0 ]
stb  %g2, [ %g3 + %o2 ]
or  %o1, %o5, %o1
add  %o3, 7, %o3
add  %o0, 7, %o0
stb  %o1, [ %l0 + 0x3d9 ]
st  %o0, [ %o7 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o3, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %o3
ld  [ %i0 + 0x3d0 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o3 + 0x24c ], %o0
inc  %o1
add  %o2, 2, %o2
b  0x400069e4 <K6502_Step__FUs+18468>
add  %o0, 2, %o0
sethi  %hi(0x40031800), %o3
ld  [ %i0 + 0x3d0 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o3 + 0x24c ], %o0
inc  %o1
add  %o2, 3, %o2
b  0x400069e4 <K6502_Step__FUs+18468>
add  %o0, 3, %o0
sethi  %hi(0x40031800), %o3
ld  [ %i0 + 0x3d0 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o3 + 0x24c ], %o0
b  0x400069dc <K6502_Step__FUs+18460>
inc  %o1
sethi  %hi(0x40031800), %o3
ld  [ %i0 + 0x3d0 ], %o1
lduh  [ %l4 + 0x248 ], %o2
ld  [ %o3 + 0x24c ], %o0
add  %o1, 2, %o1
add  %o2, 4, %o2
add  %o0, 4, %o0
st  %o1, [ %i0 + 0x3d0 ]
st  %o0, [ %o3 + 0x24c ]
b  0x40006a10 <K6502_Step__FUs+18512>
sth  %o2, [ %l4 + 0x248 ]
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o1
ld  [ %o2 + 0x24c ], %o0
add  %o1, 2, %o1
add  %o0, 2, %o0
st  %o0, [ %o2 + 0x24c ]
sth  %o1, [ %l4 + 0x248 ]
sll  %l5, 0x10, %o0
srl  %o0, 0x10, %o0
lduh  [ %l4 + 0x248 ], %o1
cmp  %o1, %o0
bcs  0x400021e8 <K6502_Step__FUs+40>
sethi  %hi(0x40031800), %o2
lduh  [ %l4 + 0x248 ], %o0
sub  %o0, %l5, %o0
sth  %o0, [ %l4 + 0x248 ]
ret 
restore 
call  0x4000f2bc <do_single_div+20>
call  0x4000f680 <L1.16>
call  0x4000f914 <read+104>
call  0x40021218 <gamefile+66848>
call  0x4002108c <gamefile+66452>
call  0x4000f940 <read+148>
call  0x4000fa94 <write+176>
call  0x40021228 <gamefile+66864>
call  0x4000fc4c <gettimeofday+80>
call  0x4000fda0 <cpuinit+204>
call  0x4000fe14 <cpuinit+320>
call  0x40021238 <gamefile+66880>
call  0x4002118c <gamefile+66708>
call  0x4000fe60 <cpuinit+396>
call  0x400100a4 <__FRAME_BEGIN__+324>
call  0x40021248 <gamefile+66896>
call  0x4001028c <__FRAME_BEGIN__+348>
call  0x40010350 <__FRAME_BEGIN__+544>
call  0x4000f954 <read+168>
call  0x40021258 <gamefile+66912>
call  0x4002114c <gamefile+66644>
call  0x400105d0 <__FRAME_BEGIN__+1184>
call  0x400106a4 <__FRAME_BEGIN__+192>
call  0x40021268 <gamefile+66928>
call  0x4001087c <__FRAME_END__+8>
call  0x400108f0 <InfoNES_DrawLine__Fii+112>
call  0x40021274 <gamefile+66940>
call  0x40021278 <gamefile+66944>
call  0x400211cc <gamefile+66772>
call  0x40010b40 <NES_RefreshSprites__FPUcT0+68>
call  0x40010d84 <gamefile+140>
call  0x40021288 <gamefile+66960>
call  0x40010eec <gamefile+500>
call  0x400111d0 <gamefile+1240>
call  0x4000f994 <read+232>
call  0x40021298 <gamefile+66976>
call  0x4001146c <gamefile+1908>
call  0x40011640 <gamefile+2376>
call  0x40011794 <gamefile+2716>
call  0x400212a8 <gamefile+66992>
call  0x400118ac <gamefile+2996>
call  0x400119e0 <gamefile+3304>
call  0x40011a54 <gamefile+3420>
call  0x400212b8 <gamefile+67008>
call  0x40011b0c <gamefile+3604>
call  0x40011d90 <gamefile+4248>
call  0x40011fd4 <gamefile+4828>
call  0x400212c8 <gamefile+67024>
call  0x400120fc <gamefile+5124>
call  0x400121d0 <gamefile+5336>
call  0x4000f9d4 <read+296>
call  0x400212d8 <gamefile+67040>
call  0x400211cc <gamefile+66772>
call  0x40012530 <gamefile+6200>
call  0x40012604 <gamefile+6412>
call  0x400212e8 <gamefile+67056>
call  0x4001270c <gamefile+6676>
call  0x40012780 <gamefile+6792>
call  0x400212f4 <gamefile+67068>
call  0x400212f8 <gamefile+67072>
call  0x4002124c <gamefile+66900>
call  0x40012ab0 <gamefile+7608>
call  0x40012dd4 <gamefile+8412>
call  0x40021308 <gamefile+67088>
call  0x40012f4c <gamefile+8788>
call  0x40013200 <gamefile+9480>
call  0x4000fa14 <write+48>
call  0x40021318 <gamefile+67104>
call  0x4002118c <gamefile+66708>
call  0x400134a0 <gamefile+10152>
call  0x400135f4 <gamefile+10492>
call  0x40021328 <gamefile+67120>
call  0x400137ac <gamefile+10932>
call  0x400138e0 <gamefile+11240>
call  0x40013a64 <gamefile+11628>
call  0x40021338 <gamefile+67136>
call  0x40013bcc <gamefile+11988>
call  0x40013d80 <gamefile+12424>
call  0x40013fc4 <gamefile+13004>
call  0x40021348 <gamefile+67152>
call  0x400141ac <gamefile+13492>
call  0x40014270 <gamefile+13688>
call  0x4000fa54 <write+112>
call  0x40021358 <gamefile+67168>
call  0x4002124c <gamefile+66900>
call  0x400145f0 <gamefile+14584>
call  0x400147d4 <gamefile+15068>
call  0x40021368 <gamefile+67184>
call  0x400149ac <gamefile+15540>
call  0x40014a20 <gamefile+15656>
call  0x40021374 <gamefile+67196>
call  0x40021378 <gamefile+67200>
call  0x400212cc <gamefile+67028>
call  0x40014ad0 <gamefile+15832>
call  0x40014e14 <gamefile+16668>
call  0x40021388 <gamefile+67216>
call  0x4001507c <gamefile+17284>
call  0x400152d0 <gamefile+17880>
call  0x4000fa94 <write+176>
call  0x40021398 <gamefile+67232>
call  0x4002120c <gamefile+66836>
call  0x400156b0 <gamefile+18872>
call  0x40015974 <gamefile+19580>
call  0x400213a8 <gamefile+67248>
call  0x40015b7c <gamefile+20100>
call  0x40015d10 <gamefile+20504>
call  0x40015fa4 <gamefile+21164>
call  0x400213b8 <gamefile+67264>
call  0x4001613c <gamefile+21572>
call  0x40016430 <gamefile+22328>
call  0x400167b4 <gamefile+23228>
call  0x400213c8 <gamefile+67280>
call  0x400169fc <gamefile+23812>
call  0x40016ad0 <gamefile+24024>
call  0x4000fad4 <write+240>
call  0x400213d8 <gamefile+67296>
call  0x400212cc <gamefile+67028>
call  0x40016f80 <gamefile+25224>
call  0x40017274 <gamefile+25980>
call  0x400213e8 <gamefile+67312>
call  0x4001749c <gamefile+26532>
call  0x40017510 <gamefile+26648>
call  0x400213f4 <gamefile+67324>
call  0x400213f8 <gamefile+67328>
call  0x4002134c <gamefile+67156>
call  0x40017990 <gamefile+27800>
call  0x40017e04 <gamefile+28940>
call  0x40021408 <gamefile+67344>
call  0x400211fc <gamefile+66820>
call  0x400180a0 <gamefile+29608>
call  0x40021204 <gamefile+66828>
call  0x40021418 <gamefile+67360>
call  0x4001828c <gamefile+30100>
call  0x40018330 <gamefile+30264>
call  0x400183d4 <gamefile+30428>
call  0x40021428 <gamefile+67376>
call  0x400184ec <gamefile+30708>
call  0x40021220 <gamefile+66856>
call  0x40018534 <gamefile+30780>
call  0x40021438 <gamefile+67392>
call  0x4001856c <gamefile+30836>
call  0x40018700 <gamefile+31240>
call  0x40018894 <gamefile+31644>
call  0x40021448 <gamefile+67408>
call  0x40018a2c <gamefile+32052>
call  0x40018b10 <gamefile+32280>
call  0x4000fb54 <write+368>
call  0x40021458 <gamefile+67424>
call  0x40018d0c <gamefile+32788>
call  0x40018de0 <gamefile+33000>
call  0x40018eb4 <gamefile+33212>
call  0x40021468 <gamefile+67440>
call  0x40018ffc <gamefile+33540>
call  0x40019150 <gamefile+33880>
call  0x40019314 <gamefile+34332>
call  0x40021478 <gamefile+67456>
call  0x4002147c <gamefile+67460>
call  0x400193e0 <gamefile+34536>
call  0x40021484 <gamefile+67468>
call  0x40021488 <gamefile+67472>
call  0x400195ac <gamefile+34996>
call  0x400196d0 <gamefile+35288>
call  0x40019914 <gamefile+35868>
call  0x40021498 <gamefile+67488>
call  0x40019a3c <gamefile+36164>
call  0x40019b70 <gamefile+36472>
call  0x40019ca4 <gamefile+36780>
call  0x400214a8 <gamefile+67504>
call  0x40019ddc <gamefile+37092>
call  0x40019f30 <gamefile+37432>
call  0x4001a054 <gamefile+37724>
call  0x400214b8 <gamefile+67520>
call  0x4001a08c <gamefile+37780>
call  0x4001a2d0 <gamefile+38360>
call  0x4001a514 <gamefile+38940>
call  0x400214c8 <gamefile+67536>
call  0x4001a75c <gamefile+39524>
call  0x4001a830 <gamefile+39736>
call  0x4000fbd4 <clock+8>
call  0x400214d8 <gamefile+67552>
call  0x4001ab3c <gamefile+40516>
call  0x4001ad10 <gamefile+40984>
call  0x4001aee4 <gamefile+41452>
call  0x400214e8 <gamefile+67568>
call  0x4001b0bc <gamefile+41924>
call  0x4001b130 <gamefile+42040>
call  0x4001b1d4 <gamefile+42204>
call  0x400214f8 <gamefile+67584>
call  0x4001b32c <gamefile+42548>
call  0x4001b650 <gamefile+43352>
call  0x4001b974 <gamefile+44156>
call  0x40021508 <gamefile+67600>
call  0x4001bc9c <gamefile+44964>
call  0x4001be80 <gamefile+45448>
call  0x40021304 <gamefile+67084>
call  0x40021518 <gamefile+67616>
call  0x4001c1bc <gamefile+46276>
call  0x4001c3f0 <gamefile+46840>
call  0x4001c624 <gamefile+47404>
call  0x40021528 <gamefile+67632>
call  0x4001c6ac <gamefile+47540>
call  0x4001c800 <gamefile+47880>
call  0x4001c9e4 <gamefile+48364>
call  0x40021538 <gamefile+67648>
call  0x4001ca2c <gamefile+48436>
call  0x4001cd10 <gamefile+49176>
call  0x4001cff4 <gamefile+49916>
call  0x40021548 <gamefile+67664>
call  0x4001d1bc <gamefile+50372>
call  0x4001d280 <gamefile+50568>
call  0x4000fc54 <gettimeofday+88>
call  0x40021558 <gamefile+67680>
call  0x4002144c <gamefile+67412>
call  0x4001d690 <gamefile+51608>
call  0x4001d8f4 <gamefile+52220>
call  0x40021568 <gamefile+67696>
call  0x4001d99c <gamefile+52388>
call  0x4001da10 <gamefile+52504>
call  0x40021574 <gamefile+67708>
call  0x40021578 <gamefile+67712>
call  0x400214cc <gamefile+67540>
call  0x4001ddf0 <gamefile+53496>
call  0x4001e1c4 <gamefile+54476>
call  0x40021588 <gamefile+67728>
call  0x4001e2dc <gamefile+54756>
call  0x4001e4c0 <gamefile+55240>
call  0x40021384 <gamefile+67212>
call  0x40021598 <gamefile+67744>
call  0x4001e8cc <gamefile+56276>
call  0x4001eb00 <gamefile+56840>
call  0x4001ede4 <gamefile+57580>
call  0x400215a8 <gamefile+67760>
call  0x4001ef9c <gamefile+58020>
call  0x4001f0f0 <gamefile+58360>
call  0x400215b4 <gamefile+67772>
call  0x400215b8 <gamefile+67776>
call  0x4001f3bc <gamefile+59076>
call  0x4001f6a0 <gamefile+59816>
call  0x4001fa54 <gamefile+60764>
call  0x400215c8 <gamefile+67792>
call  0x4001fc1c <gamefile+61220>
call  0x4001fdd0 <gamefile+61656>
call  0x4000fcd4 <cpuinit>
call  0x400215d8 <gamefile+67808>
call  0x400214cc <gamefile+67540>
call  0x400202b0 <gamefile+62904>
call  0x400205c4 <gamefile+63692>
call  0x400215e8 <gamefile+67824>
call  0x4002079c <gamefile+64164>
call  0x40020860 <gamefile+64360>
call  0x400215f4 <gamefile+67836>
call  0x400215f8 <gamefile+67840>
call  0x4002154c <gamefile+67668>
call  0x40020d10 <gamefile+65560>
call  0x400211b4 <gamefile+66748>
