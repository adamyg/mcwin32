;;
;   Module: aesni_3_6_2.obj (objconv'ed mingw32 object)
;
;   Notes:
;
;       o Force memcpy inlining
;
;               #if defined(__GNUC__)
;               #define memcpy(a,b,c) __builtin_memcpy((a),(b),(c)) //XXX
;               #endif
;
;   Functions:
;       int  cdecl mbedtls_aesni_has_support(unsigned int what);
;
;       int  cdecl mbedtls_aesni_crypt_ecb(mbedtls_aes_context *ctx, int mode, const unsigned char input[16], unsigned char output[16]);
;
;       void cdecl mbedtls_aesni_gcm_mult(unsigned char c[16], const unsigned char a[16], const unsigned char b[16]);
;
;       void cdecl mbedtls_aesni_inverse_key(unsigned char *invkey, const unsigned char *fwdkey, int nr);
;
;       int  cdecl mbedtls_aesni_setkey_enc(unsigned char *rk, const unsigned char *key, size_t bits);
;
;   Build Options:
;       uasm32 aesni_3_6_2.asm
;;

option dotname

.686
.xmm
.model flat

public _mbedtls_aesni_has_support
public _mbedtls_aesni_crypt_ecb
public _mbedtls_aesni_gcm_mult
public _mbedtls_aesni_inverse_key
public _mbedtls_aesni_setkey_enc

.bss    SEGMENT DWORD PUBLIC 'BSS'                      ; section number 3
check_done      label dword
        dd      ?                                       ; 0000
check_info0     label dword
        dd      ?                                       ; 0004
check_info1     label dword
        dd      ?                                       ; 0008
check_info2     label dword
        dd      ?                                       ; 000C
check_info3     label dword
        dd      ?                                       ; 0010

check_flag      label dword
        dd      ?                                       ; 0014

.bss    ENDS

.text   SEGMENT DWORD PUBLIC 'CODE'                     ; section number 1

_mbedtls_aesni_has_support LABEL NEAR
        push    ebp                                     ; 0000 _ 55
        mov     ebp, esp                                ; 0001 _ 89. E5
        push    ebx                                     ; 0003 _ 53
        mov     eax, dword ptr [check_done]             ; 0004 _ A1, 00000000(d)
        test    eax, eax                                ; 0009 _ 85. C0
        jnz     ?_001                                   ; 000B _ 75, 40
        mov     eax, 1                                  ; 000D _ B8, 00000001
        mov     ecx, 0                                  ; 0012 _ B9, 00000000
        mov     edx, 0                                  ; 0017 _ BA, 00000000
        mov     ebx, ecx                                ; 001C _ 89. CB
        mov     ecx, edx                                ; 001E _ 89. D1
        cpuid                                           ; 0020 _ 0F A2
        mov     dword ptr [check_info0], eax            ; 0022 _ A3, 00000004(d)
        mov     dword ptr [check_info1], ebx            ; 0027 _ 89. 1D, 00000008(d)
        mov     dword ptr [check_info2], ecx            ; 002D _ 89. 0D, 0000000C(d)
        mov     dword ptr [check_info3], edx            ; 0033 _ 89. 15, 00000010(d)
        mov     eax, dword ptr [check_info2]            ; 0039 _ A1, 0000000C(d)
        mov     dword ptr [check_flag], eax             ; 003E _ A3, 00000014(d)
        mov     dword ptr [check_done], 1               ; 0043 _ C7. 05, 00000000(d), 00000001
?_001:  mov     eax, dword ptr [check_flag]             ; 004D _ A1, 00000014(d)
        and     eax, dword ptr [ebp+8H]                 ; 0052 _ 23. 45, 08
        test    eax, eax                                ; 0055 _ 85. C0
        setne   al                                      ; 0057 _ 0F 95. C0
        movzx   eax, al                                 ; 005A _ 0F B6. C0
        mov     ebx, dword ptr [ebp-4H]                 ; 005D _ 8B. 5D, FC
        leave                                           ; 0060 _ C9
        ret                                             ; 0061 _ C3

_mbedtls_aesni_crypt_ecb PROC NEAR
        push    ebp                                     ; 0062 _ 55
        mov     ebp, esp                                ; 0063 _ 89. E5
        and     esp, 0FFFFFFF0H                         ; 0065 _ 83. E4, F0
        sub     esp, 192                                ; 0068 _ 81. EC, 000000C0
        mov     eax, dword ptr [ebp+8H]                 ; 006E _ 8B. 45, 08
        lea     edx, [eax+8H]                           ; 0071 _ 8D. 50, 08
        mov     eax, dword ptr [ebp+8H]                 ; 0074 _ 8B. 45, 08
        mov     eax, dword ptr [eax+4H]                 ; 0077 _ 8B. 40, 04
        shl     eax, 2                                  ; 007A _ C1. E0, 02
        add     eax, edx                                ; 007D _ 01. D0
        mov     dword ptr [esp+0BCH], eax               ; 007F _ 89. 84 24, 000000BC
        mov     eax, dword ptr [ebp+8H]                 ; 0086 _ 8B. 45, 08
        mov     eax, dword ptr [eax]                    ; 0089 _ 8B. 00
        mov     dword ptr [esp+0B8H], eax               ; 008B _ 89. 84 24, 000000B8
        mov     eax, dword ptr [ebp+10H]                ; 0092 _ 8B. 45, 10
        movdqu  xmm0, xmmword ptr [eax]                 ; 0095 _ F3: 0F 6F. 00
        movups  xmmword ptr [esp], xmm0                 ; 0099 _ 0F 11. 04 24
        mov     eax, dword ptr [esp+0BCH]               ; 009D _ 8B. 84 24, 000000BC
        movdqu  xmm0, xmmword ptr [eax]                 ; 00A4 _ F3: 0F 6F. 00
        movdqu  xmm1, xmmword ptr [esp]                 ; 00A8 _ F3: 0F 6F. 0C 24
        movups  xmmword ptr [esp+0A0H], xmm1            ; 00AD _ 0F 11. 8C 24, 000000A0
        movups  xmmword ptr [esp+90H], xmm0             ; 00B5 _ 0F 11. 84 24, 00000090
        movdqu  xmm1, xmmword ptr [esp+0A0H]            ; 00BD _ F3: 0F 6F. 8C 24, 000000A0
        movdqu  xmm0, xmmword ptr [esp+90H]             ; 00C6 _ F3: 0F 6F. 84 24, 00000090
        pxor    xmm0, xmm1                              ; 00CF _ 66: 0F EF. C1
        movups  xmmword ptr [esp], xmm0                 ; 00D3 _ 0F 11. 04 24
        add     dword ptr [esp+0BCH], 16                ; 00D7 _ 83. 84 24, 000000BC, 10
        sub     dword ptr [esp+0B8H], 1                 ; 00DF _ 83. AC 24, 000000B8, 01
        cmp     dword ptr [ebp+0CH], 0                  ; 00E7 _ 83. 7D, 0C, 00
        jne     ?_005                                   ; 00EB _ 0F 85, 000000B8
        jmp     ?_003                                   ; 00F1 _ EB, 42

?_002:  mov     eax, dword ptr [esp+0BCH]               ; 00F3 _ 8B. 84 24, 000000BC
        movdqu  xmm0, xmmword ptr [eax]                 ; 00FA _ F3: 0F 6F. 00
        movdqu  xmm1, xmmword ptr [esp]                 ; 00FE _ F3: 0F 6F. 0C 24
        movups  xmmword ptr [esp+80H], xmm1             ; 0103 _ 0F 11. 8C 24, 00000080
        movups  xmmword ptr [esp+70H], xmm0             ; 010B _ 0F 11. 44 24, 70
        movdqu  xmm0, xmmword ptr [esp+80H]             ; 0110 _ F3: 0F 6F. 84 24, 00000080
        aesdec  xmm0, xmmword ptr [esp+70H]             ; 0119 _ 66: 0F 38 DE. 44 24, 70
        nop                                             ; 0120 _ 90
        movups  xmmword ptr [esp], xmm0                 ; 0121 _ 0F 11. 04 24
        add     dword ptr [esp+0BCH], 16                ; 0125 _ 83. 84 24, 000000BC, 10
        sub     dword ptr [esp+0B8H], 1                 ; 012D _ 83. AC 24, 000000B8, 01
?_003:  cmp     dword ptr [esp+0B8H], 0                 ; 0135 _ 83. BC 24, 000000B8, 00
        jnz     ?_002                                   ; 013D _ 75, B4
        mov     eax, dword ptr [esp+0BCH]               ; 013F _ 8B. 84 24, 000000BC
        movdqu  xmm0, xmmword ptr [eax]                 ; 0146 _ F3: 0F 6F. 00
        movdqu  xmm1, xmmword ptr [esp]                 ; 014A _ F3: 0F 6F. 0C 24
        movups  xmmword ptr [esp+60H], xmm1             ; 014F _ 0F 11. 4C 24, 60
        movups  xmmword ptr [esp+50H], xmm0             ; 0154 _ 0F 11. 44 24, 50
        movdqu  xmm0, xmmword ptr [esp+60H]             ; 0159 _ F3: 0F 6F. 44 24, 60
        aesdeclast xmm0, xmmword ptr [esp+50H]          ; 015F _ 66: 0F 38 DF. 44 24, 50
        nop                                             ; 0166 _ 90
        movups  xmmword ptr [esp], xmm0                 ; 0167 _ 0F 11. 04 24
        jmp     ?_006                                   ; 016B _ EB, 72

?_004:  mov     eax, dword ptr [esp+0BCH]               ; 016D _ 8B. 84 24, 000000BC
        movdqu  xmm0, xmmword ptr [eax]                 ; 0174 _ F3: 0F 6F. 00
        movdqu  xmm1, xmmword ptr [esp]                 ; 0178 _ F3: 0F 6F. 0C 24
        movups  xmmword ptr [esp+40H], xmm1             ; 017D _ 0F 11. 4C 24, 40
        movups  xmmword ptr [esp+30H], xmm0             ; 0182 _ 0F 11. 44 24, 30
        movdqu  xmm0, xmmword ptr [esp+40H]             ; 0187 _ F3: 0F 6F. 44 24, 40
        aesenc  xmm0, xmmword ptr [esp+30H]             ; 018D _ 66: 0F 38 DC. 44 24, 30
        nop                                             ; 0194 _ 90
        movups  xmmword ptr [esp], xmm0                 ; 0195 _ 0F 11. 04 24
        add     dword ptr [esp+0BCH], 16                ; 0199 _ 83. 84 24, 000000BC, 10
        sub     dword ptr [esp+0B8H], 1                 ; 01A1 _ 83. AC 24, 000000B8, 01
?_005:  cmp     dword ptr [esp+0B8H], 0                 ; 01A9 _ 83. BC 24, 000000B8, 00
        jnz     ?_004                                   ; 01B1 _ 75, BA
        mov     eax, dword ptr [esp+0BCH]               ; 01B3 _ 8B. 84 24, 000000BC
        movdqu  xmm0, xmmword ptr [eax]                 ; 01BA _ F3: 0F 6F. 00
        movdqu  xmm1, xmmword ptr [esp]                 ; 01BE _ F3: 0F 6F. 0C 24
        movups  xmmword ptr [esp+20H], xmm1             ; 01C3 _ 0F 11. 4C 24, 20
        movups  xmmword ptr [esp+10H], xmm0             ; 01C8 _ 0F 11. 44 24, 10
        movdqu  xmm0, xmmword ptr [esp+20H]             ; 01CD _ F3: 0F 6F. 44 24, 20
        aesenclast xmm0, xmmword ptr [esp+10H]          ; 01D3 _ 66: 0F 38 DD. 44 24, 10
        nop                                             ; 01DA _ 90
        movups  xmmword ptr [esp], xmm0                 ; 01DB _ 0F 11. 04 24
?_006:  movdqu  xmm0, xmmword ptr [esp]                 ; 01DF _ F3: 0F 6F. 04 24
        mov     eax, dword ptr [ebp+14H]                ; 01E4 _ 8B. 45, 14
        movups  xmmword ptr [eax], xmm0                 ; 01E7 _ 0F 11. 00
        mov     eax, 0                                  ; 01EA _ B8, 00000000
        leave                                           ; 01EF _ C9
        ret                                             ; 01F0 _ C3
_mbedtls_aesni_crypt_ecb ENDP

_gcm_clmul LABEL NEAR
        push    ebp                                     ; 01F1 _ 55
        mov     ebp, esp                                ; 01F2 _ 89. E5
        and     esp, 0FFFFFFF0H                         ; 01F4 _ 83. E4, F0
        sub     esp, 160                                ; 01F7 _ 81. EC, 000000A0
        movups  xmmword ptr [esp+10H], xmm0             ; 01FD _ 0F 11. 44 24, 10
        movups  xmmword ptr [esp], xmm1                 ; 0202 _ 0F 11. 0C 24
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 0206 _ F3: 0F 6F. 44 24, 10
        movdqu  xmm1, xmmword ptr [esp]                 ; 020C _ F3: 0F 6F. 0C 24
        pclmulqdq xmm0, xmm1, 00H                       ; 0211 _ 66: 0F 3A 44. C1, 00
        mov     eax, dword ptr [ebp+8H]                 ; 0217 _ 8B. 45, 08
        movups  xmmword ptr [eax], xmm0                 ; 021A _ 0F 11. 00
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 021D _ F3: 0F 6F. 44 24, 10
        movdqu  xmm1, xmmword ptr [esp]                 ; 0223 _ F3: 0F 6F. 0C 24
        pclmulqdq xmm0, xmm1, 11H                       ; 0228 _ 66: 0F 3A 44. C1, 11
        mov     eax, dword ptr [ebp+0CH]                ; 022E _ 8B. 45, 0C
        movups  xmmword ptr [eax], xmm0                 ; 0231 _ 0F 11. 00
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 0234 _ F3: 0F 6F. 44 24, 10
        movdqu  xmm1, xmmword ptr [esp]                 ; 023A _ F3: 0F 6F. 0C 24
        pclmulqdq xmm0, xmm1, 10H                       ; 023F _ 66: 0F 3A 44. C1, 10
        movups  xmmword ptr [esp+90H], xmm0             ; 0245 _ 0F 11. 84 24, 00000090
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 024D _ F3: 0F 6F. 44 24, 10
        movdqu  xmm1, xmmword ptr [esp]                 ; 0253 _ F3: 0F 6F. 0C 24
        pclmulqdq xmm0, xmm1, 01H                       ; 0258 _ 66: 0F 3A 44. C1, 01
        movups  xmmword ptr [esp+80H], xmm0             ; 025E _ 0F 11. 84 24, 00000080
        movdqu  xmm0, xmmword ptr [esp+80H]             ; 0266 _ F3: 0F 6F. 84 24, 00000080
        movups  xmmword ptr [esp+30H], xmm0             ; 026F _ 0F 11. 44 24, 30
        movdqu  xmm0, xmmword ptr [esp+90H]             ; 0274 _ F3: 0F 6F. 84 24, 00000090
        movups  xmmword ptr [esp+20H], xmm0             ; 027D _ 0F 11. 44 24, 20
        movdqu  xmm1, xmmword ptr [esp+30H]             ; 0282 _ F3: 0F 6F. 4C 24, 30
        movdqu  xmm0, xmmword ptr [esp+20H]             ; 0288 _ F3: 0F 6F. 44 24, 20
        pxor    xmm0, xmm1                              ; 028E _ 66: 0F EF. C1
        movups  xmmword ptr [esp+80H], xmm0             ; 0292 _ 0F 11. 84 24, 00000080
        movdqu  xmm0, xmmword ptr [esp+80H]             ; 029A _ F3: 0F 6F. 84 24, 00000080
        movups  xmmword ptr [esp+90H], xmm0             ; 02A3 _ 0F 11. 84 24, 00000090
        movdqu  xmm0, xmmword ptr [esp+80H]             ; 02AB _ F3: 0F 6F. 84 24, 00000080
        psrldq  xmm0, 8                                 ; 02B4 _ 66: 0F 73. D8, 08
        movups  xmmword ptr [esp+80H], xmm0             ; 02B9 _ 0F 11. 84 24, 00000080
        movdqu  xmm0, xmmword ptr [esp+90H]             ; 02C1 _ F3: 0F 6F. 84 24, 00000090
        pslldq  xmm0, 8                                 ; 02CA _ 66: 0F 73. F8, 08
        movups  xmmword ptr [esp+90H], xmm0             ; 02CF _ 0F 11. 84 24, 00000090
        mov     eax, dword ptr [ebp+0CH]                ; 02D7 _ 8B. 45, 0C
        movdqu  xmm0, xmmword ptr [eax]                 ; 02DA _ F3: 0F 6F. 00
        movups  xmmword ptr [esp+50H], xmm0             ; 02DE _ 0F 11. 44 24, 50
        movdqu  xmm0, xmmword ptr [esp+80H]             ; 02E3 _ F3: 0F 6F. 84 24, 00000080
        movups  xmmword ptr [esp+40H], xmm0             ; 02EC _ 0F 11. 44 24, 40
        movdqu  xmm1, xmmword ptr [esp+50H]             ; 02F1 _ F3: 0F 6F. 4C 24, 50
        movdqu  xmm0, xmmword ptr [esp+40H]             ; 02F7 _ F3: 0F 6F. 44 24, 40
        pxor    xmm0, xmm1                              ; 02FD _ 66: 0F EF. C1
        mov     eax, dword ptr [ebp+0CH]                ; 0301 _ 8B. 45, 0C
        movups  xmmword ptr [eax], xmm0                 ; 0304 _ 0F 11. 00
        mov     eax, dword ptr [ebp+8H]                 ; 0307 _ 8B. 45, 08
        movdqu  xmm0, xmmword ptr [eax]                 ; 030A _ F3: 0F 6F. 00
        movups  xmmword ptr [esp+70H], xmm0             ; 030E _ 0F 11. 44 24, 70
        movdqu  xmm0, xmmword ptr [esp+90H]             ; 0313 _ F3: 0F 6F. 84 24, 00000090
        movups  xmmword ptr [esp+60H], xmm0             ; 031C _ 0F 11. 44 24, 60
        movdqu  xmm1, xmmword ptr [esp+70H]             ; 0321 _ F3: 0F 6F. 4C 24, 70
        movdqu  xmm0, xmmword ptr [esp+60H]             ; 0327 _ F3: 0F 6F. 44 24, 60
        pxor    xmm0, xmm1                              ; 032D _ 66: 0F EF. C1
        mov     eax, dword ptr [ebp+8H]                 ; 0331 _ 8B. 45, 08
        movups  xmmword ptr [eax], xmm0                 ; 0334 _ 0F 11. 00
        nop                                             ; 0337 _ 90
        leave                                           ; 0338 _ C9
        ret                                             ; 0339 _ C3

_gcm_shift LABEL NEAR
        push    ebp                                     ; 033A _ 55
        mov     ebp, esp                                ; 033B _ 89. E5
        and     esp, 0FFFFFFF0H                         ; 033D _ 83. E4, F0
        sub     esp, 304                                ; 0340 _ 81. EC, 00000130
        mov     eax, dword ptr [ebp+8H]                 ; 0346 _ 8B. 45, 08
        movdqu  xmm0, xmmword ptr [eax]                 ; 0349 _ F3: 0F 6F. 00
        movups  xmmword ptr [esp+10H], xmm0             ; 034D _ 0F 11. 44 24, 10
        mov     dword ptr [esp+0CH], 1                  ; 0352 _ C7. 44 24, 0C, 00000001
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 035A _ F3: 0F 6F. 44 24, 10
        movd    xmm1, dword ptr [esp+0CH]               ; 0360 _ 66: 0F 6E. 4C 24, 0C
        psllq   xmm0, xmm1                              ; 0366 _ 66: 0F F3. C1
        nop                                             ; 036A _ 90
        movups  xmmword ptr [esp+120H], xmm0            ; 036B _ 0F 11. 84 24, 00000120
        mov     eax, dword ptr [ebp+0CH]                ; 0373 _ 8B. 45, 0C
        movdqu  xmm0, xmmword ptr [eax]                 ; 0376 _ F3: 0F 6F. 00
        movups  xmmword ptr [esp+30H], xmm0             ; 037A _ 0F 11. 44 24, 30
        mov     dword ptr [esp+2CH], 1                  ; 037F _ C7. 44 24, 2C, 00000001
        movdqu  xmm0, xmmword ptr [esp+30H]             ; 0387 _ F3: 0F 6F. 44 24, 30
        movd    xmm1, dword ptr [esp+2CH]               ; 038D _ 66: 0F 6E. 4C 24, 2C
        psllq   xmm0, xmm1                              ; 0393 _ 66: 0F F3. C1
        nop                                             ; 0397 _ 90
        movups  xmmword ptr [esp+110H], xmm0            ; 0398 _ 0F 11. 84 24, 00000110
        mov     eax, dword ptr [ebp+8H]                 ; 03A0 _ 8B. 45, 08
        movdqu  xmm0, xmmword ptr [eax]                 ; 03A3 _ F3: 0F 6F. 00
        movups  xmmword ptr [esp+50H], xmm0             ; 03A7 _ 0F 11. 44 24, 50
        mov     dword ptr [esp+4CH], 63                 ; 03AC _ C7. 44 24, 4C, 0000003F
        movdqu  xmm0, xmmword ptr [esp+50H]             ; 03B4 _ F3: 0F 6F. 44 24, 50
        movd    xmm1, dword ptr [esp+4CH]               ; 03BA _ 66: 0F 6E. 4C 24, 4C
        psrlq   xmm0, xmm1                              ; 03C0 _ 66: 0F D3. C1
        nop                                             ; 03C4 _ 90
        movups  xmmword ptr [esp+100H], xmm0            ; 03C5 _ 0F 11. 84 24, 00000100
        mov     eax, dword ptr [ebp+0CH]                ; 03CD _ 8B. 45, 0C
        movdqu  xmm0, xmmword ptr [eax]                 ; 03D0 _ F3: 0F 6F. 00
        movups  xmmword ptr [esp+70H], xmm0             ; 03D4 _ 0F 11. 44 24, 70
        mov     dword ptr [esp+6CH], 63                 ; 03D9 _ C7. 44 24, 6C, 0000003F
        movdqu  xmm0, xmmword ptr [esp+70H]             ; 03E1 _ F3: 0F 6F. 44 24, 70
        movd    xmm1, dword ptr [esp+6CH]               ; 03E7 _ 66: 0F 6E. 4C 24, 6C
        psrlq   xmm0, xmm1                              ; 03ED _ 66: 0F D3. C1
        nop                                             ; 03F1 _ 90
        movups  xmmword ptr [esp+0F0H], xmm0            ; 03F2 _ 0F 11. 84 24, 000000F0
        movdqu  xmm0, xmmword ptr [esp+100H]            ; 03FA _ F3: 0F 6F. 84 24, 00000100
        psrldq  xmm0, 8                                 ; 0403 _ 66: 0F 73. D8, 08
        movups  xmmword ptr [esp+0E0H], xmm0            ; 0408 _ 0F 11. 84 24, 000000E0
        movdqu  xmm0, xmmword ptr [esp+100H]            ; 0410 _ F3: 0F 6F. 84 24, 00000100
        pslldq  xmm0, 8                                 ; 0419 _ 66: 0F 73. F8, 08
        movups  xmmword ptr [esp+100H], xmm0            ; 041E _ 0F 11. 84 24, 00000100
        movdqu  xmm0, xmmword ptr [esp+0F0H]            ; 0426 _ F3: 0F 6F. 84 24, 000000F0
        pslldq  xmm0, 8                                 ; 042F _ 66: 0F 73. F8, 08
        movups  xmmword ptr [esp+0F0H], xmm0            ; 0434 _ 0F 11. 84 24, 000000F0
        movdqu  xmm0, xmmword ptr [esp+120H]            ; 043C _ F3: 0F 6F. 84 24, 00000120
        movups  xmmword ptr [esp+90H], xmm0             ; 0445 _ 0F 11. 84 24, 00000090
        movdqu  xmm0, xmmword ptr [esp+100H]            ; 044D _ F3: 0F 6F. 84 24, 00000100
        movups  xmmword ptr [esp+80H], xmm0             ; 0456 _ 0F 11. 84 24, 00000080
        movdqu  xmm1, xmmword ptr [esp+90H]             ; 045E _ F3: 0F 6F. 8C 24, 00000090
        movdqu  xmm0, xmmword ptr [esp+80H]             ; 0467 _ F3: 0F 6F. 84 24, 00000080
        por     xmm0, xmm1                              ; 0470 _ 66: 0F EB. C1
        mov     eax, dword ptr [ebp+8H]                 ; 0474 _ 8B. 45, 08
        movups  xmmword ptr [eax], xmm0                 ; 0477 _ 0F 11. 00
        movdqu  xmm0, xmmword ptr [esp+110H]            ; 047A _ F3: 0F 6F. 84 24, 00000110
        movups  xmmword ptr [esp+0B0H], xmm0            ; 0483 _ 0F 11. 84 24, 000000B0
        movdqu  xmm0, xmmword ptr [esp+0F0H]            ; 048B _ F3: 0F 6F. 84 24, 000000F0
        movups  xmmword ptr [esp+0A0H], xmm0            ; 0494 _ 0F 11. 84 24, 000000A0
        movdqu  xmm1, xmmword ptr [esp+0B0H]            ; 049C _ F3: 0F 6F. 8C 24, 000000B0
        movdqu  xmm0, xmmword ptr [esp+0A0H]            ; 04A5 _ F3: 0F 6F. 84 24, 000000A0
        por     xmm0, xmm1                              ; 04AE _ 66: 0F EB. C1
        movups  xmmword ptr [esp+0D0H], xmm0            ; 04B2 _ 0F 11. 84 24, 000000D0
        movdqu  xmm0, xmmword ptr [esp+0E0H]            ; 04BA _ F3: 0F 6F. 84 24, 000000E0
        movups  xmmword ptr [esp+0C0H], xmm0            ; 04C3 _ 0F 11. 84 24, 000000C0
        movdqu  xmm1, xmmword ptr [esp+0D0H]            ; 04CB _ F3: 0F 6F. 8C 24, 000000D0
        movdqu  xmm0, xmmword ptr [esp+0C0H]            ; 04D4 _ F3: 0F 6F. 84 24, 000000C0
        por     xmm0, xmm1                              ; 04DD _ 66: 0F EB. C1
        mov     eax, dword ptr [ebp+0CH]                ; 04E1 _ 8B. 45, 0C
        movups  xmmword ptr [eax], xmm0                 ; 04E4 _ 0F 11. 00
        nop                                             ; 04E7 _ 90
        leave                                           ; 04E8 _ C9
        ret                                             ; 04E9 _ C3

_gcm_reduce LABEL NEAR
        push    ebp                                     ; 04EA _ 55
        mov     ebp, esp                                ; 04EB _ 89. E5
        and     esp, 0FFFFFFF0H                         ; 04ED _ 83. E4, F0
        sub     esp, 272                                ; 04F0 _ 81. EC, 00000110
        movups  xmmword ptr [esp], xmm0                 ; 04F6 _ 0F 11. 04 24
        movdqu  xmm0, xmmword ptr [esp]                 ; 04FA _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+20H], xmm0             ; 04FF _ 0F 11. 44 24, 20
        mov     dword ptr [esp+1CH], 63                 ; 0504 _ C7. 44 24, 1C, 0000003F
        movdqu  xmm0, xmmword ptr [esp+20H]             ; 050C _ F3: 0F 6F. 44 24, 20
        movd    xmm1, dword ptr [esp+1CH]               ; 0512 _ 66: 0F 6E. 4C 24, 1C
        psllq   xmm0, xmm1                              ; 0518 _ 66: 0F F3. C1
        nop                                             ; 051C _ 90
        movups  xmmword ptr [esp+100H], xmm0            ; 051D _ 0F 11. 84 24, 00000100
        movdqu  xmm0, xmmword ptr [esp]                 ; 0525 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+40H], xmm0             ; 052A _ 0F 11. 44 24, 40
        mov     dword ptr [esp+3CH], 62                 ; 052F _ C7. 44 24, 3C, 0000003E
        movdqu  xmm0, xmmword ptr [esp+40H]             ; 0537 _ F3: 0F 6F. 44 24, 40
        movd    xmm1, dword ptr [esp+3CH]               ; 053D _ 66: 0F 6E. 4C 24, 3C
        psllq   xmm0, xmm1                              ; 0543 _ 66: 0F F3. C1
        nop                                             ; 0547 _ 90
        movups  xmmword ptr [esp+0F0H], xmm0            ; 0548 _ 0F 11. 84 24, 000000F0
        movdqu  xmm0, xmmword ptr [esp]                 ; 0550 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+60H], xmm0             ; 0555 _ 0F 11. 44 24, 60
        mov     dword ptr [esp+5CH], 57                 ; 055A _ C7. 44 24, 5C, 00000039
        movdqu  xmm0, xmmword ptr [esp+60H]             ; 0562 _ F3: 0F 6F. 44 24, 60
        movd    xmm1, dword ptr [esp+5CH]               ; 0568 _ 66: 0F 6E. 4C 24, 5C
        psllq   xmm0, xmm1                              ; 056E _ 66: 0F F3. C1
        nop                                             ; 0572 _ 90
        movups  xmmword ptr [esp+0E0H], xmm0            ; 0573 _ 0F 11. 84 24, 000000E0
        movdqu  xmm0, xmmword ptr [esp+100H]            ; 057B _ F3: 0F 6F. 84 24, 00000100
        movups  xmmword ptr [esp+80H], xmm0             ; 0584 _ 0F 11. 84 24, 00000080
        movdqu  xmm0, xmmword ptr [esp+0F0H]            ; 058C _ F3: 0F 6F. 84 24, 000000F0
        movups  xmmword ptr [esp+70H], xmm0             ; 0595 _ 0F 11. 44 24, 70
        movdqu  xmm1, xmmword ptr [esp+80H]             ; 059A _ F3: 0F 6F. 8C 24, 00000080
        movdqu  xmm0, xmmword ptr [esp+70H]             ; 05A3 _ F3: 0F 6F. 44 24, 70
        pxor    xmm0, xmm1                              ; 05A9 _ 66: 0F EF. C1
        movups  xmmword ptr [esp+0A0H], xmm0            ; 05AD _ 0F 11. 84 24, 000000A0
        movdqu  xmm0, xmmword ptr [esp+0E0H]            ; 05B5 _ F3: 0F 6F. 84 24, 000000E0
        movups  xmmword ptr [esp+90H], xmm0             ; 05BE _ 0F 11. 84 24, 00000090
        movdqu  xmm1, xmmword ptr [esp+0A0H]            ; 05C6 _ F3: 0F 6F. 8C 24, 000000A0
        movdqu  xmm0, xmmword ptr [esp+90H]             ; 05CF _ F3: 0F 6F. 84 24, 00000090
        pxor    xmm0, xmm1                              ; 05D8 _ 66: 0F EF. C1
        pslldq  xmm0, 8                                 ; 05DC _ 66: 0F 73. F8, 08
        movups  xmmword ptr [esp+0D0H], xmm0            ; 05E1 _ 0F 11. 84 24, 000000D0
        movdqu  xmm0, xmmword ptr [esp+0D0H]            ; 05E9 _ F3: 0F 6F. 84 24, 000000D0
        movups  xmmword ptr [esp+0C0H], xmm0            ; 05F2 _ 0F 11. 84 24, 000000C0
        movdqu  xmm0, xmmword ptr [esp]                 ; 05FA _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+0B0H], xmm0            ; 05FF _ 0F 11. 84 24, 000000B0
        movdqu  xmm1, xmmword ptr [esp+0C0H]            ; 0607 _ F3: 0F 6F. 8C 24, 000000C0
        movdqu  xmm0, xmmword ptr [esp+0B0H]            ; 0610 _ F3: 0F 6F. 84 24, 000000B0
        pxor    xmm0, xmm1                              ; 0619 _ 66: 0F EF. C1
        leave                                           ; 061D _ C9
        ret                                             ; 061E _ C3

