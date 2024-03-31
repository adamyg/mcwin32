;;
;   Module: aesni.obj
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
;       uasm32 or ml
;;

option dotname

.686
.xmm
.model flat

public _mbedtls_aesni_crypt_ecb
public _mbedtls_aesni_gcm_mult
public _mbedtls_aesni_has_support
public _mbedtls_aesni_inverse_key
public _mbedtls_aesni_setkey_enc

;; BSS

.bss    SEGMENT DWORD PUBLIC 'BSS'

aesni_has_support_done label dword
        dd      ?                                       ; 0000

aesni_has_support_flag label dword
        dd      ?                                       ; 0004

aesni_has_support_info label dword
        dd      4 dup (?)                               ; 0008

.bss    ENDS

;; CODE

.text$mn SEGMENT PARA PUBLIC 'CODE'
        ; Communal section not supported by MASM

_aesni_set_rk_128 LABEL NEAR
        movaps  xmm3, xmm0                              ; 0000 _ 0F 28. D8
        pshufd  xmm1, xmm1, 0FFH                        ; 0003 _ 66: 0F 70. C9, FF
        pxor    xmm1, xmm3                              ; 0008 _ 66: 0F EF. CB
        pslldq  xmm3, 4                                 ; 000C _ 66: 0F 73. FB, 04
        movaps  xmm2, xmm3                              ; 0011 _ 0F 28. D3
        pslldq  xmm3, 4                                 ; 0014 _ 66: 0F 73. FB, 04
        movaps  xmm0, xmm3                              ; 0019 _ 0F 28. C3
        pxor    xmm2, xmm1                              ; 001C _ 66: 0F EF. D1
        pxor    xmm0, xmm2                              ; 0020 _ 66: 0F EF. C2
        pslldq  xmm3, 4                                 ; 0024 _ 66: 0F 73. FB, 04
        pxor    xmm0, xmm3                              ; 0029 _ 66: 0F EF. C3
        ret                                             ; 002D _ C3

.text$mn ENDS

.text$mn SEGMENT PARA PUBLIC 'CODE'                     ; section number 7
        ; Communal section not supported by MASM

_aesni_set_rk_192 LABEL NEAR
        mov     eax, dword ptr [esp+4H]                 ; 0000 _ 8B. 44 24, 04
        mov     edx, dword ptr [esp+8H]                 ; 0004 _ 8B. 54 24, 08
        mov     ecx, dword ptr [esp+0CH]                ; 0008 _ 8B. 4C 24, 0C
        pshufd  xmm0, xmm0, 55H                         ; 000C _ 66: 0F 70. C0, 55
        movaps  xmm2, xmmword ptr [eax]                 ; 0011 _ 0F 28. 10
        pxor    xmm0, xmm2                              ; 0014 _ 66: 0F EF. C2
        pslldq  xmm2, 4                                 ; 0018 _ 66: 0F 73. FA, 04
        movaps  xmm1, xmm2                              ; 001D _ 0F 28. CA
        pslldq  xmm2, 4                                 ; 0020 _ 66: 0F 73. FA, 04
        pxor    xmm1, xmm0                              ; 0025 _ 66: 0F EF. C8
        movaps  xmm0, xmm2                              ; 0029 _ 0F 28. C2
        pxor    xmm0, xmm1                              ; 002C _ 66: 0F EF. C1
        pslldq  xmm2, 4                                 ; 0030 _ 66: 0F 73. FA, 04
        pxor    xmm2, xmm0                              ; 0035 _ 66: 0F EF. D0
        movaps  xmmword ptr [eax], xmm2                 ; 0039 _ 0F 29. 10
        movaps  xmm1, xmmword ptr [edx]                 ; 003C _ 0F 28. 0A
        pshufd  xmm0, xmm2, 0FFH                        ; 003F _ 66: 0F 70. C2, FF
        pxor    xmm0, xmm1                              ; 0044 _ 66: 0F EF. C1
        pslldq  xmm1, 4                                 ; 0048 _ 66: 0F 73. F9, 04
        pxor    xmm1, xmm0                              ; 004D _ 66: 0F EF. C8
        movaps  xmmword ptr [edx], xmm1                 ; 0051 _ 0F 29. 0A
        movups  xmm0, xmmword ptr [eax]                 ; 0054 _ 0F 10. 00
        movups  xmmword ptr [ecx], xmm0                 ; 0057 _ 0F 11. 01
        mov     eax, dword ptr [edx]                    ; 005A _ 8B. 02
        mov     dword ptr [ecx+10H], eax                ; 005C _ 89. 41, 10
        mov     eax, dword ptr [edx+4H]                 ; 005F _ 8B. 42, 04
        mov     dword ptr [ecx+14H], eax                ; 0062 _ 89. 41, 14
        ret                                             ; 0065 _ C3

.text$mn ENDS

.text$mn SEGMENT PARA PUBLIC 'CODE'
        ; Communal section not supported by MASM

_aesni_set_rk_256 LABEL NEAR
        mov     eax, dword ptr [esp+4H]                 ; 0000 _ 8B. 44 24, 04
        movaps  xmm4, xmm0                              ; 0004 _ 0F 28. E0
        pshufd  xmm2, xmm2, 0FFH                        ; 0007 _ 66: 0F 70. D2, FF
        movaps  xmm5, xmm1                              ; 000C _ 0F 28. E9
        pxor    xmm2, xmm4                              ; 000F _ 66: 0F EF. D4
        pslldq  xmm4, 4                                 ; 0013 _ 66: 0F 73. FC, 04
        movaps  xmm3, xmm4                              ; 0018 _ 0F 28. DC
        pslldq  xmm4, 4                                 ; 001B _ 66: 0F 73. FC, 04
        movaps  xmm0, xmm4                              ; 0020 _ 0F 28. C4
        pxor    xmm3, xmm2                              ; 0023 _ 66: 0F EF. DA
        pxor    xmm0, xmm3                              ; 0027 _ 66: 0F EF. C3
        pslldq  xmm4, 4                                 ; 002B _ 66: 0F 73. FC, 04
        pxor    xmm0, xmm4                              ; 0030 _ 66: 0F EF. C4
        movaps  xmmword ptr [eax], xmm0                 ; 0034 _ 0F 29. 00
        mov     eax, dword ptr [esp+8H]                 ; 0037 _ 8B. 44 24, 08
        aeskeygenassist xmm0, xmm0, 00H                 ; 003B _ 66: 0F 3A DF. C0, 00
        pshufd  xmm0, xmm0, 0AAH                        ; 0041 _ 66: 0F 70. C0, AA
        pxor    xmm0, xmm5                              ; 0046 _ 66: 0F EF. C5
        pslldq  xmm5, 4                                 ; 004A _ 66: 0F 73. FD, 04
        movaps  xmm1, xmm5                              ; 004F _ 0F 28. CD
        pslldq  xmm5, 4                                 ; 0052 _ 66: 0F 73. FD, 04
        pxor    xmm1, xmm0                              ; 0057 _ 66: 0F EF. C8
        movaps  xmm0, xmm5                              ; 005B _ 0F 28. C5
        pxor    xmm0, xmm1                              ; 005E _ 66: 0F EF. C1
        pslldq  xmm5, 4                                 ; 0062 _ 66: 0F 73. FD, 04
        pxor    xmm0, xmm5                              ; 0067 _ 66: 0F EF. C5
        movaps  xmmword ptr [eax], xmm0                 ; 006B _ 0F 29. 00
        ret                                             ; 006E _ C3

.text$mn ENDS

.text$mn SEGMENT PARA PUBLIC 'CODE'
        ; Communal section not supported by MASM