_gcm_mix LABEL NEAR
        push    ebp                                     ; 061F _ 55
        mov     ebp, esp                                ; 0620 _ 89. E5
        and     esp, 0FFFFFFF0H                         ; 0622 _ 83. E4, F0
        sub     esp, 512                                ; 0625 _ 81. EC, 00000200
        movups  xmmword ptr [esp], xmm0                 ; 062B _ 0F 11. 04 24
        movdqu  xmm0, xmmword ptr [esp]                 ; 062F _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+20H], xmm0             ; 0634 _ 0F 11. 44 24, 20
        mov     dword ptr [esp+1CH], 1                  ; 0639 _ C7. 44 24, 1C, 00000001
        movdqu  xmm0, xmmword ptr [esp+20H]             ; 0641 _ F3: 0F 6F. 44 24, 20
        movd    xmm1, dword ptr [esp+1CH]               ; 0647 _ 66: 0F 6E. 4C 24, 1C
        psrlq   xmm0, xmm1                              ; 064D _ 66: 0F D3. C1
        nop                                             ; 0651 _ 90
        movups  xmmword ptr [esp+1F0H], xmm0            ; 0652 _ 0F 11. 84 24, 000001F0
        movdqu  xmm0, xmmword ptr [esp]                 ; 065A _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+40H], xmm0             ; 065F _ 0F 11. 44 24, 40
        mov     dword ptr [esp+3CH], 2                  ; 0664 _ C7. 44 24, 3C, 00000002
        movdqu  xmm0, xmmword ptr [esp+40H]             ; 066C _ F3: 0F 6F. 44 24, 40
        movd    xmm1, dword ptr [esp+3CH]               ; 0672 _ 66: 0F 6E. 4C 24, 3C
        psrlq   xmm0, xmm1                              ; 0678 _ 66: 0F D3. C1
        nop                                             ; 067C _ 90
        movups  xmmword ptr [esp+1E0H], xmm0            ; 067D _ 0F 11. 84 24, 000001E0
        movdqu  xmm0, xmmword ptr [esp]                 ; 0685 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+60H], xmm0             ; 068A _ 0F 11. 44 24, 60
        mov     dword ptr [esp+5CH], 7                  ; 068F _ C7. 44 24, 5C, 00000007
        movdqu  xmm0, xmmword ptr [esp+60H]             ; 0697 _ F3: 0F 6F. 44 24, 60
        movd    xmm1, dword ptr [esp+5CH]               ; 069D _ 66: 0F 6E. 4C 24, 5C
        psrlq   xmm0, xmm1                              ; 06A3 _ 66: 0F D3. C1
        nop                                             ; 06A7 _ 90
        movups  xmmword ptr [esp+1D0H], xmm0            ; 06A8 _ 0F 11. 84 24, 000001D0
        movdqu  xmm0, xmmword ptr [esp]                 ; 06B0 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+80H], xmm0             ; 06B5 _ 0F 11. 84 24, 00000080
        mov     dword ptr [esp+7CH], 63                 ; 06BD _ C7. 44 24, 7C, 0000003F
        movdqu  xmm0, xmmword ptr [esp+80H]             ; 06C5 _ F3: 0F 6F. 84 24, 00000080
        movd    xmm1, dword ptr [esp+7CH]               ; 06CE _ 66: 0F 6E. 4C 24, 7C
        psllq   xmm0, xmm1                              ; 06D4 _ 66: 0F F3. C1
        nop                                             ; 06D8 _ 90
        movups  xmmword ptr [esp+1C0H], xmm0            ; 06D9 _ 0F 11. 84 24, 000001C0
        movdqu  xmm0, xmmword ptr [esp]                 ; 06E1 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+0A0H], xmm0            ; 06E6 _ 0F 11. 84 24, 000000A0
        mov     dword ptr [esp+9CH], 62                 ; 06EE _ C7. 84 24, 0000009C, 0000003E
        movdqu  xmm0, xmmword ptr [esp+0A0H]            ; 06F9 _ F3: 0F 6F. 84 24, 000000A0
        movd    xmm1, dword ptr [esp+9CH]               ; 0702 _ 66: 0F 6E. 8C 24, 0000009C
        psllq   xmm0, xmm1                              ; 070B _ 66: 0F F3. C1
        nop                                             ; 070F _ 90
        movups  xmmword ptr [esp+1B0H], xmm0            ; 0710 _ 0F 11. 84 24, 000001B0
        movdqu  xmm0, xmmword ptr [esp]                 ; 0718 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+0C0H], xmm0            ; 071D _ 0F 11. 84 24, 000000C0
        mov     dword ptr [esp+0BCH], 57                ; 0725 _ C7. 84 24, 000000BC, 00000039
        movdqu  xmm0, xmmword ptr [esp+0C0H]            ; 0730 _ F3: 0F 6F. 84 24, 000000C0
        movd    xmm1, dword ptr [esp+0BCH]              ; 0739 _ 66: 0F 6E. 8C 24, 000000BC
        psllq   xmm0, xmm1                              ; 0742 _ 66: 0F F3. C1
        nop                                             ; 0746 _ 90
        movups  xmmword ptr [esp+1A0H], xmm0            ; 0747 _ 0F 11. 84 24, 000001A0
        movdqu  xmm0, xmmword ptr [esp+1C0H]            ; 074F _ F3: 0F 6F. 84 24, 000001C0
        movups  xmmword ptr [esp+0E0H], xmm0            ; 0758 _ 0F 11. 84 24, 000000E0
        movdqu  xmm0, xmmword ptr [esp+1B0H]            ; 0760 _ F3: 0F 6F. 84 24, 000001B0
        movups  xmmword ptr [esp+0D0H], xmm0            ; 0769 _ 0F 11. 84 24, 000000D0
        movdqu  xmm1, xmmword ptr [esp+0E0H]            ; 0771 _ F3: 0F 6F. 8C 24, 000000E0
        movdqu  xmm0, xmmword ptr [esp+0D0H]            ; 077A _ F3: 0F 6F. 84 24, 000000D0
        pxor    xmm0, xmm1                              ; 0783 _ 66: 0F EF. C1
        movups  xmmword ptr [esp+100H], xmm0            ; 0787 _ 0F 11. 84 24, 00000100
        movdqu  xmm0, xmmword ptr [esp+1A0H]            ; 078F _ F3: 0F 6F. 84 24, 000001A0
        movups  xmmword ptr [esp+0F0H], xmm0            ; 0798 _ 0F 11. 84 24, 000000F0
        movdqu  xmm1, xmmword ptr [esp+100H]            ; 07A0 _ F3: 0F 6F. 8C 24, 00000100
        movdqu  xmm0, xmmword ptr [esp+0F0H]            ; 07A9 _ F3: 0F 6F. 84 24, 000000F0
        pxor    xmm0, xmm1                              ; 07B2 _ 66: 0F EF. C1
        psrldq  xmm0, 8                                 ; 07B6 _ 66: 0F 73. D8, 08
        movups  xmmword ptr [esp+190H], xmm0            ; 07BB _ 0F 11. 84 24, 00000190
        movdqu  xmm0, xmmword ptr [esp+1F0H]            ; 07C3 _ F3: 0F 6F. 84 24, 000001F0
        movups  xmmword ptr [esp+120H], xmm0            ; 07CC _ 0F 11. 84 24, 00000120
        movdqu  xmm0, xmmword ptr [esp+1E0H]            ; 07D4 _ F3: 0F 6F. 84 24, 000001E0
        movups  xmmword ptr [esp+110H], xmm0            ; 07DD _ 0F 11. 84 24, 00000110
        movdqu  xmm1, xmmword ptr [esp+120H]            ; 07E5 _ F3: 0F 6F. 8C 24, 00000120
        movdqu  xmm0, xmmword ptr [esp+110H]            ; 07EE _ F3: 0F 6F. 84 24, 00000110
        pxor    xmm0, xmm1                              ; 07F7 _ 66: 0F EF. C1
        movups  xmmword ptr [esp+140H], xmm0            ; 07FB _ 0F 11. 84 24, 00000140
        movdqu  xmm0, xmmword ptr [esp+1D0H]            ; 0803 _ F3: 0F 6F. 84 24, 000001D0
        movups  xmmword ptr [esp+130H], xmm0            ; 080C _ 0F 11. 84 24, 00000130
        movdqu  xmm1, xmmword ptr [esp+140H]            ; 0814 _ F3: 0F 6F. 8C 24, 00000140
        movdqu  xmm0, xmmword ptr [esp+130H]            ; 081D _ F3: 0F 6F. 84 24, 00000130
        pxor    xmm0, xmm1                              ; 0826 _ 66: 0F EF. C1
        movups  xmmword ptr [esp+160H], xmm0            ; 082A _ 0F 11. 84 24, 00000160
        movdqu  xmm0, xmmword ptr [esp+190H]            ; 0832 _ F3: 0F 6F. 84 24, 00000190
        movups  xmmword ptr [esp+150H], xmm0            ; 083B _ 0F 11. 84 24, 00000150
        movdqu  xmm1, xmmword ptr [esp+160H]            ; 0843 _ F3: 0F 6F. 8C 24, 00000160
        movdqu  xmm0, xmmword ptr [esp+150H]            ; 084C _ F3: 0F 6F. 84 24, 00000150
        pxor    xmm0, xmm1                              ; 0855 _ 66: 0F EF. C1
        movups  xmmword ptr [esp+180H], xmm0            ; 0859 _ 0F 11. 84 24, 00000180
        movdqu  xmm0, xmmword ptr [esp]                 ; 0861 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+170H], xmm0            ; 0866 _ 0F 11. 84 24, 00000170
        movdqu  xmm1, xmmword ptr [esp+180H]            ; 086E _ F3: 0F 6F. 8C 24, 00000180
        movdqu  xmm0, xmmword ptr [esp+170H]            ; 0877 _ F3: 0F 6F. 84 24, 00000170
        pxor    xmm0, xmm1                              ; 0880 _ 66: 0F EF. C1
        leave                                           ; 0884 _ C9
        ret                                             ; 0885 _ C3

_mbedtls_aesni_gcm_mult PROC NEAR
        push    ebp                                     ; 0886 _ 55
        mov     ebp, esp                                ; 0887 _ 89. E5
        and     esp, 0FFFFFFF0H                         ; 0889 _ 83. E4, F0
        sub     esp, 160                                ; 088C _ 81. EC, 000000A0
        pxor    xmm0, xmm0                              ; 0892 _ 66: 0F EF. C0
        movups  xmmword ptr [esp+40H], xmm0             ; 0896 _ 0F 11. 44 24, 40
        pxor    xmm0, xmm0                              ; 089B _ 66: 0F EF. C0
        movups  xmmword ptr [esp+30H], xmm0             ; 089F _ 0F 11. 44 24, 30
        mov     dword ptr [esp+9CH], 0                  ; 08A4 _ C7. 84 24, 0000009C, 00000000
        jmp     ?_008                                   ; 08AF _ EB, 52

?_007:  mov     eax, 15                                 ; 08B1 _ B8, 0000000F
        sub     eax, dword ptr [esp+9CH]                ; 08B6 _ 2B. 84 24, 0000009C
        mov     edx, eax                                ; 08BD _ 89. C2
        mov     eax, dword ptr [ebp+0CH]                ; 08BF _ 8B. 45, 0C
        add     eax, edx                                ; 08C2 _ 01. D0
        lea     ecx, [esp+40H]                          ; 08C4 _ 8D. 4C 24, 40
        mov     edx, dword ptr [esp+9CH]                ; 08C8 _ 8B. 94 24, 0000009C
        add     edx, ecx                                ; 08CF _ 01. CA
        movzx   eax, byte ptr [eax]                     ; 08D1 _ 0F B6. 00
        mov     byte ptr [edx], al                      ; 08D4 _ 88. 02
        mov     eax, 15                                 ; 08D6 _ B8, 0000000F
        sub     eax, dword ptr [esp+9CH]                ; 08DB _ 2B. 84 24, 0000009C
        mov     edx, eax                                ; 08E2 _ 89. C2
        mov     eax, dword ptr [ebp+10H]                ; 08E4 _ 8B. 45, 10
        add     eax, edx                                ; 08E7 _ 01. D0
        lea     ecx, [esp+30H]                          ; 08E9 _ 8D. 4C 24, 30
        mov     edx, dword ptr [esp+9CH]                ; 08ED _ 8B. 94 24, 0000009C
        add     edx, ecx                                ; 08F4 _ 01. CA
        movzx   eax, byte ptr [eax]                     ; 08F6 _ 0F B6. 00
        mov     byte ptr [edx], al                      ; 08F9 _ 88. 02
        add     dword ptr [esp+9CH], 1                  ; 08FB _ 83. 84 24, 0000009C, 01