_aesni_setkey_enc_128 LABEL NEAR
        mov     eax, dword ptr [esp+8H]                 ; 0000 _ 8B. 44 24, 08
        movups  xmm0, xmmword ptr [eax]                 ; 0004 _ 0F 10. 00
        mov     eax, dword ptr [esp+4H]                 ; 0007 _ 8B. 44 24, 04
        movups  xmmword ptr [eax], xmm0                 ; 000B _ 0F 11. 00
        movaps  xmm2, xmmword ptr [eax]                 ; 000E _ 0F 28. 10
        aeskeygenassist xmm0, xmm2, 01H                 ; 0011 _ 66: 0F 3A DF. C2, 01
        pshufd  xmm0, xmm0, 0FFH                        ; 0017 _ 66: 0F 70. C0, FF
        pxor    xmm0, xmm2                              ; 001C _ 66: 0F EF. C2
        pslldq  xmm2, 4                                 ; 0020 _ 66: 0F 73. FA, 04
        movaps  xmm1, xmm2                              ; 0025 _ 0F 28. CA
        pslldq  xmm2, 4                                 ; 0028 _ 66: 0F 73. FA, 04
        pxor    xmm1, xmm0                              ; 002D _ 66: 0F EF. C8
        movaps  xmm3, xmm2                              ; 0031 _ 0F 28. DA
        pxor    xmm3, xmm1                              ; 0034 _ 66: 0F EF. D9
        pslldq  xmm2, 4                                 ; 0038 _ 66: 0F 73. FA, 04
        pxor    xmm3, xmm2                              ; 003D _ 66: 0F EF. DA
        aeskeygenassist xmm0, xmm3, 02H                 ; 0041 _ 66: 0F 3A DF. C3, 02
        pshufd  xmm0, xmm0, 0FFH                        ; 0047 _ 66: 0F 70. C0, FF
        pxor    xmm0, xmm3                              ; 004C _ 66: 0F EF. C3
        movaps  xmmword ptr [eax+10H], xmm3             ; 0050 _ 0F 29. 58, 10
        pslldq  xmm3, 4                                 ; 0054 _ 66: 0F 73. FB, 04
        movaps  xmm1, xmm3                              ; 0059 _ 0F 28. CB
        pslldq  xmm3, 4                                 ; 005C _ 66: 0F 73. FB, 04
        pxor    xmm1, xmm0                              ; 0061 _ 66: 0F EF. C8
        movaps  xmm2, xmm3                              ; 0065 _ 0F 28. D3
        pxor    xmm2, xmm1                              ; 0068 _ 66: 0F EF. D1
        pslldq  xmm3, 4                                 ; 006C _ 66: 0F 73. FB, 04
        pxor    xmm2, xmm3                              ; 0071 _ 66: 0F EF. D3
        aeskeygenassist xmm0, xmm2, 04H                 ; 0075 _ 66: 0F 3A DF. C2, 04
        pshufd  xmm0, xmm0, 0FFH                        ; 007B _ 66: 0F 70. C0, FF
        pxor    xmm0, xmm2                              ; 0080 _ 66: 0F EF. C2
        movaps  xmmword ptr [eax+20H], xmm2             ; 0084 _ 0F 29. 50, 20
        pslldq  xmm2, 4                                 ; 0088 _ 66: 0F 73. FA, 04
        movaps  xmm1, xmm2                              ; 008D _ 0F 28. CA
        pslldq  xmm2, 4                                 ; 0090 _ 66: 0F 73. FA, 04
        pxor    xmm1, xmm0                              ; 0095 _ 66: 0F EF. C8
        movaps  xmm3, xmm2                              ; 0099 _ 0F 28. DA
        pxor    xmm3, xmm1                              ; 009C _ 66: 0F EF. D9
        pslldq  xmm2, 4                                 ; 00A0 _ 66: 0F 73. FA, 04
        pxor    xmm3, xmm2                              ; 00A5 _ 66: 0F EF. DA
        aeskeygenassist xmm0, xmm3, 08H                 ; 00A9 _ 66: 0F 3A DF. C3, 08
        pshufd  xmm0, xmm0, 0FFH                        ; 00AF _ 66: 0F 70. C0, FF
        pxor    xmm0, xmm3                              ; 00B4 _ 66: 0F EF. C3
        movaps  xmmword ptr [eax+30H], xmm3             ; 00B8 _ 0F 29. 58, 30
        pslldq  xmm3, 4                                 ; 00BC _ 66: 0F 73. FB, 04
        movaps  xmm1, xmm3                              ; 00C1 _ 0F 28. CB
        pslldq  xmm3, 4                                 ; 00C4 _ 66: 0F 73. FB, 04
        pxor    xmm1, xmm0                              ; 00C9 _ 66: 0F EF. C8
        movaps  xmm2, xmm3                              ; 00CD _ 0F 28. D3
        pxor    xmm2, xmm1                              ; 00D0 _ 66: 0F EF. D1
        pslldq  xmm3, 4                                 ; 00D4 _ 66: 0F 73. FB, 04
        pxor    xmm2, xmm3                              ; 00D9 _ 66: 0F EF. D3
        aeskeygenassist xmm0, xmm2, 10H                 ; 00DD _ 66: 0F 3A DF. C2, 10
        pshufd  xmm0, xmm0, 0FFH                        ; 00E3 _ 66: 0F 70. C0, FF
        pxor    xmm0, xmm2                              ; 00E8 _ 66: 0F EF. C2
        movaps  xmmword ptr [eax+40H], xmm2             ; 00EC _ 0F 29. 50, 40
        pslldq  xmm2, 4                                 ; 00F0 _ 66: 0F 73. FA, 04
        movaps  xmm1, xmm2                              ; 00F5 _ 0F 28. CA
        pslldq  xmm2, 4                                 ; 00F8 _ 66: 0F 73. FA, 04
        pxor    xmm1, xmm0                              ; 00FD _ 66: 0F EF. C8
        movaps  xmm3, xmm2                              ; 0101 _ 0F 28. DA
        pxor    xmm3, xmm1                              ; 0104 _ 66: 0F EF. D9
        pslldq  xmm2, 4                                 ; 0108 _ 66: 0F 73. FA, 04
        pxor    xmm3, xmm2                              ; 010D _ 66: 0F EF. DA
        aeskeygenassist xmm0, xmm3, 20H                 ; 0111 _ 66: 0F 3A DF. C3, 20
        pshufd  xmm0, xmm0, 0FFH                        ; 0117 _ 66: 0F 70. C0, FF
        pxor    xmm0, xmm3                              ; 011C _ 66: 0F EF. C3
        movaps  xmmword ptr [eax+50H], xmm3             ; 0120 _ 0F 29. 58, 50
        pslldq  xmm3, 4                                 ; 0124 _ 66: 0F 73. FB, 04
        movaps  xmm1, xmm3                              ; 0129 _ 0F 28. CB
        pslldq  xmm3, 4                                 ; 012C _ 66: 0F 73. FB, 04
        pxor    xmm1, xmm0                              ; 0131 _ 66: 0F EF. C8
        movaps  xmm2, xmm3                              ; 0135 _ 0F 28. D3
        pxor    xmm2, xmm1                              ; 0138 _ 66: 0F EF. D1
        pslldq  xmm3, 4                                 ; 013C _ 66: 0F 73. FB, 04
        pxor    xmm2, xmm3                              ; 0141 _ 66: 0F EF. D3
        aeskeygenassist xmm0, xmm2, 40H                 ; 0145 _ 66: 0F 3A DF. C2, 40
        pshufd  xmm0, xmm0, 0FFH                        ; 014B _ 66: 0F 70. C0, FF
        movaps  xmmword ptr [eax+60H], xmm2             ; 0150 _ 0F 29. 50, 60
        pxor    xmm0, xmm2                              ; 0154 _ 66: 0F EF. C2
        pslldq  xmm2, 4                                 ; 0158 _ 66: 0F 73. FA, 04
        movaps  xmm1, xmm2                              ; 015D _ 0F 28. CA
        pslldq  xmm2, 4                                 ; 0160 _ 66: 0F 73. FA, 04
        pxor    xmm1, xmm0                              ; 0165 _ 66: 0F EF. C8
        movaps  xmm3, xmm2                              ; 0169 _ 0F 28. DA
        pxor    xmm3, xmm1                              ; 016C _ 66: 0F EF. D9
        pslldq  xmm2, 4                                 ; 0170 _ 66: 0F 73. FA, 04
        pxor    xmm3, xmm2                              ; 0175 _ 66: 0F EF. DA
        aeskeygenassist xmm0, xmm3, 80H                 ; 0179 _ 66: 0F 3A DF. C3, 80
        pshufd  xmm0, xmm0, 0FFH                        ; 017F _ 66: 0F 70. C0, FF
        pxor    xmm0, xmm3                              ; 0184 _ 66: 0F EF. C3
        movaps  xmmword ptr [eax+70H], xmm3             ; 0188 _ 0F 29. 58, 70
        pslldq  xmm3, 4                                 ; 018C _ 66: 0F 73. FB, 04
        movaps  xmm1, xmm3                              ; 0191 _ 0F 28. CB
        pslldq  xmm3, 4                                 ; 0194 _ 66: 0F 73. FB, 04
        pxor    xmm1, xmm0                              ; 0199 _ 66: 0F EF. C8
        movaps  xmm2, xmm3                              ; 019D _ 0F 28. D3
        pxor    xmm2, xmm1                              ; 01A0 _ 66: 0F EF. D1
        pslldq  xmm3, 4                                 ; 01A4 _ 66: 0F 73. FB, 04
        pxor    xmm2, xmm3                              ; 01A9 _ 66: 0F EF. D3
        aeskeygenassist xmm0, xmm2, 1BH                 ; 01AD _ 66: 0F 3A DF. C2, 1B
        pshufd  xmm0, xmm0, 0FFH                        ; 01B3 _ 66: 0F 70. C0, FF
        pxor    xmm0, xmm2                              ; 01B8 _ 66: 0F EF. C2
        movaps  xmmword ptr [eax+80H], xmm2             ; 01BC _ 0F 29. 90, 00000080
        pslldq  xmm2, 4                                 ; 01C3 _ 66: 0F 73. FA, 04
        movaps  xmm1, xmm2                              ; 01C8 _ 0F 28. CA
        pslldq  xmm2, 4                                 ; 01CB _ 66: 0F 73. FA, 04
        pxor    xmm1, xmm0                              ; 01D0 _ 66: 0F EF. C8
        movaps  xmm3, xmm2                              ; 01D4 _ 0F 28. DA
        pxor    xmm3, xmm1                              ; 01D7 _ 66: 0F EF. D9
        pslldq  xmm2, 4                                 ; 01DB _ 66: 0F 73. FA, 04
        pxor    xmm3, xmm2                              ; 01E0 _ 66: 0F EF. DA
        aeskeygenassist xmm0, xmm3, 36H                 ; 01E4 _ 66: 0F 3A DF. C3, 36
        pshufd  xmm0, xmm0, 0FFH                        ; 01EA _ 66: 0F 70. C0, FF
        pxor    xmm0, xmm3                              ; 01EF _ 66: 0F EF. C3
        movaps  xmmword ptr [eax+90H], xmm3             ; 01F3 _ 0F 29. 98, 00000090
        pslldq  xmm3, 4                                 ; 01FA _ 66: 0F 73. FB, 04
        movaps  xmm1, xmm3                              ; 01FF _ 0F 28. CB
        pslldq  xmm3, 4                                 ; 0202 _ 66: 0F 73. FB, 04
        pxor    xmm1, xmm0                              ; 0207 _ 66: 0F EF. C8
        movaps  xmm0, xmm3                              ; 020B _ 0F 28. C3
        pxor    xmm0, xmm1                              ; 020E _ 66: 0F EF. C1
        pslldq  xmm3, 4                                 ; 0212 _ 66: 0F 73. FB, 04
        pxor    xmm0, xmm3                              ; 0217 _ 66: 0F EF. C3
        movaps  xmmword ptr [eax+0A0H], xmm0            ; 021B _ 0F 29. 80, 000000A0
        ret                                             ; 0222 _ C3