?_008:  cmp     dword ptr [esp+9CH], 15                 ; 0903 _ 83. BC 24, 0000009C, 0F
        jbe     ?_007                                   ; 090B _ 76, A4
        movdqu  xmm1, xmmword ptr [esp+30H]             ; 090D _ F3: 0F 6F. 4C 24, 30
        movdqu  xmm0, xmmword ptr [esp+40H]             ; 0913 _ F3: 0F 6F. 44 24, 40
        lea     eax, [esp+10H]                          ; 0919 _ 8D. 44 24, 10
        mov     dword ptr [esp+4H], eax                 ; 091D _ 89. 44 24, 04
        lea     eax, [esp+20H]                          ; 0921 _ 8D. 44 24, 20
        mov     dword ptr [esp], eax                    ; 0925 _ 89. 04 24
        call    _gcm_clmul                              ; 0928 _ E8, FFFFF8C4
        lea     eax, [esp+10H]                          ; 092D _ 8D. 44 24, 10
        mov     dword ptr [esp+4H], eax                 ; 0931 _ 89. 44 24, 04
        lea     eax, [esp+20H]                          ; 0935 _ 8D. 44 24, 20
        mov     dword ptr [esp], eax                    ; 0939 _ 89. 04 24
        call    _gcm_shift                              ; 093C _ E8, FFFFF9F9
        movdqu  xmm0, xmmword ptr [esp+20H]             ; 0941 _ F3: 0F 6F. 44 24, 20
        call    _gcm_reduce                             ; 0947 _ E8, FFFFFB9E
        movups  xmmword ptr [esp+80H], xmm0             ; 094C _ 0F 11. 84 24, 00000080
        movdqu  xmm0, xmmword ptr [esp+80H]             ; 0954 _ F3: 0F 6F. 84 24, 00000080
        call    _gcm_mix                                ; 095D _ E8, FFFFFCBD
        movups  xmmword ptr [esp+70H], xmm0             ; 0962 _ 0F 11. 44 24, 70
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 0967 _ F3: 0F 6F. 44 24, 10
        movdqu  xmm1, xmmword ptr [esp+70H]             ; 096D _ F3: 0F 6F. 4C 24, 70
        movups  xmmword ptr [esp+60H], xmm1             ; 0973 _ 0F 11. 4C 24, 60
        movups  xmmword ptr [esp+50H], xmm0             ; 0978 _ 0F 11. 44 24, 50
        movdqu  xmm1, xmmword ptr [esp+60H]             ; 097D _ F3: 0F 6F. 4C 24, 60
        movdqu  xmm0, xmmword ptr [esp+50H]             ; 0983 _ F3: 0F 6F. 44 24, 50
        pxor    xmm0, xmm1                              ; 0989 _ 66: 0F EF. C1
        movups  xmmword ptr [esp+20H], xmm0             ; 098D _ 0F 11. 44 24, 20
        mov     dword ptr [esp+98H], 0                  ; 0992 _ C7. 84 24, 00000098, 00000000
        jmp     ?_010                                   ; 099D _ EB, 2B

?_009:  mov     eax, 15                                 ; 099F _ B8, 0000000F
        sub     eax, dword ptr [esp+98H]                ; 09A4 _ 2B. 84 24, 00000098
        lea     edx, [esp+20H]                          ; 09AB _ 8D. 54 24, 20
        add     eax, edx                                ; 09AF _ 01. D0
        mov     ecx, dword ptr [ebp+8H]                 ; 09B1 _ 8B. 4D, 08
        mov     edx, dword ptr [esp+98H]                ; 09B4 _ 8B. 94 24, 00000098
        add     edx, ecx                                ; 09BB _ 01. CA
        movzx   eax, byte ptr [eax]                     ; 09BD _ 0F B6. 00
        mov     byte ptr [edx], al                      ; 09C0 _ 88. 02
        add     dword ptr [esp+98H], 1                  ; 09C2 _ 83. 84 24, 00000098, 01
?_010:  cmp     dword ptr [esp+98H], 15                 ; 09CA _ 83. BC 24, 00000098, 0F
        jbe     ?_009                                   ; 09D2 _ 76, CB
        nop                                             ; 09D4 _ 90
        leave                                           ; 09D5 _ C9
        ret                                             ; 09D6 _ C3
_mbedtls_aesni_gcm_mult ENDP

_mbedtls_aesni_inverse_key PROC NEAR
        push    ebp                                     ; 09D7 _ 55
        mov     ebp, esp                                ; 09D8 _ 89. E5
        and     esp, 0FFFFFFF0H                         ; 09DA _ 83. E4, F0
        sub     esp, 32                                 ; 09DD _ 83. EC, 20
        mov     eax, dword ptr [ebp+8H]                 ; 09E0 _ 8B. 45, 08
        mov     dword ptr [esp+1CH], eax                ; 09E3 _ 89. 44 24, 1C
        mov     eax, dword ptr [ebp+10H]                ; 09E7 _ 8B. 45, 10
        shl     eax, 4                                  ; 09EA _ C1. E0, 04
        mov     edx, eax                                ; 09ED _ 89. C2
        mov     eax, dword ptr [ebp+0CH]                ; 09EF _ 8B. 45, 0C
        add     eax, edx                                ; 09F2 _ 01. D0
        mov     dword ptr [esp+18H], eax                ; 09F4 _ 89. 44 24, 18
        mov     eax, dword ptr [esp+18H]                ; 09F8 _ 8B. 44 24, 18
        movdqu  xmm0, xmmword ptr [eax]                 ; 09FC _ F3: 0F 6F. 00
        mov     eax, dword ptr [esp+1CH]                ; 0A00 _ 8B. 44 24, 1C
        movups  xmmword ptr [eax], xmm0                 ; 0A04 _ 0F 11. 00
        sub     dword ptr [esp+18H], 16                 ; 0A07 _ 83. 6C 24, 18, 10
        add     dword ptr [esp+1CH], 16                 ; 0A0C _ 83. 44 24, 1C, 10
        jmp     ?_012                                   ; 0A11 _ EB, 24

?_011:  mov     eax, dword ptr [esp+18H]                ; 0A13 _ 8B. 44 24, 18
        movdqu  xmm0, xmmword ptr [eax]                 ; 0A17 _ F3: 0F 6F. 00
        movups  xmmword ptr [esp], xmm0                 ; 0A1B _ 0F 11. 04 24
        aesimc  xmm0, xmmword ptr [esp]                 ; 0A1F _ 66: 0F 38 DB. 04 24
        nop                                             ; 0A25 _ 90
        mov     eax, dword ptr [esp+1CH]                ; 0A26 _ 8B. 44 24, 1C
        movups  xmmword ptr [eax], xmm0                 ; 0A2A _ 0F 11. 00
        sub     dword ptr [esp+18H], 16                 ; 0A2D _ 83. 6C 24, 18, 10
        add     dword ptr [esp+1CH], 16                 ; 0A32 _ 83. 44 24, 1C, 10
?_012:  mov     eax, dword ptr [esp+18H]                ; 0A37 _ 8B. 44 24, 18
        cmp     dword ptr [ebp+0CH], eax                ; 0A3B _ 39. 45, 0C
        jc      ?_011                                   ; 0A3E _ 72, D3
        mov     eax, dword ptr [esp+18H]                ; 0A40 _ 8B. 44 24, 18
        movdqu  xmm0, xmmword ptr [eax]                 ; 0A44 _ F3: 0F 6F. 00
        mov     eax, dword ptr [esp+1CH]                ; 0A48 _ 8B. 44 24, 1C
        movups  xmmword ptr [eax], xmm0                 ; 0A4C _ 0F 11. 00
        nop                                             ; 0A4F _ 90
        leave                                           ; 0A50 _ C9
        ret                                             ; 0A51 _ C3
_mbedtls_aesni_inverse_key ENDP

_aesni_set_rk_128 LABEL NEAR
        push    ebp                                     ; 0A52 _ 55
        mov     ebp, esp                                ; 0A53 _ 89. E5
        and     esp, 0FFFFFFF0H                         ; 0A55 _ 83. E4, F0
        sub     esp, 160                                ; 0A58 _ 81. EC, 000000A0
        movups  xmmword ptr [esp+10H], xmm0             ; 0A5E _ 0F 11. 44 24, 10
        movups  xmmword ptr [esp], xmm1                 ; 0A63 _ 0F 11. 0C 24
        movdqu  xmm0, xmmword ptr [esp]                 ; 0A67 _ F3: 0F 6F. 04 24
        pshufd  xmm0, xmm0, 0FFH                        ; 0A6C _ 66: 0F 70. C0, FF
        movups  xmmword ptr [esp], xmm0                 ; 0A71 _ 0F 11. 04 24
        movdqu  xmm0, xmmword ptr [esp]                 ; 0A75 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+30H], xmm0             ; 0A7A _ 0F 11. 44 24, 30
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 0A7F _ F3: 0F 6F. 44 24, 10
        movups  xmmword ptr [esp+20H], xmm0             ; 0A85 _ 0F 11. 44 24, 20
        movdqu  xmm1, xmmword ptr [esp+30H]             ; 0A8A _ F3: 0F 6F. 4C 24, 30
        movdqu  xmm0, xmmword ptr [esp+20H]             ; 0A90 _ F3: 0F 6F. 44 24, 20
        pxor    xmm0, xmm1                              ; 0A96 _ 66: 0F EF. C1
        movups  xmmword ptr [esp], xmm0                 ; 0A9A _ 0F 11. 04 24
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 0A9E _ F3: 0F 6F. 44 24, 10
        pslldq  xmm0, 4                                 ; 0AA4 _ 66: 0F 73. F8, 04
        movups  xmmword ptr [esp+10H], xmm0             ; 0AA9 _ 0F 11. 44 24, 10
        movdqu  xmm0, xmmword ptr [esp]                 ; 0AAE _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+50H], xmm0             ; 0AB3 _ 0F 11. 44 24, 50
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 0AB8 _ F3: 0F 6F. 44 24, 10
        movups  xmmword ptr [esp+40H], xmm0             ; 0ABE _ 0F 11. 44 24, 40
        movdqu  xmm1, xmmword ptr [esp+50H]             ; 0AC3 _ F3: 0F 6F. 4C 24, 50
        movdqu  xmm0, xmmword ptr [esp+40H]             ; 0AC9 _ F3: 0F 6F. 44 24, 40
        pxor    xmm0, xmm1                              ; 0ACF _ 66: 0F EF. C1
        movups  xmmword ptr [esp], xmm0                 ; 0AD3 _ 0F 11. 04 24
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 0AD7 _ F3: 0F 6F. 44 24, 10
        pslldq  xmm0, 4                                 ; 0ADD _ 66: 0F 73. F8, 04
        movups  xmmword ptr [esp+10H], xmm0             ; 0AE2 _ 0F 11. 44 24, 10
        movdqu  xmm0, xmmword ptr [esp]                 ; 0AE7 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+70H], xmm0             ; 0AEC _ 0F 11. 44 24, 70
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 0AF1 _ F3: 0F 6F. 44 24, 10
        movups  xmmword ptr [esp+60H], xmm0             ; 0AF7 _ 0F 11. 44 24, 60
        movdqu  xmm1, xmmword ptr [esp+70H]             ; 0AFC _ F3: 0F 6F. 4C 24, 70
        movdqu  xmm0, xmmword ptr [esp+60H]             ; 0B02 _ F3: 0F 6F. 44 24, 60
        pxor    xmm0, xmm1                              ; 0B08 _ 66: 0F EF. C1
        movups  xmmword ptr [esp], xmm0                 ; 0B0C _ 0F 11. 04 24
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 0B10 _ F3: 0F 6F. 44 24, 10
        pslldq  xmm0, 4                                 ; 0B16 _ 66: 0F 73. F8, 04
        movups  xmmword ptr [esp+10H], xmm0             ; 0B1B _ 0F 11. 44 24, 10
        movdqu  xmm0, xmmword ptr [esp]                 ; 0B20 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+90H], xmm0             ; 0B25 _ 0F 11. 84 24, 00000090
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 0B2D _ F3: 0F 6F. 44 24, 10
        movups  xmmword ptr [esp+80H], xmm0             ; 0B33 _ 0F 11. 84 24, 00000080
        movdqu  xmm1, xmmword ptr [esp+90H]             ; 0B3B _ F3: 0F 6F. 8C 24, 00000090
        movdqu  xmm0, xmmword ptr [esp+80H]             ; 0B44 _ F3: 0F 6F. 84 24, 00000080
        pxor    xmm0, xmm1                              ; 0B4D _ 66: 0F EF. C1
        movups  xmmword ptr [esp+10H], xmm0             ; 0B51 _ 0F 11. 44 24, 10
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 0B56 _ F3: 0F 6F. 44 24, 10
        leave                                           ; 0B5C _ C9
        ret                                             ; 0B5D _ C3