.text$mn ENDS

.text$mn SEGMENT PARA PUBLIC 'CODE'
        ; Communal section not supported by MASM

_aesni_setkey_enc_192 LABEL NEAR
        mov     ecx, dword ptr [esp+4H]                 ; 0000 _ 8B. 4C 24, 04
        mov     eax, dword ptr [esp+8H]                 ; 0004 _ 8B. 44 24, 08
        movups  xmm0, xmmword ptr [eax]                 ; 0008 _ 0F 10. 00
        movups  xmmword ptr [ecx], xmm0                 ; 000B _ 0F 11. 01
        movq    xmm0, qword ptr [eax+10H]               ; 000E _ F3: 0F 7E. 40, 10
        movq    qword ptr [ecx+10H], xmm0               ; 0013 _ 66: 0F D6. 41, 10
        movq    xmm2, qword ptr [ecx+10H]               ; 0018 _ F3: 0F 7E. 51, 10
        movaps  xmm1, xmmword ptr [ecx]                 ; 001D _ 0F 28. 09
        aeskeygenassist xmm0, xmm2, 01H                 ; 0020 _ 66: 0F 3A DF. C2, 01
        pshufd  xmm3, xmm0, 55H                         ; 0026 _ 66: 0F 70. D8, 55
        pxor    xmm3, xmm1                              ; 002B _ 66: 0F EF. D9
        pslldq  xmm1, 4                                 ; 002F _ 66: 0F 73. F9, 04
        pxor    xmm3, xmm1                              ; 0034 _ 66: 0F EF. D9
        pslldq  xmm1, 4                                 ; 0038 _ 66: 0F 73. F9, 04
        pxor    xmm3, xmm1                              ; 003D _ 66: 0F EF. D9
        pslldq  xmm1, 4                                 ; 0041 _ 66: 0F 73. F9, 04
        pxor    xmm3, xmm1                              ; 0046 _ 66: 0F EF. D9
        pshufd  xmm1, xmm3, 0FFH                        ; 004A _ 66: 0F 70. CB, FF
        pxor    xmm1, xmm2                              ; 004F _ 66: 0F EF. CA
        pslldq  xmm2, 4                                 ; 0053 _ 66: 0F 73. FA, 04
        pxor    xmm1, xmm2                              ; 0058 _ 66: 0F EF. CA
        movups  xmmword ptr [ecx+18H], xmm3             ; 005C _ 0F 11. 59, 18
        movaps  xmm0, xmm1                              ; 0060 _ 0F 28. C1
        movd    dword ptr [ecx+28H], xmm1               ; 0063 _ 66: 0F 7E. 49, 28
        psrldq  xmm0, 4                                 ; 0068 _ 66: 0F 73. D8, 04
        movd    dword ptr [ecx+2CH], xmm0               ; 006D _ 66: 0F 7E. 41, 2C
        aeskeygenassist xmm0, xmm1, 02H                 ; 0072 _ 66: 0F 3A DF. C1, 02
        pshufd  xmm4, xmm0, 55H                         ; 0078 _ 66: 0F 70. E0, 55
        pxor    xmm4, xmm3                              ; 007D _ 66: 0F EF. E3
        pslldq  xmm3, 4                                 ; 0081 _ 66: 0F 73. FB, 04
        pxor    xmm4, xmm3                              ; 0086 _ 66: 0F EF. E3
        pslldq  xmm3, 4                                 ; 008A _ 66: 0F 73. FB, 04
        pxor    xmm4, xmm3                              ; 008F _ 66: 0F EF. E3
        pslldq  xmm3, 4                                 ; 0093 _ 66: 0F 73. FB, 04
        pxor    xmm4, xmm3                              ; 0098 _ 66: 0F EF. E3
        pshufd  xmm2, xmm4, 0FFH                        ; 009C _ 66: 0F 70. D4, FF
        pxor    xmm2, xmm1                              ; 00A1 _ 66: 0F EF. D1
        pslldq  xmm1, 4                                 ; 00A5 _ 66: 0F 73. F9, 04
        pxor    xmm2, xmm1                              ; 00AA _ 66: 0F EF. D1
        movups  xmmword ptr [ecx+30H], xmm4             ; 00AE _ 0F 11. 61, 30
        movaps  xmm0, xmm2                              ; 00B2 _ 0F 28. C2
        movd    dword ptr [ecx+40H], xmm2               ; 00B5 _ 66: 0F 7E. 51, 40
        psrldq  xmm0, 4                                 ; 00BA _ 66: 0F 73. D8, 04
        movd    dword ptr [ecx+44H], xmm0               ; 00BF _ 66: 0F 7E. 41, 44
        aeskeygenassist xmm0, xmm2, 04H                 ; 00C4 _ 66: 0F 3A DF. C2, 04
        pshufd  xmm3, xmm0, 55H                         ; 00CA _ 66: 0F 70. D8, 55
        pxor    xmm3, xmm4                              ; 00CF _ 66: 0F EF. DC
        pslldq  xmm4, 4                                 ; 00D3 _ 66: 0F 73. FC, 04
        pxor    xmm3, xmm4                              ; 00D8 _ 66: 0F EF. DC
        pslldq  xmm4, 4                                 ; 00DC _ 66: 0F 73. FC, 04
        pxor    xmm3, xmm4                              ; 00E1 _ 66: 0F EF. DC
        pslldq  xmm4, 4                                 ; 00E5 _ 66: 0F 73. FC, 04
        pxor    xmm3, xmm4                              ; 00EA _ 66: 0F EF. DC
        pshufd  xmm1, xmm3, 0FFH                        ; 00EE _ 66: 0F 70. CB, FF
        pxor    xmm1, xmm2                              ; 00F3 _ 66: 0F EF. CA
        pslldq  xmm2, 4                                 ; 00F7 _ 66: 0F 73. FA, 04
        pxor    xmm1, xmm2                              ; 00FC _ 66: 0F EF. CA
        movaps  xmm0, xmm1                              ; 0100 _ 0F 28. C1
        movd    dword ptr [ecx+58H], xmm1               ; 0103 _ 66: 0F 7E. 49, 58
        psrldq  xmm0, 4                                 ; 0108 _ 66: 0F 73. D8, 04
        movd    dword ptr [ecx+5CH], xmm0               ; 010D _ 66: 0F 7E. 41, 5C
        aeskeygenassist xmm0, xmm1, 08H                 ; 0112 _ 66: 0F 3A DF. C1, 08
        movups  xmmword ptr [ecx+48H], xmm3             ; 0118 _ 0F 11. 59, 48
        pshufd  xmm4, xmm0, 55H                         ; 011C _ 66: 0F 70. E0, 55
        pxor    xmm4, xmm3                              ; 0121 _ 66: 0F EF. E3
        pslldq  xmm3, 4                                 ; 0125 _ 66: 0F 73. FB, 04
        pxor    xmm4, xmm3                              ; 012A _ 66: 0F EF. E3
        pslldq  xmm3, 4                                 ; 012E _ 66: 0F 73. FB, 04
        pxor    xmm4, xmm3                              ; 0133 _ 66: 0F EF. E3
        pslldq  xmm3, 4                                 ; 0137 _ 66: 0F 73. FB, 04
        pxor    xmm4, xmm3                              ; 013C _ 66: 0F EF. E3
        pshufd  xmm2, xmm4, 0FFH                        ; 0140 _ 66: 0F 70. D4, FF
        pxor    xmm2, xmm1                              ; 0145 _ 66: 0F EF. D1
        pslldq  xmm1, 4                                 ; 0149 _ 66: 0F 73. F9, 04
        pxor    xmm2, xmm1                              ; 014E _ 66: 0F EF. D1
        movaps  xmm0, xmm2                              ; 0152 _ 0F 28. C2
        movd    dword ptr [ecx+70H], xmm2               ; 0155 _ 66: 0F 7E. 51, 70
        psrldq  xmm0, 4                                 ; 015A _ 66: 0F 73. D8, 04
        movd    dword ptr [ecx+74H], xmm0               ; 015F _ 66: 0F 7E. 41, 74
        movups  xmmword ptr [ecx+60H], xmm4             ; 0164 _ 0F 11. 61, 60
        aeskeygenassist xmm0, xmm2, 10H                 ; 0168 _ 66: 0F 3A DF. C2, 10
        pshufd  xmm3, xmm0, 55H                         ; 016E _ 66: 0F 70. D8, 55
        pxor    xmm3, xmm4                              ; 0173 _ 66: 0F EF. DC
        pslldq  xmm4, 4                                 ; 0177 _ 66: 0F 73. FC, 04
        pxor    xmm3, xmm4                              ; 017C _ 66: 0F EF. DC
        pslldq  xmm4, 4                                 ; 0180 _ 66: 0F 73. FC, 04
        pxor    xmm3, xmm4                              ; 0185 _ 66: 0F EF. DC
        pslldq  xmm4, 4                                 ; 0189 _ 66: 0F 73. FC, 04
        pxor    xmm3, xmm4                              ; 018E _ 66: 0F EF. DC
        pshufd  xmm1, xmm3, 0FFH                        ; 0192 _ 66: 0F 70. CB, FF
        pxor    xmm1, xmm2                              ; 0197 _ 66: 0F EF. CA
        pslldq  xmm2, 4                                 ; 019B _ 66: 0F 73. FA, 04
        pxor    xmm1, xmm2                              ; 01A0 _ 66: 0F EF. CA
        movups  xmmword ptr [ecx+78H], xmm3             ; 01A4 _ 0F 11. 59, 78
        movaps  xmm0, xmm1                              ; 01A8 _ 0F 28. C1
        movd    dword ptr [ecx+88H], xmm1               ; 01AB _ 66: 0F 7E. 89, 00000088
        psrldq  xmm0, 4                                 ; 01B3 _ 66: 0F 73. D8, 04
        movd    dword ptr [ecx+8CH], xmm0               ; 01B8 _ 66: 0F 7E. 81, 0000008C
        aeskeygenassist xmm0, xmm1, 20H                 ; 01C0 _ 66: 0F 3A DF. C1, 20
        pshufd  xmm4, xmm0, 55H                         ; 01C6 _ 66: 0F 70. E0, 55
        pxor    xmm4, xmm3                              ; 01CB _ 66: 0F EF. E3
        pslldq  xmm3, 4                                 ; 01CF _ 66: 0F 73. FB, 04
        pxor    xmm4, xmm3                              ; 01D4 _ 66: 0F EF. E3
        pslldq  xmm3, 4                                 ; 01D8 _ 66: 0F 73. FB, 04
        pxor    xmm4, xmm3                              ; 01DD _ 66: 0F EF. E3
        pslldq  xmm3, 4                                 ; 01E1 _ 66: 0F 73. FB, 04
        pxor    xmm4, xmm3                              ; 01E6 _ 66: 0F EF. E3
        pshufd  xmm2, xmm4, 0FFH                        ; 01EA _ 66: 0F 70. D4, FF
        pxor    xmm2, xmm1                              ; 01EF _ 66: 0F EF. D1
        pslldq  xmm1, 4                                 ; 01F3 _ 66: 0F 73. F9, 04
        pxor    xmm2, xmm1                              ; 01F8 _ 66: 0F EF. D1
        movaps  xmm0, xmm2                              ; 01FC _ 0F 28. C2
        movd    dword ptr [ecx+0A0H], xmm2              ; 01FF _ 66: 0F 7E. 91, 000000A0
        psrldq  xmm0, 4                                 ; 0207 _ 66: 0F 73. D8, 04
        movd    dword ptr [ecx+0A4H], xmm0              ; 020C _ 66: 0F 7E. 81, 000000A4
        movups  xmmword ptr [ecx+90H], xmm4             ; 0214 _ 0F 11. A1, 00000090
        aeskeygenassist xmm0, xmm2, 40H                 ; 021B _ 66: 0F 3A DF. C2, 40
        pshufd  xmm5, xmm0, 55H                         ; 0221 _ 66: 0F 70. E8, 55
        pxor    xmm5, xmm4                              ; 0226 _ 66: 0F EF. EC
        pslldq  xmm4, 4                                 ; 022A _ 66: 0F 73. FC, 04
        pxor    xmm5, xmm4                              ; 022F _ 66: 0F EF. EC
        pslldq  xmm4, 4                                 ; 0233 _ 66: 0F 73. FC, 04
        pxor    xmm5, xmm4                              ; 0238 _ 66: 0F EF. EC
        pslldq  xmm4, 4                                 ; 023C _ 66: 0F 73. FC, 04
        pxor    xmm5, xmm4                              ; 0241 _ 66: 0F EF. EC
        pshufd  xmm3, xmm5, 0FFH                        ; 0245 _ 66: 0F 70. DD, FF
        pxor    xmm3, xmm2                              ; 024A _ 66: 0F EF. DA
        pslldq  xmm2, 4                                 ; 024E _ 66: 0F 73. FA, 04
        pxor    xmm3, xmm2                              ; 0253 _ 66: 0F EF. DA
        movaps  xmm0, xmm3                              ; 0257 _ 0F 28. C3
        movd    dword ptr [ecx+0B8H], xmm3              ; 025A _ 66: 0F 7E. 99, 000000B8
        psrldq  xmm0, 4                                 ; 0262 _ 66: 0F 73. D8, 04
        movd    dword ptr [ecx+0BCH], xmm0              ; 0267 _ 66: 0F 7E. 81, 000000BC
        aeskeygenassist xmm0, xmm3, 80H                 ; 026F _ 66: 0F 3A DF. C3, 80
        pshufd  xmm1, xmm0, 55H                         ; 0275 _ 66: 0F 70. C8, 55
        movups  xmmword ptr [ecx+0A8H], xmm5            ; 027A _ 0F 11. A9, 000000A8
        pxor    xmm1, xmm5                              ; 0281 _ 66: 0F EF. CD
        pslldq  xmm5, 4                                 ; 0285 _ 66: 0F 73. FD, 04
        pxor    xmm1, xmm5                              ; 028A _ 66: 0F EF. CD
        pslldq  xmm5, 4                                 ; 028E _ 66: 0F 73. FD, 04
        pxor    xmm1, xmm5                              ; 0293 _ 66: 0F EF. CD
        pslldq  xmm5, 4                                 ; 0297 _ 66: 0F 73. FD, 04
        pxor    xmm1, xmm5                              ; 029C _ 66: 0F EF. CD
        pshufd  xmm0, xmm1, 0FFH                        ; 02A0 _ 66: 0F 70. C1, FF
        pxor    xmm0, xmm3                              ; 02A5 _ 66: 0F EF. C3
        pslldq  xmm3, 4                                 ; 02A9 _ 66: 0F 73. FB, 04
        pxor    xmm0, xmm3                              ; 02AE _ 66: 0F EF. C3
        movd    dword ptr [ecx+0D0H], xmm0              ; 02B2 _ 66: 0F 7E. 81, 000000D0
        psrldq  xmm0, 4                                 ; 02BA _ 66: 0F 73. D8, 04
        movups  xmmword ptr [ecx+0C0H], xmm1            ; 02BF _ 0F 11. 89, 000000C0
        movd    dword ptr [ecx+0D4H], xmm0              ; 02C6 _ 66: 0F 7E. 81, 000000D4
        ret                                             ; 02CE _ C3

.text$mn ENDS

.text$mn SEGMENT PARA PUBLIC 'CODE'
        ; Communal section not supported by MASM

_aesni_setkey_enc_256 LABEL NEAR
        mov     eax, dword ptr [esp+8H]                 ; 0000 _ 8B. 44 24, 08
        push    ebx                                     ; 0004 _ 53
        push    ebp                                     ; 0005 _ 55
        mov     ebp, dword ptr [esp+0CH]                ; 0006 _ 8B. 6C 24, 0C
        movups  xmm0, xmmword ptr [eax]                 ; 000A _ 0F 10. 00
        push    esi                                     ; 000D _ 56
        push    edi                                     ; 000E _ 57
        movups  xmmword ptr [ebp], xmm0                 ; 000F _ 0F 11. 45, 00
        lea     ecx, [ebp+30H]                          ; 0013 _ 8D. 4D, 30
        movups  xmm0, xmmword ptr [eax+10H]             ; 0016 _ 0F 10. 40, 10
        lea     esi, [ebp+20H]                          ; 001A _ 8D. 75, 20
        push    ecx                                     ; 001D _ 51
        movups  xmmword ptr [ebp+10H], xmm0             ; 001E _ 0F 11. 45, 10
        push    esi                                     ; 0022 _ 56
        movaps  xmm1, xmmword ptr [ebp+10H]             ; 0023 _ 0F 28. 4D, 10
        movaps  xmm0, xmmword ptr [ebp]                 ; 0027 _ 0F 28. 45, 00
        aeskeygenassist xmm2, xmm1, 01H                 ; 002B _ 66: 0F 3A DF. D1, 01
        call    _aesni_set_rk_256                       ; 0031 _ E8, 00000000(rel)
        movaps  xmm1, xmmword ptr [ecx]                 ; 0036 _ 0F 28. 09
        lea     edx, [ebp+50H]                          ; 0039 _ 8D. 55, 50
        movaps  xmm0, xmmword ptr [esi]                 ; 003C _ 0F 28. 06
        lea     edi, [ebp+40H]                          ; 003F _ 8D. 7D, 40
        push    edx                                     ; 0042 _ 52
        push    edi                                     ; 0043 _ 57
        aeskeygenassist xmm2, xmm1, 02H                 ; 0044 _ 66: 0F 3A DF. D1, 02
        call    _aesni_set_rk_256                       ; 004A _ E8, 00000000(rel)
        movaps  xmm1, xmmword ptr [edx]                 ; 004F _ 0F 28. 0A
        lea     ecx, [ebp+70H]                          ; 0052 _ 8D. 4D, 70
        movaps  xmm0, xmmword ptr [edi]                 ; 0055 _ 0F 28. 07
        lea     esi, [ebp+60H]                          ; 0058 _ 8D. 75, 60
        push    ecx                                     ; 005B _ 51
        push    esi                                     ; 005C _ 56
        aeskeygenassist xmm2, xmm1, 04H                 ; 005D _ 66: 0F 3A DF. D1, 04
        call    _aesni_set_rk_256                       ; 0063 _ E8, 00000000(rel)
        movaps  xmm1, xmmword ptr [ecx]                 ; 0068 _ 0F 28. 09
        lea     edx, [ebp+90H]                          ; 006B _ 8D. 95, 00000090
        movaps  xmm0, xmmword ptr [esi]                 ; 0071 _ 0F 28. 06
        lea     edi, [ebp+80H]                          ; 0074 _ 8D. BD, 00000080
        push    edx                                     ; 007A _ 52
        push    edi                                     ; 007B _ 57
        aeskeygenassist xmm2, xmm1, 08H                 ; 007C _ 66: 0F 3A DF. D1, 08
        call    _aesni_set_rk_256                       ; 0082 _ E8, 00000000(rel)
        movaps  xmm1, xmmword ptr [edx]                 ; 0087 _ 0F 28. 0A
        lea     esi, [ebp+0B0H]                         ; 008A _ 8D. B5, 000000B0
        movaps  xmm0, xmmword ptr [edi]                 ; 0090 _ 0F 28. 07
        lea     ebx, [ebp+0A0H]                         ; 0093 _ 8D. 9D, 000000A0
        push    esi                                     ; 0099 _ 56
        push    ebx                                     ; 009A _ 53
        aeskeygenassist xmm2, xmm1, 10H                 ; 009B _ 66: 0F 3A DF. D1, 10
        call    _aesni_set_rk_256                       ; 00A1 _ E8, 00000000(rel)
        movaps  xmm1, xmmword ptr [esi]                 ; 00A6 _ 0F 28. 0E
        lea     ecx, [ebp+0D0H]                         ; 00A9 _ 8D. 8D, 000000D0
        movaps  xmm0, xmmword ptr [ebx]                 ; 00AF _ 0F 28. 03
        lea     edx, [ebp+0C0H]                         ; 00B2 _ 8D. 95, 000000C0
        push    ecx                                     ; 00B8 _ 51
        push    edx                                     ; 00B9 _ 52
        aeskeygenassist xmm2, xmm1, 20H                 ; 00BA _ 66: 0F 3A DF. D1, 20
        call    _aesni_set_rk_256                       ; 00C0 _ E8, 00000000(rel)
        movaps  xmm1, xmmword ptr [ecx]                 ; 00C5 _ 0F 28. 09
        lea     eax, [ebp+0F0H]                         ; 00C8 _ 8D. 85, 000000F0
        movaps  xmm0, xmmword ptr [edx]                 ; 00CE _ 0F 28. 02
        push    eax                                     ; 00D1 _ 50
        lea     eax, [ebp+0E0H]                         ; 00D2 _ 8D. 85, 000000E0
        push    eax                                     ; 00D8 _ 50
        aeskeygenassist xmm2, xmm1, 40H                 ; 00D9 _ 66: 0F 3A DF. D1, 40
        call    _aesni_set_rk_256                       ; 00DF _ E8, 00000000(rel)
        add     esp, 56                                 ; 00E4 _ 83. C4, 38
        pop     edi                                     ; 00E7 _ 5F
        pop     esi                                     ; 00E8 _ 5E
        pop     ebp                                     ; 00E9 _ 5D
        pop     ebx                                     ; 00EA _ 5B
        ret                                             ; 00EB _ C3