_aesni_setkey_enc_128 LABEL NEAR
        push    ebp                                     ; 0B5E _ 55
        mov     ebp, esp                                ; 0B5F _ 89. E5
        push    ebx                                     ; 0B61 _ 53
        and     esp, 0FFFFFFF0H                         ; 0B62 _ 83. E4, F0
        sub     esp, 16                                 ; 0B65 _ 83. EC, 10
        mov     eax, dword ptr [ebp+8H]                 ; 0B68 _ 8B. 45, 08
        mov     dword ptr [esp+0CH], eax                ; 0B6B _ 89. 44 24, 0C
        mov     eax, dword ptr [ebp+0CH]                ; 0B6F _ 8B. 45, 0C
        movdqu  xmm0, xmmword ptr [eax]                 ; 0B72 _ F3: 0F 6F. 00
        mov     eax, dword ptr [esp+0CH]                ; 0B76 _ 8B. 44 24, 0C
        movups  xmmword ptr [eax], xmm0                 ; 0B7A _ 0F 11. 00
        mov     eax, dword ptr [esp+0CH]                ; 0B7D _ 8B. 44 24, 0C
        movdqu  xmm0, xmmword ptr [eax]                 ; 0B81 _ F3: 0F 6F. 00
        aeskeygenassist xmm1, xmm0, 01H                 ; 0B85 _ 66: 0F 3A DF. C8, 01
        mov     eax, dword ptr [esp+0CH]                ; 0B8B _ 8B. 44 24, 0C
        movdqu  xmm0, xmmword ptr [eax]                 ; 0B8F _ F3: 0F 6F. 00
        mov     eax, dword ptr [esp+0CH]                ; 0B93 _ 8B. 44 24, 0C
        lea     ebx, [eax+10H]                          ; 0B97 _ 8D. 58, 10
        call    _aesni_set_rk_128                       ; 0B9A _ E8, FFFFFEB3
        movups  xmmword ptr [ebx], xmm0                 ; 0B9F _ 0F 11. 03
        mov     eax, dword ptr [esp+0CH]                ; 0BA2 _ 8B. 44 24, 0C
        add     eax, 16                                 ; 0BA6 _ 83. C0, 10
        movdqu  xmm0, xmmword ptr [eax]                 ; 0BA9 _ F3: 0F 6F. 00
        aeskeygenassist xmm1, xmm0, 02H                 ; 0BAD _ 66: 0F 3A DF. C8, 02
        mov     eax, dword ptr [esp+0CH]                ; 0BB3 _ 8B. 44 24, 0C
        add     eax, 16                                 ; 0BB7 _ 83. C0, 10
        movdqu  xmm0, xmmword ptr [eax]                 ; 0BBA _ F3: 0F 6F. 00
        mov     eax, dword ptr [esp+0CH]                ; 0BBE _ 8B. 44 24, 0C
        lea     ebx, [eax+20H]                          ; 0BC2 _ 8D. 58, 20
        call    _aesni_set_rk_128                       ; 0BC5 _ E8, FFFFFE88
        movups  xmmword ptr [ebx], xmm0                 ; 0BCA _ 0F 11. 03
        mov     eax, dword ptr [esp+0CH]                ; 0BCD _ 8B. 44 24, 0C
        add     eax, 32                                 ; 0BD1 _ 83. C0, 20
        movdqu  xmm0, xmmword ptr [eax]                 ; 0BD4 _ F3: 0F 6F. 00
        aeskeygenassist xmm1, xmm0, 04H                 ; 0BD8 _ 66: 0F 3A DF. C8, 04
        mov     eax, dword ptr [esp+0CH]                ; 0BDE _ 8B. 44 24, 0C
        add     eax, 32                                 ; 0BE2 _ 83. C0, 20
        movdqu  xmm0, xmmword ptr [eax]                 ; 0BE5 _ F3: 0F 6F. 00
        mov     eax, dword ptr [esp+0CH]                ; 0BE9 _ 8B. 44 24, 0C
        lea     ebx, [eax+30H]                          ; 0BED _ 8D. 58, 30
        call    _aesni_set_rk_128                       ; 0BF0 _ E8, FFFFFE5D
        movups  xmmword ptr [ebx], xmm0                 ; 0BF5 _ 0F 11. 03
        mov     eax, dword ptr [esp+0CH]                ; 0BF8 _ 8B. 44 24, 0C
        add     eax, 48                                 ; 0BFC _ 83. C0, 30
        movdqu  xmm0, xmmword ptr [eax]                 ; 0BFF _ F3: 0F 6F. 00
        aeskeygenassist xmm1, xmm0, 08H                 ; 0C03 _ 66: 0F 3A DF. C8, 08
        mov     eax, dword ptr [esp+0CH]                ; 0C09 _ 8B. 44 24, 0C
        add     eax, 48                                 ; 0C0D _ 83. C0, 30
        movdqu  xmm0, xmmword ptr [eax]                 ; 0C10 _ F3: 0F 6F. 00
        mov     eax, dword ptr [esp+0CH]                ; 0C14 _ 8B. 44 24, 0C
        lea     ebx, [eax+40H]                          ; 0C18 _ 8D. 58, 40
        call    _aesni_set_rk_128                       ; 0C1B _ E8, FFFFFE32
        movups  xmmword ptr [ebx], xmm0                 ; 0C20 _ 0F 11. 03
        mov     eax, dword ptr [esp+0CH]                ; 0C23 _ 8B. 44 24, 0C
        add     eax, 64                                 ; 0C27 _ 83. C0, 40
        movdqu  xmm0, xmmword ptr [eax]                 ; 0C2A _ F3: 0F 6F. 00
        aeskeygenassist xmm1, xmm0, 10H                 ; 0C2E _ 66: 0F 3A DF. C8, 10
        mov     eax, dword ptr [esp+0CH]                ; 0C34 _ 8B. 44 24, 0C
        add     eax, 64                                 ; 0C38 _ 83. C0, 40
        movdqu  xmm0, xmmword ptr [eax]                 ; 0C3B _ F3: 0F 6F. 00
        mov     eax, dword ptr [esp+0CH]                ; 0C3F _ 8B. 44 24, 0C
        lea     ebx, [eax+50H]                          ; 0C43 _ 8D. 58, 50
        call    _aesni_set_rk_128                       ; 0C46 _ E8, FFFFFE07
        movups  xmmword ptr [ebx], xmm0                 ; 0C4B _ 0F 11. 03
        mov     eax, dword ptr [esp+0CH]                ; 0C4E _ 8B. 44 24, 0C
        add     eax, 80                                 ; 0C52 _ 83. C0, 50
        movdqu  xmm0, xmmword ptr [eax]                 ; 0C55 _ F3: 0F 6F. 00
        aeskeygenassist xmm1, xmm0, 20H                 ; 0C59 _ 66: 0F 3A DF. C8, 20
        mov     eax, dword ptr [esp+0CH]                ; 0C5F _ 8B. 44 24, 0C
        add     eax, 80                                 ; 0C63 _ 83. C0, 50
        movdqu  xmm0, xmmword ptr [eax]                 ; 0C66 _ F3: 0F 6F. 00
        mov     eax, dword ptr [esp+0CH]                ; 0C6A _ 8B. 44 24, 0C
        lea     ebx, [eax+60H]                          ; 0C6E _ 8D. 58, 60
        call    _aesni_set_rk_128                       ; 0C71 _ E8, FFFFFDDC
        movups  xmmword ptr [ebx], xmm0                 ; 0C76 _ 0F 11. 03
        mov     eax, dword ptr [esp+0CH]                ; 0C79 _ 8B. 44 24, 0C
        add     eax, 96                                 ; 0C7D _ 83. C0, 60
        movdqu  xmm0, xmmword ptr [eax]                 ; 0C80 _ F3: 0F 6F. 00
        aeskeygenassist xmm1, xmm0, 40H                 ; 0C84 _ 66: 0F 3A DF. C8, 40
        mov     eax, dword ptr [esp+0CH]                ; 0C8A _ 8B. 44 24, 0C
        add     eax, 96                                 ; 0C8E _ 83. C0, 60
        movdqu  xmm0, xmmword ptr [eax]                 ; 0C91 _ F3: 0F 6F. 00
        mov     eax, dword ptr [esp+0CH]                ; 0C95 _ 8B. 44 24, 0C
        lea     ebx, [eax+70H]                          ; 0C99 _ 8D. 58, 70
        call    _aesni_set_rk_128                       ; 0C9C _ E8, FFFFFDB1
        movups  xmmword ptr [ebx], xmm0                 ; 0CA1 _ 0F 11. 03
        mov     eax, dword ptr [esp+0CH]                ; 0CA4 _ 8B. 44 24, 0C
        add     eax, 112                                ; 0CA8 _ 83. C0, 70
        movdqu  xmm0, xmmword ptr [eax]                 ; 0CAB _ F3: 0F 6F. 00
        aeskeygenassist xmm1, xmm0, 80H                 ; 0CAF _ 66: 0F 3A DF. C8, 80
        mov     eax, dword ptr [esp+0CH]                ; 0CB5 _ 8B. 44 24, 0C
        add     eax, 112                                ; 0CB9 _ 83. C0, 70
        movdqu  xmm0, xmmword ptr [eax]                 ; 0CBC _ F3: 0F 6F. 00
        mov     eax, dword ptr [esp+0CH]                ; 0CC0 _ 8B. 44 24, 0C
        lea     ebx, [eax+80H]                          ; 0CC4 _ 8D. 98, 00000080
        call    _aesni_set_rk_128                       ; 0CCA _ E8, FFFFFD83
        movups  xmmword ptr [ebx], xmm0                 ; 0CCF _ 0F 11. 03
        mov     eax, dword ptr [esp+0CH]                ; 0CD2 _ 8B. 44 24, 0C
        sub     eax, -128                               ; 0CD6 _ 83. E8, 80
        movdqu  xmm0, xmmword ptr [eax]                 ; 0CD9 _ F3: 0F 6F. 00
        aeskeygenassist xmm1, xmm0, 1BH                 ; 0CDD _ 66: 0F 3A DF. C8, 1B
        mov     eax, dword ptr [esp+0CH]                ; 0CE3 _ 8B. 44 24, 0C
        sub     eax, -128                               ; 0CE7 _ 83. E8, 80
        movdqu  xmm0, xmmword ptr [eax]                 ; 0CEA _ F3: 0F 6F. 00
        mov     eax, dword ptr [esp+0CH]                ; 0CEE _ 8B. 44 24, 0C
        lea     ebx, [eax+90H]                          ; 0CF2 _ 8D. 98, 00000090
        call    _aesni_set_rk_128                       ; 0CF8 _ E8, FFFFFD55
        movups  xmmword ptr [ebx], xmm0                 ; 0CFD _ 0F 11. 03
        mov     eax, dword ptr [esp+0CH]                ; 0D00 _ 8B. 44 24, 0C
        add     eax, 144                                ; 0D04 _ 05, 00000090
        movdqu  xmm0, xmmword ptr [eax]                 ; 0D09 _ F3: 0F 6F. 00
        aeskeygenassist xmm1, xmm0, 36H                 ; 0D0D _ 66: 0F 3A DF. C8, 36
        mov     eax, dword ptr [esp+0CH]                ; 0D13 _ 8B. 44 24, 0C
        add     eax, 144                                ; 0D17 _ 05, 00000090
        movdqu  xmm0, xmmword ptr [eax]                 ; 0D1C _ F3: 0F 6F. 00
        mov     eax, dword ptr [esp+0CH]                ; 0D20 _ 8B. 44 24, 0C
        lea     ebx, [eax+0A0H]                         ; 0D24 _ 8D. 98, 000000A0
        call    _aesni_set_rk_128                       ; 0D2A _ E8, FFFFFD23
        movups  xmmword ptr [ebx], xmm0                 ; 0D2F _ 0F 11. 03
        nop                                             ; 0D32 _ 90
        mov     ebx, dword ptr [ebp-4H]                 ; 0D33 _ 8B. 5D, FC
        leave                                           ; 0D36 _ C9
        ret                                             ; 0D37 _ C3