.text$mn ENDS

.text$mn SEGMENT PARA PUBLIC 'CODE'
        ; Communal section not supported by MASM

_gcm_clmul LABEL NEAR
        mov     ecx, dword ptr [esp+4H]                 ; 0000 _ 8B. 4C 24, 04
        movaps  xmm4, xmm0                              ; 0004 _ 0F 28. E0
        mov     eax, dword ptr [esp+8H]                 ; 0007 _ 8B. 44 24, 08
        movaps  xmm2, xmm4                              ; 000B _ 0F 28. D4
        pclmulqdq xmm2, xmm1, 00H                       ; 000E _ 66: 0F 3A 44. D1, 00
        movaps  xmm3, xmm4                              ; 0014 _ 0F 28. DC
        movaps  xmmword ptr [ecx], xmm2                 ; 0017 _ 0F 29. 11
        movaps  xmm2, xmm4                              ; 001A _ 0F 28. D4
        pclmulqdq xmm3, xmm1, 10H                       ; 001D _ 66: 0F 3A 44. D9, 10
        pclmulqdq xmm2, xmm1, 01H                       ; 0023 _ 66: 0F 3A 44. D1, 01
        pxor    xmm3, xmm2                              ; 0029 _ 66: 0F EF. DA
        movaps  xmm0, xmm3                              ; 002D _ 0F 28. C3
        pslldq  xmm3, 8                                 ; 0030 _ 66: 0F 73. FB, 08
        pclmulqdq xmm4, xmm1, 11H                       ; 0035 _ 66: 0F 3A 44. E1, 11
        psrldq  xmm0, 8                                 ; 003B _ 66: 0F 73. D8, 08
        pxor    xmm4, xmm0                              ; 0040 _ 66: 0F EF. E0
        movaps  xmmword ptr [eax], xmm4                 ; 0044 _ 0F 29. 20
        pxor    xmm3, xmmword ptr [ecx]                 ; 0047 _ 66: 0F EF. 19
        movaps  xmmword ptr [ecx], xmm3                 ; 004B _ 0F 29. 19
        ret                                             ; 004E _ C3

.text$mn ENDS

.text$mn SEGMENT PARA PUBLIC 'CODE'
        ; Communal section not supported by MASM

_gcm_mix LABEL NEAR
        movaps  xmm3, xmm0                              ; 0000 _ 0F 28. D8
        psllq   xmm0, 63                                ; 0003 _ 66: 0F 73. F0, 3F
        movaps  xmm1, xmm3                              ; 0008 _ 0F 28. CB
        movaps  xmm2, xmm3                              ; 000B _ 0F 28. D3
        psllq   xmm1, 62                                ; 000E _ 66: 0F 73. F1, 3E
        psrlq   xmm2, 2                                 ; 0013 _ 66: 0F 73. D2, 02
        pxor    xmm0, xmm1                              ; 0018 _ 66: 0F EF. C1
        movaps  xmm1, xmm3                              ; 001C _ 0F 28. CB
        psllq   xmm1, 57                                ; 001F _ 66: 0F 73. F1, 39
        pxor    xmm0, xmm1                              ; 0024 _ 66: 0F EF. C1
        movaps  xmm1, xmm3                              ; 0028 _ 0F 28. CB
        psrlq   xmm1, 1                                 ; 002B _ 66: 0F 73. D1, 01
        psrldq  xmm0, 8                                 ; 0030 _ 66: 0F 73. D8, 08
        pxor    xmm2, xmm1                              ; 0035 _ 66: 0F EF. D1
        movaps  xmm1, xmm3                              ; 0039 _ 0F 28. CB
        psrlq   xmm1, 7                                 ; 003C _ 66: 0F 73. D1, 07
        pxor    xmm2, xmm1                              ; 0041 _ 66: 0F EF. D1
        pxor    xmm0, xmm2                              ; 0045 _ 66: 0F EF. C2
        pxor    xmm0, xmm3                              ; 0049 _ 66: 0F EF. C3
        ret                                             ; 004D _ C3

.text$mn ENDS

.text$mn SEGMENT PARA PUBLIC 'CODE'
        ; Communal section not supported by MASM

_gcm_reduce LABEL NEAR
        movaps  xmm2, xmm0                              ; 0000 _ 0F 28. D0
        psllq   xmm0, 63                                ; 0003 _ 66: 0F 73. F0, 3F
        movaps  xmm1, xmm2                              ; 0008 _ 0F 28. CA
        psllq   xmm1, 62                                ; 000B _ 66: 0F 73. F1, 3E
        pxor    xmm0, xmm1                              ; 0010 _ 66: 0F EF. C1
        movaps  xmm1, xmm2                              ; 0014 _ 0F 28. CA
        psllq   xmm1, 57                                ; 0017 _ 66: 0F 73. F1, 39
        pxor    xmm0, xmm1                              ; 001C _ 66: 0F EF. C1
        pslldq  xmm0, 8                                 ; 0020 _ 66: 0F 73. F8, 08
        pxor    xmm0, xmm2                              ; 0025 _ 66: 0F EF. C2
        ret                                             ; 0029 _ C3

.text$mn ENDS

.text$mn SEGMENT PARA PUBLIC 'CODE'
        ; Communal section not supported by MASM

_gcm_shift LABEL NEAR
        mov     eax, dword ptr [esp+4H]                 ; 0000 _ 8B. 44 24, 04
        mov     ecx, dword ptr [esp+8H]                 ; 0004 _ 8B. 4C 24, 08
        movaps  xmm1, xmmword ptr [eax]                 ; 0008 _ 0F 28. 08
        movaps  xmm4, xmmword ptr [ecx]                 ; 000B _ 0F 28. 21
        movaps  xmm2, xmm1                              ; 000E _ 0F 28. D1
        movaps  xmm3, xmm4                              ; 0011 _ 0F 28. DC
        psrlq   xmm2, 63                                ; 0014 _ 66: 0F 73. D2, 3F
        psrlq   xmm4, 63                                ; 0019 _ 66: 0F 73. D4, 3F
        movaps  xmm0, xmm2                              ; 001E _ 0F 28. C2
        psllq   xmm3, 1                                 ; 0021 _ 66: 0F 73. F3, 01
        pslldq  xmm4, 8                                 ; 0026 _ 66: 0F 73. FC, 08
        psllq   xmm1, 1                                 ; 002B _ 66: 0F 73. F1, 01
        por     xmm4, xmm3                              ; 0030 _ 66: 0F EB. E3
        pslldq  xmm0, 8                                 ; 0034 _ 66: 0F 73. F8, 08
        psrldq  xmm2, 8                                 ; 0039 _ 66: 0F 73. DA, 08
        por     xmm1, xmm0                              ; 003E _ 66: 0F EB. C8
        por     xmm4, xmm2                              ; 0042 _ 66: 0F EB. E2
        movaps  xmmword ptr [eax], xmm1                 ; 0046 _ 0F 29. 08
        movaps  xmmword ptr [ecx], xmm4                 ; 0049 _ 0F 29. 21
        ret                                             ; 004C _ C3

.text$mn ENDS

.text$mn SEGMENT PARA PUBLIC 'CODE'
        ; Communal section not supported by MASM

_mbedtls_aesni_crypt_ecb PROC NEAR
        mov     edx, dword ptr [esp+4H]                 ; 0000 _ 8B. 54 24, 04
        mov     ecx, dword ptr [esp+0CH]                ; 0004 _ 8B. 4C 24, 0C
        mov     eax, dword ptr [edx+4H]                 ; 0008 _ 8B. 42, 04
        movups  xmm0, xmmword ptr [ecx]                 ; 000B _ 0F 10. 01
        lea     eax, [edx+eax*4]                        ; 000E _ 8D. 04 82
        mov     ecx, dword ptr [edx]                    ; 0011 _ 8B. 0A
        movaps  xmm1, xmmword ptr [eax+8H]              ; 0013 _ 0F 28. 48, 08
        dec     ecx                                     ; 0017 _ 49
        add     eax, 24                                 ; 0018 _ 83. C0, 18
        pxor    xmm1, xmm0                              ; 001B _ 66: 0F EF. C8
        cmp     dword ptr [esp+8H], 0                   ; 001F _ 83. 7C 24, 08, 00
        jnz     ?_003                                   ; 0024 _ 75, 26
        test    ecx, ecx                                ; 0026 _ 85. C9
        jz      ?_002                                   ; 0028 _ 74, 13
                ; Filling space: 6H
                ; Filler type: Multi-byte NOP
                ;       db 66H, 0FH, 1FH, 44H, 00H, 00H