_aesni_set_rk_192 LABEL NEAR
        push    ebp                                     ; 0D38 _ 55
        mov     ebp, esp                                ; 0D39 _ 89. E5
        and     esp, 0FFFFFFF0H                         ; 0D3B _ 83. E4, F0
        sub     esp, 208                                ; 0D3E _ 81. EC, 000000D0
        movups  xmmword ptr [esp], xmm0                 ; 0D44 _ 0F 11. 04 24
        movdqu  xmm0, xmmword ptr [esp]                 ; 0D48 _ F3: 0F 6F. 04 24
        pshufd  xmm0, xmm0, 55H                         ; 0D4D _ 66: 0F 70. C0, 55
        movups  xmmword ptr [esp], xmm0                 ; 0D52 _ 0F 11. 04 24
        mov     eax, dword ptr [ebp+8H]                 ; 0D56 _ 8B. 45, 08
        movdqu  xmm0, xmmword ptr [eax]                 ; 0D59 _ F3: 0F 6F. 00
        movdqu  xmm1, xmmword ptr [esp]                 ; 0D5D _ F3: 0F 6F. 0C 24
        movups  xmmword ptr [esp+20H], xmm1             ; 0D62 _ 0F 11. 4C 24, 20
        movups  xmmword ptr [esp+10H], xmm0             ; 0D67 _ 0F 11. 44 24, 10
        movdqu  xmm1, xmmword ptr [esp+20H]             ; 0D6C _ F3: 0F 6F. 4C 24, 20
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 0D72 _ F3: 0F 6F. 44 24, 10
        pxor    xmm0, xmm1                              ; 0D78 _ 66: 0F EF. C1
        movups  xmmword ptr [esp], xmm0                 ; 0D7C _ 0F 11. 04 24
        mov     eax, dword ptr [ebp+8H]                 ; 0D80 _ 8B. 45, 08
        movdqu  xmm0, xmmword ptr [eax]                 ; 0D83 _ F3: 0F 6F. 00
        pslldq  xmm0, 4                                 ; 0D87 _ 66: 0F 73. F8, 04
        mov     eax, dword ptr [ebp+8H]                 ; 0D8C _ 8B. 45, 08
        movups  xmmword ptr [eax], xmm0                 ; 0D8F _ 0F 11. 00
        mov     eax, dword ptr [ebp+8H]                 ; 0D92 _ 8B. 45, 08
        movdqu  xmm0, xmmword ptr [eax]                 ; 0D95 _ F3: 0F 6F. 00
        movdqu  xmm1, xmmword ptr [esp]                 ; 0D99 _ F3: 0F 6F. 0C 24
        movups  xmmword ptr [esp+40H], xmm1             ; 0D9E _ 0F 11. 4C 24, 40
        movups  xmmword ptr [esp+30H], xmm0             ; 0DA3 _ 0F 11. 44 24, 30
        movdqu  xmm1, xmmword ptr [esp+40H]             ; 0DA8 _ F3: 0F 6F. 4C 24, 40
        movdqu  xmm0, xmmword ptr [esp+30H]             ; 0DAE _ F3: 0F 6F. 44 24, 30
        pxor    xmm0, xmm1                              ; 0DB4 _ 66: 0F EF. C1
        movups  xmmword ptr [esp], xmm0                 ; 0DB8 _ 0F 11. 04 24
        mov     eax, dword ptr [ebp+8H]                 ; 0DBC _ 8B. 45, 08
        movdqu  xmm0, xmmword ptr [eax]                 ; 0DBF _ F3: 0F 6F. 00
        pslldq  xmm0, 4                                 ; 0DC3 _ 66: 0F 73. F8, 04
        mov     eax, dword ptr [ebp+8H]                 ; 0DC8 _ 8B. 45, 08
        movups  xmmword ptr [eax], xmm0                 ; 0DCB _ 0F 11. 00
        mov     eax, dword ptr [ebp+8H]                 ; 0DCE _ 8B. 45, 08
        movdqu  xmm0, xmmword ptr [eax]                 ; 0DD1 _ F3: 0F 6F. 00
        movdqu  xmm1, xmmword ptr [esp]                 ; 0DD5 _ F3: 0F 6F. 0C 24
        movups  xmmword ptr [esp+60H], xmm1             ; 0DDA _ 0F 11. 4C 24, 60
        movups  xmmword ptr [esp+50H], xmm0             ; 0DDF _ 0F 11. 44 24, 50
        movdqu  xmm1, xmmword ptr [esp+60H]             ; 0DE4 _ F3: 0F 6F. 4C 24, 60
        movdqu  xmm0, xmmword ptr [esp+50H]             ; 0DEA _ F3: 0F 6F. 44 24, 50
        pxor    xmm0, xmm1                              ; 0DF0 _ 66: 0F EF. C1
        movups  xmmword ptr [esp], xmm0                 ; 0DF4 _ 0F 11. 04 24
        mov     eax, dword ptr [ebp+8H]                 ; 0DF8 _ 8B. 45, 08
        movdqu  xmm0, xmmword ptr [eax]                 ; 0DFB _ F3: 0F 6F. 00
        pslldq  xmm0, 4                                 ; 0DFF _ 66: 0F 73. F8, 04
        mov     eax, dword ptr [ebp+8H]                 ; 0E04 _ 8B. 45, 08
        movups  xmmword ptr [eax], xmm0                 ; 0E07 _ 0F 11. 00
        mov     eax, dword ptr [ebp+8H]                 ; 0E0A _ 8B. 45, 08
        movdqu  xmm0, xmmword ptr [eax]                 ; 0E0D _ F3: 0F 6F. 00
        movdqu  xmm1, xmmword ptr [esp]                 ; 0E11 _ F3: 0F 6F. 0C 24
        movups  xmmword ptr [esp+80H], xmm1             ; 0E16 _ 0F 11. 8C 24, 00000080
        movups  xmmword ptr [esp+70H], xmm0             ; 0E1E _ 0F 11. 44 24, 70
        movdqu  xmm1, xmmword ptr [esp+80H]             ; 0E23 _ F3: 0F 6F. 8C 24, 00000080
        movdqu  xmm0, xmmword ptr [esp+70H]             ; 0E2C _ F3: 0F 6F. 44 24, 70
        pxor    xmm0, xmm1                              ; 0E32 _ 66: 0F EF. C1
        movups  xmmword ptr [esp], xmm0                 ; 0E36 _ 0F 11. 04 24
        mov     eax, dword ptr [ebp+8H]                 ; 0E3A _ 8B. 45, 08
        movdqu  xmm0, xmmword ptr [esp]                 ; 0E3D _ F3: 0F 6F. 04 24
        movups  xmmword ptr [eax], xmm0                 ; 0E42 _ 0F 11. 00
        movdqu  xmm0, xmmword ptr [esp]                 ; 0E45 _ F3: 0F 6F. 04 24
        pshufd  xmm0, xmm0, 0FFH                        ; 0E4A _ 66: 0F 70. C0, FF
        movups  xmmword ptr [esp], xmm0                 ; 0E4F _ 0F 11. 04 24
        mov     eax, dword ptr [ebp+0CH]                ; 0E53 _ 8B. 45, 0C
        movdqu  xmm0, xmmword ptr [eax]                 ; 0E56 _ F3: 0F 6F. 00
        movdqu  xmm1, xmmword ptr [esp]                 ; 0E5A _ F3: 0F 6F. 0C 24
        movups  xmmword ptr [esp+0A0H], xmm1            ; 0E5F _ 0F 11. 8C 24, 000000A0
        movups  xmmword ptr [esp+90H], xmm0             ; 0E67 _ 0F 11. 84 24, 00000090
        movdqu  xmm1, xmmword ptr [esp+0A0H]            ; 0E6F _ F3: 0F 6F. 8C 24, 000000A0
        movdqu  xmm0, xmmword ptr [esp+90H]             ; 0E78 _ F3: 0F 6F. 84 24, 00000090
        pxor    xmm0, xmm1                              ; 0E81 _ 66: 0F EF. C1
        movups  xmmword ptr [esp], xmm0                 ; 0E85 _ 0F 11. 04 24
        mov     eax, dword ptr [ebp+0CH]                ; 0E89 _ 8B. 45, 0C
        movdqu  xmm0, xmmword ptr [eax]                 ; 0E8C _ F3: 0F 6F. 00
        pslldq  xmm0, 4                                 ; 0E90 _ 66: 0F 73. F8, 04
        mov     eax, dword ptr [ebp+0CH]                ; 0E95 _ 8B. 45, 0C
        movups  xmmword ptr [eax], xmm0                 ; 0E98 _ 0F 11. 00
        mov     eax, dword ptr [ebp+0CH]                ; 0E9B _ 8B. 45, 0C
        movdqu  xmm0, xmmword ptr [eax]                 ; 0E9E _ F3: 0F 6F. 00
        movdqu  xmm1, xmmword ptr [esp]                 ; 0EA2 _ F3: 0F 6F. 0C 24
        movups  xmmword ptr [esp+0C0H], xmm1            ; 0EA7 _ 0F 11. 8C 24, 000000C0
        movups  xmmword ptr [esp+0B0H], xmm0            ; 0EAF _ 0F 11. 84 24, 000000B0
        movdqu  xmm1, xmmword ptr [esp+0C0H]            ; 0EB7 _ F3: 0F 6F. 8C 24, 000000C0
        movdqu  xmm0, xmmword ptr [esp+0B0H]            ; 0EC0 _ F3: 0F 6F. 84 24, 000000B0
        pxor    xmm0, xmm1                              ; 0EC9 _ 66: 0F EF. C1
        movups  xmmword ptr [esp], xmm0                 ; 0ECD _ 0F 11. 04 24
        mov     eax, dword ptr [ebp+0CH]                ; 0ED1 _ 8B. 45, 0C
        movdqu  xmm0, xmmword ptr [esp]                 ; 0ED4 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [eax], xmm0                 ; 0ED9 _ 0F 11. 00
        mov     eax, dword ptr [ebp+8H]                 ; 0EDC _ 8B. 45, 08
        movdqu  xmm0, xmmword ptr [eax]                 ; 0EDF _ F3: 0F 6F. 00
        mov     eax, dword ptr [ebp+10H]                ; 0EE3 _ 8B. 45, 10
        movups  xmmword ptr [eax], xmm0                 ; 0EE6 _ 0F 11. 00
        mov     eax, dword ptr [ebp+10H]                ; 0EE9 _ 8B. 45, 10
        lea     ecx, [eax+10H]                          ; 0EEC _ 8D. 48, 10
        mov     eax, dword ptr [ebp+0CH]                ; 0EEF _ 8B. 45, 0C
        mov     edx, dword ptr [eax+4H]                 ; 0EF2 _ 8B. 50, 04
        mov     eax, dword ptr [eax]                    ; 0EF5 _ 8B. 00
        mov     dword ptr [ecx], eax                    ; 0EF7 _ 89. 01
        mov     dword ptr [ecx+4H], edx                 ; 0EF9 _ 89. 51, 04
        nop                                             ; 0EFC _ 90
        leave                                           ; 0EFD _ C9
        ret                                             ; 0EFE _ C3

_aesni_setkey_enc_192 LABEL NEAR
        push    ebp                                     ; 0EFF _ 55
        mov     ebp, esp                                ; 0F00 _ 89. E5
        push    ebx                                     ; 0F02 _ 53
        and     esp, 0FFFFFFF0H                         ; 0F03 _ 83. E4, F0
        sub     esp, 96                                 ; 0F06 _ 83. EC, 60
        mov     ecx, dword ptr [ebp+8H]                 ; 0F09 _ 8B. 4D, 08
        mov     eax, dword ptr [ebp+0CH]                ; 0F0C _ 8B. 45, 0C
        movdqu  xmm0, xmmword ptr [eax]                 ; 0F0F _ F3: 0F 6F. 00
        movups  xmmword ptr [ecx], xmm0                 ; 0F13 _ 0F 11. 01
        mov     edx, dword ptr [eax+14H]                ; 0F16 _ 8B. 50, 14
        mov     eax, dword ptr [eax+10H]                ; 0F19 _ 8B. 40, 10
        mov     dword ptr [ecx+10H], eax                ; 0F1C _ 89. 41, 10
        mov     dword ptr [ecx+14H], edx                ; 0F1F _ 89. 51, 14
        mov     eax, dword ptr [ebp+8H]                 ; 0F22 _ 8B. 45, 08
        movdqu  xmm0, xmmword ptr [eax]                 ; 0F25 _ F3: 0F 6F. 00
        movups  xmmword ptr [esp+20H], xmm0             ; 0F29 _ 0F 11. 44 24, 20
        mov     eax, dword ptr [ebp+8H]                 ; 0F2E _ 8B. 45, 08
        add     eax, 16                                 ; 0F31 _ 83. C0, 10
        mov     dword ptr [esp+5CH], eax                ; 0F34 _ 89. 44 24, 5C
        mov     eax, dword ptr [esp+5CH]                ; 0F38 _ 8B. 44 24, 5C
        mov     edx, dword ptr [eax+4H]                 ; 0F3C _ 8B. 50, 04
        mov     eax, dword ptr [eax]                    ; 0F3F _ 8B. 00
        mov     ecx, 0                                  ; 0F41 _ B9, 00000000
        mov     ebx, 0                                  ; 0F46 _ BB, 00000000
        mov     dword ptr [esp+50H], ecx                ; 0F4B _ 89. 4C 24, 50
        mov     dword ptr [esp+54H], ebx                ; 0F4F _ 89. 5C 24, 54
        mov     dword ptr [esp+48H], eax                ; 0F53 _ 89. 44 24, 48
        mov     dword ptr [esp+4CH], edx                ; 0F57 _ 89. 54 24, 4C
        mov     eax, dword ptr [esp+48H]                ; 0F5B _ 8B. 44 24, 48
        mov     edx, dword ptr [esp+4CH]                ; 0F5F _ 8B. 54 24, 4C
        mov     ecx, dword ptr [esp+50H]                ; 0F63 _ 8B. 4C 24, 50
        mov     ebx, dword ptr [esp+54H]                ; 0F67 _ 8B. 5C 24, 54
        mov     dword ptr [esp+40H], ecx                ; 0F6B _ 89. 4C 24, 40
        mov     dword ptr [esp+44H], ebx                ; 0F6F _ 89. 5C 24, 44
        mov     dword ptr [esp+38H], eax                ; 0F73 _ 89. 44 24, 38
        mov     dword ptr [esp+3CH], edx                ; 0F77 _ 89. 54 24, 3C
        movq    xmm1, qword ptr [esp+40H]               ; 0F7B _ F3: 0F 7E. 4C 24, 40
        movq    xmm0, qword ptr [esp+38H]               ; 0F81 _ F3: 0F 7E. 44 24, 38
        punpcklqdq xmm0, xmm1                           ; 0F87 _ 66: 0F 6C. C1
        nop                                             ; 0F8B _ 90
        nop                                             ; 0F8C _ 90
        movups  xmmword ptr [esp+10H], xmm0             ; 0F8D _ 0F 11. 44 24, 10
        mov     eax, dword ptr [ebp+8H]                 ; 0F92 _ 8B. 45, 08
        add     eax, 24                                 ; 0F95 _ 83. C0, 18
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 0F98 _ F3: 0F 6F. 44 24, 10
        aeskeygenassist xmm0, xmm0, 01H                 ; 0F9E _ 66: 0F 3A DF. C0, 01
        mov     dword ptr [esp+8H], eax                 ; 0FA4 _ 89. 44 24, 08
        lea     eax, [esp+10H]                          ; 0FA8 _ 8D. 44 24, 10
        mov     dword ptr [esp+4H], eax                 ; 0FAC _ 89. 44 24, 04
        lea     eax, [esp+20H]                          ; 0FB0 _ 8D. 44 24, 20
        mov     dword ptr [esp], eax                    ; 0FB4 _ 89. 04 24
        call    _aesni_set_rk_192                       ; 0FB7 _ E8, FFFFFD7C
        mov     eax, dword ptr [ebp+8H]                 ; 0FBC _ 8B. 45, 08
        add     eax, 48                                 ; 0FBF _ 83. C0, 30
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 0FC2 _ F3: 0F 6F. 44 24, 10
        aeskeygenassist xmm0, xmm0, 02H                 ; 0FC8 _ 66: 0F 3A DF. C0, 02
        mov     dword ptr [esp+8H], eax                 ; 0FCE _ 89. 44 24, 08
        lea     eax, [esp+10H]                          ; 0FD2 _ 8D. 44 24, 10
        mov     dword ptr [esp+4H], eax                 ; 0FD6 _ 89. 44 24, 04
        lea     eax, [esp+20H]                          ; 0FDA _ 8D. 44 24, 20
        mov     dword ptr [esp], eax                    ; 0FDE _ 89. 04 24
        call    _aesni_set_rk_192                       ; 0FE1 _ E8, FFFFFD52
        mov     eax, dword ptr [ebp+8H]                 ; 0FE6 _ 8B. 45, 08
        add     eax, 72                                 ; 0FE9 _ 83. C0, 48
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 0FEC _ F3: 0F 6F. 44 24, 10
        aeskeygenassist xmm0, xmm0, 04H                 ; 0FF2 _ 66: 0F 3A DF. C0, 04
        mov     dword ptr [esp+8H], eax                 ; 0FF8 _ 89. 44 24, 08
        lea     eax, [esp+10H]                          ; 0FFC _ 8D. 44 24, 10
        mov     dword ptr [esp+4H], eax                 ; 1000 _ 89. 44 24, 04
        lea     eax, [esp+20H]                          ; 1004 _ 8D. 44 24, 20
        mov     dword ptr [esp], eax                    ; 1008 _ 89. 04 24
        call    _aesni_set_rk_192                       ; 100B _ E8, FFFFFD28
        mov     eax, dword ptr [ebp+8H]                 ; 1010 _ 8B. 45, 08
        add     eax, 96                                 ; 1013 _ 83. C0, 60
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 1016 _ F3: 0F 6F. 44 24, 10
        aeskeygenassist xmm0, xmm0, 08H                 ; 101C _ 66: 0F 3A DF. C0, 08
        mov     dword ptr [esp+8H], eax                 ; 1022 _ 89. 44 24, 08
        lea     eax, [esp+10H]                          ; 1026 _ 8D. 44 24, 10
        mov     dword ptr [esp+4H], eax                 ; 102A _ 89. 44 24, 04
        lea     eax, [esp+20H]                          ; 102E _ 8D. 44 24, 20
        mov     dword ptr [esp], eax                    ; 1032 _ 89. 04 24
        call    _aesni_set_rk_192                       ; 1035 _ E8, FFFFFCFE
        mov     eax, dword ptr [ebp+8H]                 ; 103A _ 8B. 45, 08
        add     eax, 120                                ; 103D _ 83. C0, 78
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 1040 _ F3: 0F 6F. 44 24, 10
        aeskeygenassist xmm0, xmm0, 10H                 ; 1046 _ 66: 0F 3A DF. C0, 10
        mov     dword ptr [esp+8H], eax                 ; 104C _ 89. 44 24, 08
        lea     eax, [esp+10H]                          ; 1050 _ 8D. 44 24, 10
        mov     dword ptr [esp+4H], eax                 ; 1054 _ 89. 44 24, 04
        lea     eax, [esp+20H]                          ; 1058 _ 8D. 44 24, 20
        mov     dword ptr [esp], eax                    ; 105C _ 89. 04 24
        call    _aesni_set_rk_192                       ; 105F _ E8, FFFFFCD4
        mov     eax, dword ptr [ebp+8H]                 ; 1064 _ 8B. 45, 08
        add     eax, 144                                ; 1067 _ 05, 00000090
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 106C _ F3: 0F 6F. 44 24, 10
        aeskeygenassist xmm0, xmm0, 20H                 ; 1072 _ 66: 0F 3A DF. C0, 20
        mov     dword ptr [esp+8H], eax                 ; 1078 _ 89. 44 24, 08
        lea     eax, [esp+10H]                          ; 107C _ 8D. 44 24, 10
        mov     dword ptr [esp+4H], eax                 ; 1080 _ 89. 44 24, 04
        lea     eax, [esp+20H]                          ; 1084 _ 8D. 44 24, 20
        mov     dword ptr [esp], eax                    ; 1088 _ 89. 04 24
        call    _aesni_set_rk_192                       ; 108B _ E8, FFFFFCA8
        mov     eax, dword ptr [ebp+8H]                 ; 1090 _ 8B. 45, 08
        add     eax, 168                                ; 1093 _ 05, 000000A8
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 1098 _ F3: 0F 6F. 44 24, 10
        aeskeygenassist xmm0, xmm0, 40H                 ; 109E _ 66: 0F 3A DF. C0, 40
        mov     dword ptr [esp+8H], eax                 ; 10A4 _ 89. 44 24, 08
        lea     eax, [esp+10H]                          ; 10A8 _ 8D. 44 24, 10
        mov     dword ptr [esp+4H], eax                 ; 10AC _ 89. 44 24, 04
        lea     eax, [esp+20H]                          ; 10B0 _ 8D. 44 24, 20
        mov     dword ptr [esp], eax                    ; 10B4 _ 89. 04 24
        call    _aesni_set_rk_192                       ; 10B7 _ E8, FFFFFC7C
        mov     eax, dword ptr [ebp+8H]                 ; 10BC _ 8B. 45, 08
        add     eax, 192                                ; 10BF _ 05, 000000C0
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 10C4 _ F3: 0F 6F. 44 24, 10
        aeskeygenassist xmm0, xmm0, 80H                 ; 10CA _ 66: 0F 3A DF. C0, 80
        mov     dword ptr [esp+8H], eax                 ; 10D0 _ 89. 44 24, 08
        lea     eax, [esp+10H]                          ; 10D4 _ 8D. 44 24, 10
        mov     dword ptr [esp+4H], eax                 ; 10D8 _ 89. 44 24, 04
        lea     eax, [esp+20H]                          ; 10DC _ 8D. 44 24, 20
        mov     dword ptr [esp], eax                    ; 10E0 _ 89. 04 24
        call    _aesni_set_rk_192                       ; 10E3 _ E8, FFFFFC50
        nop                                             ; 10E8 _ 90
        mov     ebx, dword ptr [ebp-4H]                 ; 10E9 _ 8B. 5D, FC
        leave                                           ; 10EC _ C9
        ret                                             ; 10ED _ C3

_aesni_set_rk_256 LABEL NEAR
        push    ebp                                     ; 10EE _ 55
        mov     ebp, esp                                ; 10EF _ 89. E5
        and     esp, 0FFFFFFF0H                         ; 10F1 _ 83. E4, F0
        sub     esp, 304                                ; 10F4 _ 81. EC, 00000130
        movups  xmmword ptr [esp+20H], xmm0             ; 10FA _ 0F 11. 44 24, 20
        movups  xmmword ptr [esp+10H], xmm1             ; 10FF _ 0F 11. 4C 24, 10
        movups  xmmword ptr [esp], xmm2                 ; 1104 _ 0F 11. 14 24
        movdqu  xmm0, xmmword ptr [esp]                 ; 1108 _ F3: 0F 6F. 04 24
        pshufd  xmm0, xmm0, 0FFH                        ; 110D _ 66: 0F 70. C0, FF
        movups  xmmword ptr [esp], xmm0                 ; 1112 _ 0F 11. 04 24
        movdqu  xmm0, xmmword ptr [esp]                 ; 1116 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+40H], xmm0             ; 111B _ 0F 11. 44 24, 40
        movdqu  xmm0, xmmword ptr [esp+20H]             ; 1120 _ F3: 0F 6F. 44 24, 20
        movups  xmmword ptr [esp+30H], xmm0             ; 1126 _ 0F 11. 44 24, 30
        movdqu  xmm1, xmmword ptr [esp+40H]             ; 112B _ F3: 0F 6F. 4C 24, 40
        movdqu  xmm0, xmmword ptr [esp+30H]             ; 1131 _ F3: 0F 6F. 44 24, 30
        pxor    xmm0, xmm1                              ; 1137 _ 66: 0F EF. C1
        movups  xmmword ptr [esp], xmm0                 ; 113B _ 0F 11. 04 24
        movdqu  xmm0, xmmword ptr [esp+20H]             ; 113F _ F3: 0F 6F. 44 24, 20
        pslldq  xmm0, 4                                 ; 1145 _ 66: 0F 73. F8, 04
        movups  xmmword ptr [esp+20H], xmm0             ; 114A _ 0F 11. 44 24, 20
        movdqu  xmm0, xmmword ptr [esp]                 ; 114F _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+60H], xmm0             ; 1154 _ 0F 11. 44 24, 60
        movdqu  xmm0, xmmword ptr [esp+20H]             ; 1159 _ F3: 0F 6F. 44 24, 20
        movups  xmmword ptr [esp+50H], xmm0             ; 115F _ 0F 11. 44 24, 50
        movdqu  xmm1, xmmword ptr [esp+60H]             ; 1164 _ F3: 0F 6F. 4C 24, 60
        movdqu  xmm0, xmmword ptr [esp+50H]             ; 116A _ F3: 0F 6F. 44 24, 50
        pxor    xmm0, xmm1                              ; 1170 _ 66: 0F EF. C1
        movups  xmmword ptr [esp], xmm0                 ; 1174 _ 0F 11. 04 24
        movdqu  xmm0, xmmword ptr [esp+20H]             ; 1178 _ F3: 0F 6F. 44 24, 20
        pslldq  xmm0, 4                                 ; 117E _ 66: 0F 73. F8, 04
        movups  xmmword ptr [esp+20H], xmm0             ; 1183 _ 0F 11. 44 24, 20
        movdqu  xmm0, xmmword ptr [esp]                 ; 1188 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+80H], xmm0             ; 118D _ 0F 11. 84 24, 00000080
        movdqu  xmm0, xmmword ptr [esp+20H]             ; 1195 _ F3: 0F 6F. 44 24, 20
        movups  xmmword ptr [esp+70H], xmm0             ; 119B _ 0F 11. 44 24, 70
        movdqu  xmm1, xmmword ptr [esp+80H]             ; 11A0 _ F3: 0F 6F. 8C 24, 00000080
        movdqu  xmm0, xmmword ptr [esp+70H]             ; 11A9 _ F3: 0F 6F. 44 24, 70
        pxor    xmm0, xmm1                              ; 11AF _ 66: 0F EF. C1
        movups  xmmword ptr [esp], xmm0                 ; 11B3 _ 0F 11. 04 24
        movdqu  xmm0, xmmword ptr [esp+20H]             ; 11B7 _ F3: 0F 6F. 44 24, 20
        pslldq  xmm0, 4                                 ; 11BD _ 66: 0F 73. F8, 04
        movups  xmmword ptr [esp+20H], xmm0             ; 11C2 _ 0F 11. 44 24, 20
        movdqu  xmm0, xmmword ptr [esp+20H]             ; 11C7 _ F3: 0F 6F. 44 24, 20
        movups  xmmword ptr [esp+0A0H], xmm0            ; 11CD _ 0F 11. 84 24, 000000A0
        movdqu  xmm0, xmmword ptr [esp]                 ; 11D5 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+90H], xmm0             ; 11DA _ 0F 11. 84 24, 00000090
        movdqu  xmm1, xmmword ptr [esp+0A0H]            ; 11E2 _ F3: 0F 6F. 8C 24, 000000A0
        movdqu  xmm0, xmmword ptr [esp+90H]             ; 11EB _ F3: 0F 6F. 84 24, 00000090
        pxor    xmm0, xmm1                              ; 11F4 _ 66: 0F EF. C1
        movups  xmmword ptr [esp+20H], xmm0             ; 11F8 _ 0F 11. 44 24, 20
        mov     eax, dword ptr [ebp+8H]                 ; 11FD _ 8B. 45, 08
        movdqu  xmm0, xmmword ptr [esp+20H]             ; 1200 _ F3: 0F 6F. 44 24, 20
        movups  xmmword ptr [eax], xmm0                 ; 1206 _ 0F 11. 00
        aeskeygenassist xmm0, xmmword ptr [esp+20H], 00H; 1209 _ 66: 0F 3A DF. 44 24, 20, 00
        movups  xmmword ptr [esp], xmm0                 ; 1211 _ 0F 11. 04 24
        movdqu  xmm0, xmmword ptr [esp]                 ; 1215 _ F3: 0F 6F. 04 24
        pshufd  xmm0, xmm0, 0AAH                        ; 121A _ 66: 0F 70. C0, AA
        movups  xmmword ptr [esp], xmm0                 ; 121F _ 0F 11. 04 24
        movdqu  xmm0, xmmword ptr [esp]                 ; 1223 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+0C0H], xmm0            ; 1228 _ 0F 11. 84 24, 000000C0
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 1230 _ F3: 0F 6F. 44 24, 10
        movups  xmmword ptr [esp+0B0H], xmm0            ; 1236 _ 0F 11. 84 24, 000000B0
        movdqu  xmm1, xmmword ptr [esp+0C0H]            ; 123E _ F3: 0F 6F. 8C 24, 000000C0
        movdqu  xmm0, xmmword ptr [esp+0B0H]            ; 1247 _ F3: 0F 6F. 84 24, 000000B0
        pxor    xmm0, xmm1                              ; 1250 _ 66: 0F EF. C1
        movups  xmmword ptr [esp], xmm0                 ; 1254 _ 0F 11. 04 24
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 1258 _ F3: 0F 6F. 44 24, 10
        pslldq  xmm0, 4                                 ; 125E _ 66: 0F 73. F8, 04
        movups  xmmword ptr [esp+10H], xmm0             ; 1263 _ 0F 11. 44 24, 10
        movdqu  xmm0, xmmword ptr [esp]                 ; 1268 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+0E0H], xmm0            ; 126D _ 0F 11. 84 24, 000000E0
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 1275 _ F3: 0F 6F. 44 24, 10
        movups  xmmword ptr [esp+0D0H], xmm0            ; 127B _ 0F 11. 84 24, 000000D0
        movdqu  xmm1, xmmword ptr [esp+0E0H]            ; 1283 _ F3: 0F 6F. 8C 24, 000000E0
        movdqu  xmm0, xmmword ptr [esp+0D0H]            ; 128C _ F3: 0F 6F. 84 24, 000000D0
        pxor    xmm0, xmm1                              ; 1295 _ 66: 0F EF. C1
        movups  xmmword ptr [esp], xmm0                 ; 1299 _ 0F 11. 04 24
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 129D _ F3: 0F 6F. 44 24, 10
        pslldq  xmm0, 4                                 ; 12A3 _ 66: 0F 73. F8, 04
        movups  xmmword ptr [esp+10H], xmm0             ; 12A8 _ 0F 11. 44 24, 10
        movdqu  xmm0, xmmword ptr [esp]                 ; 12AD _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+100H], xmm0            ; 12B2 _ 0F 11. 84 24, 00000100
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 12BA _ F3: 0F 6F. 44 24, 10
        movups  xmmword ptr [esp+0F0H], xmm0            ; 12C0 _ 0F 11. 84 24, 000000F0
        movdqu  xmm1, xmmword ptr [esp+100H]            ; 12C8 _ F3: 0F 6F. 8C 24, 00000100
        movdqu  xmm0, xmmword ptr [esp+0F0H]            ; 12D1 _ F3: 0F 6F. 84 24, 000000F0
        pxor    xmm0, xmm1                              ; 12DA _ 66: 0F EF. C1
        movups  xmmword ptr [esp], xmm0                 ; 12DE _ 0F 11. 04 24
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 12E2 _ F3: 0F 6F. 44 24, 10
        pslldq  xmm0, 4                                 ; 12E8 _ 66: 0F 73. F8, 04
        movups  xmmword ptr [esp+10H], xmm0             ; 12ED _ 0F 11. 44 24, 10
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 12F2 _ F3: 0F 6F. 44 24, 10
        movups  xmmword ptr [esp+120H], xmm0            ; 12F8 _ 0F 11. 84 24, 00000120
        movdqu  xmm0, xmmword ptr [esp]                 ; 1300 _ F3: 0F 6F. 04 24
        movups  xmmword ptr [esp+110H], xmm0            ; 1305 _ 0F 11. 84 24, 00000110
        movdqu  xmm1, xmmword ptr [esp+120H]            ; 130D _ F3: 0F 6F. 8C 24, 00000120
        movdqu  xmm0, xmmword ptr [esp+110H]            ; 1316 _ F3: 0F 6F. 84 24, 00000110
        pxor    xmm0, xmm1                              ; 131F _ 66: 0F EF. C1
        movups  xmmword ptr [esp+10H], xmm0             ; 1323 _ 0F 11. 44 24, 10
        mov     eax, dword ptr [ebp+0CH]                ; 1328 _ 8B. 45, 0C
        movdqu  xmm0, xmmword ptr [esp+10H]             ; 132B _ F3: 0F 6F. 44 24, 10
        movups  xmmword ptr [eax], xmm0                 ; 1331 _ 0F 11. 00
        nop                                             ; 1334 _ 90
        leave                                           ; 1335 _ C9
        ret                                             ; 1336 _ C3