ALIGN   8
?_001:  aesdec  xmm1, xmmword ptr [eax]                 ; 0030 _ 66: 0F 38 DE. 08
        add     eax, 16                                 ; 0035 _ 83. C0, 10
        sub     ecx, 1                                  ; 0038 _ 83. E9, 01
        jnz     ?_001                                   ; 003B _ 75, F3
?_002:  aesdeclast xmm1, xmmword ptr [eax]              ; 003D _ 66: 0F 38 DF. 08
        mov     eax, dword ptr [esp+10H]                ; 0042 _ 8B. 44 24, 10
        movups  xmmword ptr [eax], xmm1                 ; 0046 _ 0F 11. 08
        xor     eax, eax                                ; 0049 _ 33. C0
        ret                                             ; 004B _ C3
_mbedtls_aesni_crypt_ecb ENDP

?_003   LABEL NEAR
        test    ecx, ecx                                ; 004C _ 85. C9
        jz      ?_005                                   ; 004E _ 74, 0D
?_004:  aesenc  xmm1, xmmword ptr [eax]                 ; 0050 _ 66: 0F 38 DC. 08
        add     eax, 16                                 ; 0055 _ 83. C0, 10
        sub     ecx, 1                                  ; 0058 _ 83. E9, 01
        jnz     ?_004                                   ; 005B _ 75, F3
?_005:  aesenclast xmm1, xmmword ptr [eax]              ; 005D _ 66: 0F 38 DD. 08
        mov     eax, dword ptr [esp+10H]                ; 0062 _ 8B. 44 24, 10
        movups  xmmword ptr [eax], xmm1                 ; 0066 _ 0F 11. 08
        xor     eax, eax                                ; 0069 _ 33. C0
        ret                                             ; 006B _ C3

.text$mn ENDS

.text$mn SEGMENT PARA PUBLIC 'CODE'
        ; Communal section not supported by MASM

_mbedtls_aesni_gcm_mult PROC NEAR
        push    ebp                                     ; 0000 _ 55
        mov     ebp, esp                                ; 0001 _ 8B. EC
        and     esp, 0FFFFFFF0H                         ; 0003 _ 83. E4, F0
        mov     ecx, dword ptr [ebp+10H]                ; 0006 _ 8B. 4D, 10
        sub     esp, 44                                 ; 0009 _ 83. EC, 2C
        xor     eax, eax                                ; 000C _ 33. C0
        push    esi                                     ; 000E _ 56
        mov     esi, dword ptr [ebp+0CH]                ; 000F _ 8B. 75, 0C
        lea     edx, [ecx+0FH]                          ; 0012 _ 8D. 51, 0F
        sub     esi, ecx                                ; 0015 _ 2B. F1
                ; Filling space: 9H
                ; Filler type: Multi-byte NOP
                ;       db 66H, 0FH, 1FH, 84H, 00H, 00H, 00H, 00H
                ;       db 00H

ALIGN   16
?_006:  movzx   ecx, byte ptr [esi+edx]                 ; 0020 _ 0F B6. 0C 16
        lea     edx, [edx-1H]                           ; 0024 _ 8D. 52, FF
        mov     byte ptr [esp+eax+10H], cl              ; 0027 _ 88. 4C 04, 10
        movzx   ecx, byte ptr [edx+1H]                  ; 002B _ 0F B6. 4A, 01
        mov     byte ptr [esp+eax+20H], cl              ; 002F _ 88. 4C 04, 20
        inc     eax                                     ; 0033 _ 40
        cmp     eax, 16                                 ; 0034 _ 83. F8, 10
        jc      ?_006                                   ; 0037 _ 72, E7
        movaps  xmm3, xmmword ptr [esp+10H]             ; 0039 _ 0F 28. 5C 24, 10
        lea     edx, [esp+2FH]                          ; 003E _ 8D. 54 24, 2F
        movaps  xmm1, xmmword ptr [esp+20H]             ; 0042 _ 0F 28. 4C 24, 20
        movaps  xmm2, xmm3                              ; 0047 _ 0F 28. D3
        pclmulqdq xmm2, xmm1, 10H                       ; 004A _ 66: 0F 3A 44. D1, 10
        movaps  xmm0, xmm3                              ; 0050 _ 0F 28. C3
        mov     esi, dword ptr [ebp+8H]                 ; 0053 _ 8B. 75, 08
        pclmulqdq xmm0, xmm1, 01H                       ; 0056 _ 66: 0F 3A 44. C1, 01
        pxor    xmm2, xmm0                              ; 005C _ 66: 0F EF. D0
        movaps  xmm5, xmm3                              ; 0060 _ 0F 28. EB
        pclmulqdq xmm3, xmm1, 00H                       ; 0063 _ 66: 0F 3A 44. D9, 00
        movaps  xmm0, xmm2                              ; 0069 _ 0F 28. C2
        xor     ecx, ecx                                ; 006C _ 33. C9
        pslldq  xmm2, 8                                 ; 006E _ 66: 0F 73. FA, 08
        pxor    xmm3, xmm2                              ; 0073 _ 66: 0F EF. DA
        psrldq  xmm0, 8                                 ; 0077 _ 66: 0F 73. D8, 08
        movaps  xmm4, xmm3                              ; 007C _ 0F 28. E3
        psllq   xmm3, 1                                 ; 007F _ 66: 0F 73. F3, 01
        pclmulqdq xmm5, xmm1, 11H                       ; 0084 _ 66: 0F 3A 44. E9, 11
        pxor    xmm5, xmm0                              ; 008A _ 66: 0F EF. E8
        psrlq   xmm4, 63                                ; 008E _ 66: 0F 73. D4, 3F
        movaps  xmm1, xmm4                              ; 0093 _ 0F 28. CC
        psrldq  xmm4, 8                                 ; 0096 _ 66: 0F 73. DC, 08
        pslldq  xmm1, 8                                 ; 009B _ 66: 0F 73. F9, 08
        por     xmm1, xmm3                              ; 00A0 _ 66: 0F EB. CB
        movaps  xmm2, xmm1                              ; 00A4 _ 0F 28. D1
        movaps  xmm0, xmm1                              ; 00A7 _ 0F 28. C1
        psllq   xmm0, 62                                ; 00AA _ 66: 0F 73. F0, 3E
        psllq   xmm2, 63                                ; 00AF _ 66: 0F 73. F2, 3F
        pxor    xmm2, xmm0                              ; 00B4 _ 66: 0F EF. D0
        movaps  xmm0, xmm1                              ; 00B8 _ 0F 28. C1
        psllq   xmm0, 57                                ; 00BB _ 66: 0F 73. F0, 39
        pxor    xmm2, xmm0                              ; 00C0 _ 66: 0F EF. D0
        pslldq  xmm2, 8                                 ; 00C4 _ 66: 0F 73. FA, 08
        pxor    xmm2, xmm1                              ; 00C9 _ 66: 0F EF. D1
        movaps  xmm3, xmm2                              ; 00CD _ 0F 28. DA
        movaps  xmm0, xmm2                              ; 00D0 _ 0F 28. C2
        psllq   xmm0, 62                                ; 00D3 _ 66: 0F 73. F0, 3E
        movaps  xmm1, xmm2                              ; 00D8 _ 0F 28. CA
        psllq   xmm3, 63                                ; 00DB _ 66: 0F 73. F3, 3F
        psrlq   xmm1, 2                                 ; 00E0 _ 66: 0F 73. D1, 02
        pxor    xmm3, xmm0                              ; 00E5 _ 66: 0F EF. D8
        movaps  xmm0, xmm2                              ; 00E9 _ 0F 28. C2
        psllq   xmm0, 57                                ; 00EC _ 66: 0F 73. F0, 39
        pxor    xmm3, xmm0                              ; 00F1 _ 66: 0F EF. D8
        movaps  xmm0, xmm2                              ; 00F5 _ 0F 28. C2
        psrlq   xmm0, 1                                 ; 00F8 _ 66: 0F 73. D0, 01
        psrldq  xmm3, 8                                 ; 00FD _ 66: 0F 73. DB, 08
        pxor    xmm1, xmm0                              ; 0102 _ 66: 0F EF. C8
        movaps  xmm0, xmm2                              ; 0106 _ 0F 28. C2
        psrlq   xmm0, 7                                 ; 0109 _ 66: 0F 73. D0, 07
        pxor    xmm1, xmm0                              ; 010E _ 66: 0F EF. C8
        movaps  xmm0, xmm5                              ; 0112 _ 0F 28. C5
        psrlq   xmm0, 63                                ; 0115 _ 66: 0F 73. D0, 3F
        pxor    xmm3, xmm1                              ; 011A _ 66: 0F EF. D9
        psllq   xmm5, 1                                 ; 011E _ 66: 0F 73. F5, 01
        pxor    xmm3, xmm2                              ; 0123 _ 66: 0F EF. DA
        pslldq  xmm0, 8                                 ; 0127 _ 66: 0F 73. F8, 08
        por     xmm0, xmm5                              ; 012C _ 66: 0F EB. C5
        por     xmm0, xmm4                              ; 0130 _ 66: 0F EB. C4
        pxor    xmm3, xmm0                              ; 0134 _ 66: 0F EF. D8
        movaps  xmmword ptr [esp+20H], xmm3             ; 0138 _ 0F 29. 5C 24, 20
                ; Filling space: 3H
                ; Filler type: Multi-byte NOP
                ;       db 0FH, 1FH, 00H

ALIGN   8
?_007:  mov     al, byte ptr [edx]                      ; 0140 _ 8A. 02
        lea     edx, [edx-1H]                           ; 0142 _ 8D. 52, FF
        mov     byte ptr [ecx+esi], al                  ; 0145 _ 88. 04 31
        inc     ecx                                     ; 0148 _ 41
        cmp     ecx, 16                                 ; 0149 _ 83. F9, 10
        jc      ?_007                                   ; 014C _ 72, F2
        pop     esi                                     ; 014E _ 5E
        mov     esp, ebp                                ; 014F _ 8B. E5
        pop     ebp                                     ; 0151 _ 5D
        ret                                             ; 0152 _ C3
_mbedtls_aesni_gcm_mult ENDP

.text$mn ENDS

.text$mn SEGMENT PARA PUBLIC 'CODE'
        ; Communal section not supported by MASM

_mbedtls_aesni_has_support PROC NEAR
        cmp     dword ptr [aesni_has_support_done], 0   ; 0000 _ 83. 3D, 00000000(d), 00
        jnz     ?_008                                   ; 0007 _ 75, 3E
        xor     ecx, ecx                                ; 0009 _ 33. C9
        mov     dword ptr [aesni_has_support_done], 1   ; 000B _ C7. 05, 00000000(d), 00000001
        push    ebx                                     ; 0015 _ 53
        mov     eax, 1                                  ; 0016 _ B8, 00000001
        cpuid                                           ; 001B _ 0F A2
        mov     dword ptr [aesni_has_support_info], eax ; 001D _ A3, 00000000(d)
        mov     eax, ecx                                ; 0022 _ 8B. C1
        mov     dword ptr [aesni_has_support_flag], eax ; 0024 _ A3, 00000000(d)
        and     eax, dword ptr [esp+8H]                 ; 0029 _ 23. 44 24, 08
        neg     eax                                     ; 002D _ F7. D8
        mov     dword ptr [aesni_has_support_info], ebx ; 002F _ 89. 1D, 00000004(d)
        mov     dword ptr [aesni_has_support_info], ecx ; 0035 _ 89. 0D, 00000008(d)
        sbb     eax, eax                                ; 003B _ 1B. C0
        mov     dword ptr [aesni_has_support_info], edx ; 003D _ 89. 15, 0000000C(d)
        pop     ebx                                     ; 0043 _ 5B
        neg     eax                                     ; 0044 _ F7. D8
        ret                                             ; 0046 _ C3
_mbedtls_aesni_has_support ENDP

?_008   LABEL NEAR
        mov     eax, dword ptr [aesni_has_support_flag] ; 0047 _ A1, 00000000(d)
        and     eax, dword ptr [esp+4H]                 ; 004C _ 23. 44 24, 04
        neg     eax                                     ; 0050 _ F7. D8
        sbb     eax, eax                                ; 0052 _ 1B. C0
        neg     eax                                     ; 0054 _ F7. D8
        ret                                             ; 0056 _ C3

.text$mn ENDS

.text$mn SEGMENT PARA PUBLIC 'CODE'
        ; Communal section not supported by MASM

_mbedtls_aesni_inverse_key PROC NEAR
        mov     eax, dword ptr [esp+0CH]                ; 0000 _ 8B. 44 24, 0C
        mov     edx, dword ptr [esp+8H]                 ; 0004 _ 8B. 54 24, 08
        mov     ecx, dword ptr [esp+4H]                 ; 0008 _ 8B. 4C 24, 04
        shl     eax, 4                                  ; 000C _ C1. E0, 04
        add     eax, edx                                ; 000F _ 03. C2
        movaps  xmm0, xmmword ptr [eax]                 ; 0011 _ 0F 28. 00
        sub     eax, 16                                 ; 0014 _ 83. E8, 10
        movaps  xmmword ptr [ecx], xmm0                 ; 0017 _ 0F 29. 01
        add     ecx, 16                                 ; 001A _ 83. C1, 10
        cmp     eax, edx                                ; 001D _ 3B. C2
        jbe     ?_010                                   ; 001F _ 76, 12
?_009:  aesimc  xmm0, xmmword ptr [eax]                 ; 0021 _ 66: 0F 38 DB. 00
        sub     eax, 16                                 ; 0026 _ 83. E8, 10
        movaps  xmmword ptr [ecx], xmm0                 ; 0029 _ 0F 29. 01
        lea     ecx, [ecx+10H]                          ; 002C _ 8D. 49, 10
        cmp     eax, edx                                ; 002F _ 3B. C2
        ja      ?_009                                   ; 0031 _ 77, EE
?_010:  movaps  xmm0, xmmword ptr [eax]                 ; 0033 _ 0F 28. 00
        movaps  xmmword ptr [ecx], xmm0                 ; 0036 _ 0F 29. 01
        ret                                             ; 0039 _ C3
_mbedtls_aesni_inverse_key ENDP

.text$mn ENDS

.text$mn SEGMENT PARA PUBLIC 'CODE'
        ; Communal section not supported by MASM

_mbedtls_aesni_setkey_enc PROC NEAR
        mov     eax, dword ptr [esp+0CH]                ; 0000 _ 8B. 44 24, 0C
        sub     eax, 128                                ; 0004 _ 2D, 00000080
        jz      ?_013                                   ; 0009 _ 74, 36
        sub     eax, 64                                 ; 000B _ 83. E8, 40
        jz      ?_012                                   ; 000E _ 74, 1E
        sub     eax, 64                                 ; 0010 _ 83. E8, 40
        jz      ?_011                                   ; 0013 _ 74, 06
        mov     eax, 4294967264                         ; 0015 _ B8, FFFFFFE0
        ret                                             ; 001A _ C3

?_011:  push    dword ptr [esp+8H]                      ; 001B _ FF. 74 24, 08
        push    dword ptr [esp+8H]                      ; 001F _ FF. 74 24, 08
        call    _aesni_setkey_enc_256                   ; 0023 _ E8, 00000000(rel)
        add     esp, 8                                  ; 0028 _ 83. C4, 08
        xor     eax, eax                                ; 002B _ 33. C0
        ret                                             ; 002D _ C3

?_012:  push    dword ptr [esp+8H]                      ; 002E _ FF. 74 24, 08
        push    dword ptr [esp+8H]                      ; 0032 _ FF. 74 24, 08
        call    _aesni_setkey_enc_192                   ; 0036 _ E8, 00000000(rel)
        add     esp, 8                                  ; 003B _ 83. C4, 08
        xor     eax, eax                                ; 003E _ 33. C0
        ret                                             ; 0040 _ C3
_mbedtls_aesni_setkey_enc ENDP

?_013   LABEL NEAR
        push    dword ptr [esp+8H]                      ; 0041 _ FF. 74 24, 08
        push    dword ptr [esp+8H]                      ; 0045 _ FF. 74 24, 08
        call    _aesni_setkey_enc_128                   ; 0049 _ E8, 00000000(rel)
        add     esp, 8                                  ; 004E _ 83. C4, 08
        xor     eax, eax                                ; 0051 _ 33. C0
        ret                                             ; 0053 _ C3

.text$mn ENDS

END