_aesni_setkey_enc_256 LABEL NEAR
        push    ebp                                     ; 1337 _ 55
        mov     ebp, esp                                ; 1338 _ 89. E5
        and     esp, 0FFFFFFF0H                         ; 133A _ 83. E4, F0
        sub     esp, 32                                 ; 133D _ 83. EC, 20
        mov     eax, dword ptr [ebp+8H]                 ; 1340 _ 8B. 45, 08
        mov     dword ptr [esp+1CH], eax                ; 1343 _ 89. 44 24, 1C
        mov     eax, dword ptr [ebp+0CH]                ; 1347 _ 8B. 45, 0C
        movdqu  xmm0, xmmword ptr [eax]                 ; 134A _ F3: 0F 6F. 00
        mov     eax, dword ptr [esp+1CH]                ; 134E _ 8B. 44 24, 1C
        movups  xmmword ptr [eax], xmm0                 ; 1352 _ 0F 11. 00
        mov     eax, dword ptr [ebp+0CH]                ; 1355 _ 8B. 45, 0C
        lea     edx, [eax+10H]                          ; 1358 _ 8D. 50, 10
        mov     eax, dword ptr [esp+1CH]                ; 135B _ 8B. 44 24, 1C
        add     eax, 16                                 ; 135F _ 83. C0, 10
        movdqu  xmm0, xmmword ptr [edx]                 ; 1362 _ F3: 0F 6F. 02
        movups  xmmword ptr [eax], xmm0                 ; 1366 _ 0F 11. 00
        mov     eax, dword ptr [esp+1CH]                ; 1369 _ 8B. 44 24, 1C
        lea     ecx, [eax+30H]                          ; 136D _ 8D. 48, 30
        mov     eax, dword ptr [esp+1CH]                ; 1370 _ 8B. 44 24, 1C
        lea     edx, [eax+20H]                          ; 1374 _ 8D. 50, 20
        mov     eax, dword ptr [esp+1CH]                ; 1377 _ 8B. 44 24, 1C
        add     eax, 16                                 ; 137B _ 83. C0, 10
        movdqu  xmm0, xmmword ptr [eax]                 ; 137E _ F3: 0F 6F. 00
        aeskeygenassist xmm2, xmm0, 01H                 ; 1382 _ 66: 0F 3A DF. D0, 01
        mov     eax, dword ptr [esp+1CH]                ; 1388 _ 8B. 44 24, 1C
        add     eax, 16                                 ; 138C _ 83. C0, 10
        movdqu  xmm1, xmmword ptr [eax]                 ; 138F _ F3: 0F 6F. 08
        mov     eax, dword ptr [esp+1CH]                ; 1393 _ 8B. 44 24, 1C
        movdqu  xmm0, xmmword ptr [eax]                 ; 1397 _ F3: 0F 6F. 00
        mov     dword ptr [esp+4H], ecx                 ; 139B _ 89. 4C 24, 04
        mov     dword ptr [esp], edx                    ; 139F _ 89. 14 24
        call    _aesni_set_rk_256                       ; 13A2 _ E8, FFFFFD47
        mov     eax, dword ptr [esp+1CH]                ; 13A7 _ 8B. 44 24, 1C
        lea     edx, [eax+50H]                          ; 13AB _ 8D. 50, 50
        mov     eax, dword ptr [esp+1CH]                ; 13AE _ 8B. 44 24, 1C
        add     eax, 64                                 ; 13B2 _ 83. C0, 40
        mov     ecx, dword ptr [esp+1CH]                ; 13B5 _ 8B. 4C 24, 1C
        add     ecx, 48                                 ; 13B9 _ 83. C1, 30
        movdqu  xmm0, xmmword ptr [ecx]                 ; 13BC _ F3: 0F 6F. 01
        aeskeygenassist xmm2, xmm0, 02H                 ; 13C0 _ 66: 0F 3A DF. D0, 02
        mov     ecx, dword ptr [esp+1CH]                ; 13C6 _ 8B. 4C 24, 1C
        add     ecx, 48                                 ; 13CA _ 83. C1, 30
        movdqu  xmm1, xmmword ptr [ecx]                 ; 13CD _ F3: 0F 6F. 09
        mov     ecx, dword ptr [esp+1CH]                ; 13D1 _ 8B. 4C 24, 1C
        add     ecx, 32                                 ; 13D5 _ 83. C1, 20
        movdqu  xmm0, xmmword ptr [ecx]                 ; 13D8 _ F3: 0F 6F. 01
        mov     dword ptr [esp+4H], edx                 ; 13DC _ 89. 54 24, 04
        mov     dword ptr [esp], eax                    ; 13E0 _ 89. 04 24
        call    _aesni_set_rk_256                       ; 13E3 _ E8, FFFFFD06
        mov     eax, dword ptr [esp+1CH]                ; 13E8 _ 8B. 44 24, 1C
        lea     edx, [eax+70H]                          ; 13EC _ 8D. 50, 70
        mov     eax, dword ptr [esp+1CH]                ; 13EF _ 8B. 44 24, 1C
        add     eax, 96                                 ; 13F3 _ 83. C0, 60
        mov     ecx, dword ptr [esp+1CH]                ; 13F6 _ 8B. 4C 24, 1C
        add     ecx, 80                                 ; 13FA _ 83. C1, 50
        movdqu  xmm0, xmmword ptr [ecx]                 ; 13FD _ F3: 0F 6F. 01
        aeskeygenassist xmm2, xmm0, 04H                 ; 1401 _ 66: 0F 3A DF. D0, 04
        mov     ecx, dword ptr [esp+1CH]                ; 1407 _ 8B. 4C 24, 1C
        add     ecx, 80                                 ; 140B _ 83. C1, 50
        movdqu  xmm1, xmmword ptr [ecx]                 ; 140E _ F3: 0F 6F. 09
        mov     ecx, dword ptr [esp+1CH]                ; 1412 _ 8B. 4C 24, 1C
        add     ecx, 64                                 ; 1416 _ 83. C1, 40
        movdqu  xmm0, xmmword ptr [ecx]                 ; 1419 _ F3: 0F 6F. 01
        mov     dword ptr [esp+4H], edx                 ; 141D _ 89. 54 24, 04
        mov     dword ptr [esp], eax                    ; 1421 _ 89. 04 24
        call    _aesni_set_rk_256                       ; 1424 _ E8, FFFFFCC5
        mov     eax, dword ptr [esp+1CH]                ; 1429 _ 8B. 44 24, 1C
        lea     edx, [eax+90H]                          ; 142D _ 8D. 90, 00000090
        mov     eax, dword ptr [esp+1CH]                ; 1433 _ 8B. 44 24, 1C
        sub     eax, -128                               ; 1437 _ 83. E8, 80
        mov     ecx, dword ptr [esp+1CH]                ; 143A _ 8B. 4C 24, 1C
        add     ecx, 112                                ; 143E _ 83. C1, 70
        movdqu  xmm0, xmmword ptr [ecx]                 ; 1441 _ F3: 0F 6F. 01
        aeskeygenassist xmm2, xmm0, 08H                 ; 1445 _ 66: 0F 3A DF. D0, 08
        mov     ecx, dword ptr [esp+1CH]                ; 144B _ 8B. 4C 24, 1C
        add     ecx, 112                                ; 144F _ 83. C1, 70
        movdqu  xmm1, xmmword ptr [ecx]                 ; 1452 _ F3: 0F 6F. 09
        mov     ecx, dword ptr [esp+1CH]                ; 1456 _ 8B. 4C 24, 1C
        add     ecx, 96                                 ; 145A _ 83. C1, 60
        movdqu  xmm0, xmmword ptr [ecx]                 ; 145D _ F3: 0F 6F. 01
        mov     dword ptr [esp+4H], edx                 ; 1461 _ 89. 54 24, 04
        mov     dword ptr [esp], eax                    ; 1465 _ 89. 04 24
        call    _aesni_set_rk_256                       ; 1468 _ E8, FFFFFC81
        mov     eax, dword ptr [esp+1CH]                ; 146D _ 8B. 44 24, 1C
        lea     edx, [eax+0B0H]                         ; 1471 _ 8D. 90, 000000B0
        mov     eax, dword ptr [esp+1CH]                ; 1477 _ 8B. 44 24, 1C
        add     eax, 160                                ; 147B _ 05, 000000A0
        mov     ecx, dword ptr [esp+1CH]                ; 1480 _ 8B. 4C 24, 1C
        add     ecx, 144                                ; 1484 _ 81. C1, 00000090
        movdqu  xmm0, xmmword ptr [ecx]                 ; 148A _ F3: 0F 6F. 01
        aeskeygenassist xmm2, xmm0, 10H                 ; 148E _ 66: 0F 3A DF. D0, 10
        mov     ecx, dword ptr [esp+1CH]                ; 1494 _ 8B. 4C 24, 1C
        add     ecx, 144                                ; 1498 _ 81. C1, 00000090
        movdqu  xmm1, xmmword ptr [ecx]                 ; 149E _ F3: 0F 6F. 09
        mov     ecx, dword ptr [esp+1CH]                ; 14A2 _ 8B. 4C 24, 1C
        sub     ecx, -128                               ; 14A6 _ 83. E9, 80
        movdqu  xmm0, xmmword ptr [ecx]                 ; 14A9 _ F3: 0F 6F. 01
        mov     dword ptr [esp+4H], edx                 ; 14AD _ 89. 54 24, 04
        mov     dword ptr [esp], eax                    ; 14B1 _ 89. 04 24
        call    _aesni_set_rk_256                       ; 14B4 _ E8, FFFFFC35
        mov     eax, dword ptr [esp+1CH]                ; 14B9 _ 8B. 44 24, 1C
        lea     edx, [eax+0D0H]                         ; 14BD _ 8D. 90, 000000D0
        mov     eax, dword ptr [esp+1CH]                ; 14C3 _ 8B. 44 24, 1C
        add     eax, 192                                ; 14C7 _ 05, 000000C0
        mov     ecx, dword ptr [esp+1CH]                ; 14CC _ 8B. 4C 24, 1C
        add     ecx, 176                                ; 14D0 _ 81. C1, 000000B0
        movdqu  xmm0, xmmword ptr [ecx]                 ; 14D6 _ F3: 0F 6F. 01
        aeskeygenassist xmm2, xmm0, 20H                 ; 14DA _ 66: 0F 3A DF. D0, 20
        mov     ecx, dword ptr [esp+1CH]                ; 14E0 _ 8B. 4C 24, 1C
        add     ecx, 176                                ; 14E4 _ 81. C1, 000000B0
        movdqu  xmm1, xmmword ptr [ecx]                 ; 14EA _ F3: 0F 6F. 09
        mov     ecx, dword ptr [esp+1CH]                ; 14EE _ 8B. 4C 24, 1C
        add     ecx, 160                                ; 14F2 _ 81. C1, 000000A0
        movdqu  xmm0, xmmword ptr [ecx]                 ; 14F8 _ F3: 0F 6F. 01
        mov     dword ptr [esp+4H], edx                 ; 14FC _ 89. 54 24, 04
        mov     dword ptr [esp], eax                    ; 1500 _ 89. 04 24
        call    _aesni_set_rk_256                       ; 1503 _ E8, FFFFFBE6
        mov     eax, dword ptr [esp+1CH]                ; 1508 _ 8B. 44 24, 1C
        lea     edx, [eax+0F0H]                         ; 150C _ 8D. 90, 000000F0
        mov     eax, dword ptr [esp+1CH]                ; 1512 _ 8B. 44 24, 1C
        add     eax, 224                                ; 1516 _ 05, 000000E0
        mov     ecx, dword ptr [esp+1CH]                ; 151B _ 8B. 4C 24, 1C
        add     ecx, 208                                ; 151F _ 81. C1, 000000D0
        movdqu  xmm0, xmmword ptr [ecx]                 ; 1525 _ F3: 0F 6F. 01
        aeskeygenassist xmm2, xmm0, 40H                 ; 1529 _ 66: 0F 3A DF. D0, 40
        mov     ecx, dword ptr [esp+1CH]                ; 152F _ 8B. 4C 24, 1C
        add     ecx, 208                                ; 1533 _ 81. C1, 000000D0
        movdqu  xmm1, xmmword ptr [ecx]                 ; 1539 _ F3: 0F 6F. 09
        mov     ecx, dword ptr [esp+1CH]                ; 153D _ 8B. 4C 24, 1C
        add     ecx, 192                                ; 1541 _ 81. C1, 000000C0
        movdqu  xmm0, xmmword ptr [ecx]                 ; 1547 _ F3: 0F 6F. 01
        mov     dword ptr [esp+4H], edx                 ; 154B _ 89. 54 24, 04
        mov     dword ptr [esp], eax                    ; 154F _ 89. 04 24
        call    _aesni_set_rk_256                       ; 1552 _ E8, FFFFFB97
        nop                                             ; 1557 _ 90
        leave                                           ; 1558 _ C9
        ret                                             ; 1559 _ C3

_mbedtls_aesni_setkey_enc PROC NEAR
        push    ebp                                     ; 155A _ 55
        mov     ebp, esp                                ; 155B _ 89. E5
        sub     esp, 24                                 ; 155D _ 83. EC, 18
        cmp     dword ptr [ebp+10H], 256                ; 1560 _ 81. 7D, 10, 00000100
        jz      ?_015                                   ; 1567 _ 74, 45
        cmp     dword ptr [ebp+10H], 256                ; 1569 _ 81. 7D, 10, 00000100
        ja      ?_016                                   ; 1570 _ 77, 50
        cmp     dword ptr [ebp+10H], 128                ; 1572 _ 81. 7D, 10, 00000080
        jz      ?_013                                   ; 1579 _ 74, 0B
        cmp     dword ptr [ebp+10H], 192                ; 157B _ 81. 7D, 10, 000000C0
        jz      ?_014                                   ; 1582 _ 74, 16
        jmp     ?_016                                   ; 1584 _ EB, 3C

?_013:  mov     eax, dword ptr [ebp+0CH]                ; 1586 _ 8B. 45, 0C
        mov     dword ptr [esp+4H], eax                 ; 1589 _ 89. 44 24, 04
        mov     eax, dword ptr [ebp+8H]                 ; 158D _ 8B. 45, 08
        mov     dword ptr [esp], eax                    ; 1590 _ 89. 04 24
        call    _aesni_setkey_enc_128                   ; 1593 _ E8, FFFFF5C6
        jmp     ?_017                                   ; 1598 _ EB, 2F

?_014:  mov     eax, dword ptr [ebp+0CH]                ; 159A _ 8B. 45, 0C
        mov     dword ptr [esp+4H], eax                 ; 159D _ 89. 44 24, 04
        mov     eax, dword ptr [ebp+8H]                 ; 15A1 _ 8B. 45, 08
        mov     dword ptr [esp], eax                    ; 15A4 _ 89. 04 24
        call    _aesni_setkey_enc_192                   ; 15A7 _ E8, FFFFF953
        jmp     ?_017                                   ; 15AC _ EB, 1B

?_015:  mov     eax, dword ptr [ebp+0CH]                ; 15AE _ 8B. 45, 0C
        mov     dword ptr [esp+4H], eax                 ; 15B1 _ 89. 44 24, 04
        mov     eax, dword ptr [ebp+8H]                 ; 15B5 _ 8B. 45, 08
        mov     dword ptr [esp], eax                    ; 15B8 _ 89. 04 24
        call    _aesni_setkey_enc_256                   ; 15BB _ E8, FFFFFD77
        jmp     ?_017                                   ; 15C0 _ EB, 07

?_016:  mov     eax, 4294967264                         ; 15C2 _ B8, FFFFFFE0
        jmp     ?_018                                   ; 15C7 _ EB, 05

?_017:  mov     eax, 0                                  ; 15C9 _ B8, 00000000
?_018:  leave                                           ; 15CE _ C9
        ret                                             ; 15CF _ C3
_mbedtls_aesni_setkey_enc ENDP

.text   ENDS

END
