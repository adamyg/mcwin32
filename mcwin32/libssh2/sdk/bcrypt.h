//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 2004.
//
//  File:       bcrypt.h
//
//  Contents:   Cryptographic Primitive API Prototypes and Definitions
//
//----------------------------------------------------------------------------

#ifndef __BCRYPT_H__
#define __BCRYPT_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WINAPI
#define WINAPI __stdcall
#endif

#ifndef _NTDEF_
typedef __success(return >= 0) LONG NTSTATUS;
typedef NTSTATUS *PNTSTATUS;
#endif

#ifndef BCRYPT_SUCCESS
#define BCRYPT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

#ifndef CONST
#define CONST const
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

//
//  Alignment macros
//

//
// BCRYPT_OBJECT_ALIGNMENT must be a power of 2
// We align all our internal data structures to 16 to
// allow fast XMM memory accesses.
// BCrypt callers do not need to take any alignment precautions.
//
#define BCRYPT_OBJECT_ALIGNMENT    16

//
// BCRYPT_STRUCT_ALIGNMENT is an alignment macro that we no longer use.
// It used to align declspec(align(4)) on x86 and declspec(align(8)) on x64/ia64 but
// all structures that used it contained a pointer so they were already 4/8 aligned.
//
#define BCRYPT_STRUCT_ALIGNMENT

//
// DeriveKey KDF Types
//
#define BCRYPT_KDF_HASH                     L"HASH"
#define BCRYPT_KDF_HMAC                     L"HMAC"
#define BCRYPT_KDF_TLS_PRF                  L"TLS_PRF"
#define BCRYPT_KDF_SP80056A_CONCAT          L"SP800_56A_CONCAT"

//
// DeriveKey KDF BufferTypes
//
// For BCRYPT_KDF_HASH and BCRYPT_KDF_HMAC operations, there may be an arbitrary
// number of KDF_SECRET_PREPEND and KDF_SECRET_APPEND buffertypes in the
// parameter list.  The BufferTypes are processed in order of appearence
// within the parameter list.
//
#define KDF_HASH_ALGORITHM      0x0
#define KDF_SECRET_PREPEND      0x1
#define KDF_SECRET_APPEND       0x2
#define KDF_HMAC_KEY            0x3
#define KDF_TLS_PRF_LABEL       0x4
#define KDF_TLS_PRF_SEED        0x5
#define KDF_SECRET_HANDLE       0x6
#define KDF_TLS_PRF_PROTOCOL    0x7
#define KDF_ALGORITHMID         0x8
#define KDF_PARTYUINFO          0x9
#define KDF_PARTYVINFO          0xA
#define KDF_SUPPPUBINFO         0xB
#define KDF_SUPPPRIVINFO        0xC

//
// DeriveKey Flags:
//
// KDF_USE_SECRET_AS_HMAC_KEY_FLAG causes the secret agreement to serve also
// as the HMAC key.  If this flag is used, the KDF_HMAC_KEY parameter should
// NOT be specified.
//
#define KDF_USE_SECRET_AS_HMAC_KEY_FLAG 0x1

//
// BCrypt structs
//

typedef struct __BCRYPT_KEY_LENGTHS_STRUCT
{
    ULONG   dwMinLength;
    ULONG   dwMaxLength;
    ULONG   dwIncrement;
} BCRYPT_KEY_LENGTHS_STRUCT;

typedef BCRYPT_KEY_LENGTHS_STRUCT BCRYPT_AUTH_TAG_LENGTHS_STRUCT;

typedef struct _BCRYPT_OID
{
    ULONG   cbOID;
    PUCHAR  pbOID;
} BCRYPT_OID;

typedef struct _BCRYPT_OID_LIST
{
    ULONG       dwOIDCount;
    BCRYPT_OID  *pOIDs;
} BCRYPT_OID_LIST;

typedef struct _BCRYPT_PKCS1_PADDING_INFO
{
    LPCWSTR pszAlgId;
} BCRYPT_PKCS1_PADDING_INFO;

typedef struct _BCRYPT_PSS_PADDING_INFO
{
    LPCWSTR pszAlgId;
    ULONG   cbSalt;
} BCRYPT_PSS_PADDING_INFO;

typedef struct _BCRYPT_OAEP_PADDING_INFO
{
    LPCWSTR pszAlgId;
    PUCHAR   pbLabel;
    ULONG   cbLabel;
} BCRYPT_OAEP_PADDING_INFO;

#define BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO_VERSION  1

#define BCRYPT_AUTH_MODE_CHAIN_CALLS_FLAG   0x00000001
#define BCRYPT_AUTH_MODE_IN_PROGRESS_FLAG   0x00000002

typedef struct _BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO
{
    ULONG       cbSize;
    ULONG       dwInfoVersion;
    PUCHAR      pbNonce;
    ULONG       cbNonce;
    PUCHAR      pbAuthData;
    ULONG       cbAuthData;
    PUCHAR      pbTag;
    ULONG       cbTag;
    PUCHAR      pbMacContext;
    ULONG       cbMacContext;
    ULONG       cbAAD;
    ULONGLONG   cbData;
    ULONG       dwFlags;
} BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO, *PBCRYPT_AUTHENTICATED_CIPHER_MODE_INFO;

#define BCRYPT_INIT_AUTH_MODE_INFO(_AUTH_INFO_STRUCT_)    \
            RtlZeroMemory((&_AUTH_INFO_STRUCT_), sizeof(BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO));  \
            (_AUTH_INFO_STRUCT_).cbSize = sizeof(BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO);          \
            (_AUTH_INFO_STRUCT_).dwInfoVersion = BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO_VERSION;

//
// BCrypt String Properties
//

// BCrypt(Import/Export)Key BLOB types
#define BCRYPT_OPAQUE_KEY_BLOB      L"OpaqueKeyBlob"
#define BCRYPT_KEY_DATA_BLOB        L"KeyDataBlob"
#define BCRYPT_AES_WRAP_KEY_BLOB    L"Rfc3565KeyWrapBlob"

// BCryptGetProperty strings
#define BCRYPT_OBJECT_LENGTH        L"ObjectLength"
#define BCRYPT_ALGORITHM_NAME       L"AlgorithmName"
#define BCRYPT_PROVIDER_HANDLE      L"ProviderHandle"
#define BCRYPT_CHAINING_MODE        L"ChainingMode"
#define BCRYPT_BLOCK_LENGTH         L"BlockLength"
#define BCRYPT_KEY_LENGTH           L"KeyLength"
#define BCRYPT_KEY_OBJECT_LENGTH    L"KeyObjectLength"
#define BCRYPT_KEY_STRENGTH         L"KeyStrength"
#define BCRYPT_KEY_LENGTHS          L"KeyLengths"
#define BCRYPT_BLOCK_SIZE_LIST      L"BlockSizeList"
#define BCRYPT_EFFECTIVE_KEY_LENGTH L"EffectiveKeyLength"
#define BCRYPT_HASH_LENGTH          L"HashDigestLength"
#define BCRYPT_HASH_OID_LIST        L"HashOIDList"
#define BCRYPT_PADDING_SCHEMES      L"PaddingSchemes"
#define BCRYPT_SIGNATURE_LENGTH     L"SignatureLength"
#define BCRYPT_HASH_BLOCK_LENGTH    L"HashBlockLength"
#define BCRYPT_AUTH_TAG_LENGTH      L"AuthTagLength"
#define BCRYPT_PRIMITIVE_TYPE       L"PrimitiveType"
#define BCRYPT_IS_KEYED_HASH        L"IsKeyedHash"

// BCryptSetProperty strings
#define BCRYPT_INITIALIZATION_VECTOR    L"IV"


// Property Strings
#define BCRYPT_CHAIN_MODE_NA        L"ChainingModeN/A"
#define BCRYPT_CHAIN_MODE_CBC       L"ChainingModeCBC"
#define BCRYPT_CHAIN_MODE_ECB       L"ChainingModeECB"
#define BCRYPT_CHAIN_MODE_CFB       L"ChainingModeCFB"
#define BCRYPT_CHAIN_MODE_CCM       L"ChainingModeCCM"
#define BCRYPT_CHAIN_MODE_GCM       L"ChainingModeGCM"

// Supported RSA Padding Types
#define BCRYPT_SUPPORTED_PAD_ROUTER     0x00000001
#define BCRYPT_SUPPORTED_PAD_PKCS1_ENC  0x00000002
#define BCRYPT_SUPPORTED_PAD_PKCS1_SIG  0x00000004
#define BCRYPT_SUPPORTED_PAD_OAEP       0x00000008
#define BCRYPT_SUPPORTED_PAD_PSS        0x00000010

//
//      BCrypt Flags
//

#define BCRYPT_PROV_DISPATCH        0x00000001  // BCryptOpenAlgorithmProvider

#define BCRYPT_BLOCK_PADDING        0x00000001  // BCryptEncrypt/Decrypt

// RSA padding schemes
#define BCRYPT_PAD_NONE             0x00000001
#define BCRYPT_PAD_PKCS1            0x00000002  // BCryptEncrypt/Decrypt BCryptSignHash/VerifySignature
#define BCRYPT_PAD_OAEP             0x00000004  // BCryptEncrypt/Decrypt
#define BCRYPT_PAD_PSS              0x00000008  // BCryptSignHash/VerifySignature


#define BCRYPTBUFFER_VERSION        0

typedef struct _BCryptBuffer {
    ULONG   cbBuffer;             // Length of buffer, in bytes
    ULONG   BufferType;           // Buffer type
    PVOID   pvBuffer;             // Pointer to buffer
} BCryptBuffer, * PBCryptBuffer;

typedef struct _BCryptBufferDesc {
    ULONG   ulVersion;            // Version number
    ULONG   cBuffers;             // Number of buffers
    PBCryptBuffer pBuffers;       // Pointer to array of buffers
} BCryptBufferDesc, * PBCryptBufferDesc;


//
// Primitive handles
//

typedef PVOID BCRYPT_HANDLE;
typedef PVOID BCRYPT_ALG_HANDLE;
typedef PVOID BCRYPT_KEY_HANDLE;
typedef PVOID BCRYPT_HASH_HANDLE;
typedef PVOID BCRYPT_SECRET_HANDLE;


//
// Structures used to represent key blobs.
//

#define BCRYPT_PUBLIC_KEY_BLOB       L"PUBLICBLOB"
#define BCRYPT_PRIVATE_KEY_BLOB      L"PRIVATEBLOB"

typedef struct _BCRYPT_KEY_BLOB
{
    ULONG   Magic;
} BCRYPT_KEY_BLOB;

// The BCRYPT_RSAPUBLIC_BLOB and BCRYPT_RSAPRIVATE_BLOB blob types are used
// to transport plaintext RSA keys. These blob types will be supported by
// all RSA primitive providers.
// The BCRYPT_RSAPRIVATE_BLOB includes the following values:
// Public Exponent
// Modulus
// Prime1
// Prime2

#define BCRYPT_RSAPUBLIC_BLOB       L"RSAPUBLICBLOB"
#define BCRYPT_RSAPRIVATE_BLOB      L"RSAPRIVATEBLOB"
#define LEGACY_RSAPUBLIC_BLOB       L"CAPIPUBLICBLOB"
#define LEGACY_RSAPRIVATE_BLOB      L"CAPIPRIVATEBLOB"

#define BCRYPT_RSAPUBLIC_MAGIC      0x31415352  // RSA1
#define BCRYPT_RSAPRIVATE_MAGIC     0x32415352  // RSA2

typedef struct _BCRYPT_RSAKEY_BLOB
{
    ULONG   Magic;
    ULONG   BitLength;
    ULONG   cbPublicExp;
    ULONG   cbModulus;
    ULONG   cbPrime1;
    ULONG   cbPrime2;
} BCRYPT_RSAKEY_BLOB;

// The BCRYPT_RSAFULLPRIVATE_BLOB blob type is used to transport
// plaintext private RSA keys.  It includes the following values:
// Public Exponent
// Modulus
// Prime1
// Prime2
// Private Exponent mod (Prime1 - 1)
// Private Exponent mod (Prime2 - 1)
// Inverse of Prime2 mod Prime1
// PrivateExponent
#define BCRYPT_RSAFULLPRIVATE_BLOB      L"RSAFULLPRIVATEBLOB"

#define BCRYPT_RSAFULLPRIVATE_MAGIC     0x33415352  // RSA3

// The BCRYPT_ECCPUBLIC_BLOB and BCRYPT_ECCPRIVATE_BLOB blob types are used
// to transport plaintext ECC keys. These blob types will be supported by
// all ECC primitive providers.
#define BCRYPT_ECCPUBLIC_BLOB           L"ECCPUBLICBLOB"
#define BCRYPT_ECCPRIVATE_BLOB          L"ECCPRIVATEBLOB"

#define BCRYPT_ECDH_PUBLIC_P256_MAGIC   0x314B4345  // ECK1
#define BCRYPT_ECDH_PRIVATE_P256_MAGIC  0x324B4345  // ECK2
#define BCRYPT_ECDH_PUBLIC_P384_MAGIC   0x334B4345  // ECK3
#define BCRYPT_ECDH_PRIVATE_P384_MAGIC  0x344B4345  // ECK4
#define BCRYPT_ECDH_PUBLIC_P521_MAGIC   0x354B4345  // ECK5
#define BCRYPT_ECDH_PRIVATE_P521_MAGIC  0x364B4345  // ECK6

#define BCRYPT_ECDSA_PUBLIC_P256_MAGIC  0x31534345  // ECS1
#define BCRYPT_ECDSA_PRIVATE_P256_MAGIC 0x32534345  // ECS2
#define BCRYPT_ECDSA_PUBLIC_P384_MAGIC  0x33534345  // ECS3
#define BCRYPT_ECDSA_PRIVATE_P384_MAGIC 0x34534345  // ECS4
#define BCRYPT_ECDSA_PUBLIC_P521_MAGIC  0x35534345  // ECS5
#define BCRYPT_ECDSA_PRIVATE_P521_MAGIC 0x36534345  // ECS6

typedef struct _BCRYPT_ECCKEY_BLOB
{
    ULONG   dwMagic;
    ULONG   cbKey;
} BCRYPT_ECCKEY_BLOB, *PBCRYPT_ECCKEY_BLOB;

// The BCRYPT_DH_PUBLIC_BLOB and BCRYPT_DH_PRIVATE_BLOB blob types are used
// to transport plaintext DH keys. These blob types will be supported by
// all DH primitive providers.
#define BCRYPT_DH_PUBLIC_BLOB           L"DHPUBLICBLOB"
#define BCRYPT_DH_PRIVATE_BLOB          L"DHPRIVATEBLOB"
#define LEGACY_DH_PUBLIC_BLOB           L"CAPIDHPUBLICBLOB"
#define LEGACY_DH_PRIVATE_BLOB          L"CAPIDHPRIVATEBLOB"

#define BCRYPT_DH_PUBLIC_MAGIC          0x42504844  // DHPB
#define BCRYPT_DH_PRIVATE_MAGIC         0x56504844  // DHPV

typedef struct _BCRYPT_DH_KEY_BLOB
{
    ULONG   dwMagic;
    ULONG   cbKey;
} BCRYPT_DH_KEY_BLOB, *PBCRYPT_DH_KEY_BLOB;

// Property Strings for DH
#define BCRYPT_DH_PARAMETERS            L"DHParameters"

#define BCRYPT_DH_PARAMETERS_MAGIC      0x4d504844  // DHPM

typedef /*__struct_bcount(cbLength)*/ struct _BCRYPT_DH_PARAMETER_HEADER
{
    ULONG           cbLength;
    ULONG           dwMagic;
    ULONG           cbKeyLength;
} BCRYPT_DH_PARAMETER_HEADER;


// The BCRYPT_DSA_PUBLIC_BLOB and BCRYPT_DSA_PRIVATE_BLOB blob types are used
// to transport plaintext DSA keys. These blob types will be supported by
// all DSA primitive providers.
#define BCRYPT_DSA_PUBLIC_BLOB          L"DSAPUBLICBLOB"
#define BCRYPT_DSA_PRIVATE_BLOB         L"DSAPRIVATEBLOB"
#define LEGACY_DSA_PUBLIC_BLOB          L"CAPIDSAPUBLICBLOB"
#define LEGACY_DSA_PRIVATE_BLOB         L"CAPIDSAPRIVATEBLOB"
#define LEGACY_DSA_V2_PUBLIC_BLOB       L"V2CAPIDSAPUBLICBLOB"
#define LEGACY_DSA_V2_PRIVATE_BLOB      L"V2CAPIDSAPRIVATEBLOB"

#define BCRYPT_DSA_PUBLIC_MAGIC         0x42505344  // DSPB
#define BCRYPT_DSA_PRIVATE_MAGIC        0x56505344  // DSPV

typedef struct _BCRYPT_DSA_KEY_BLOB
{
    ULONG   dwMagic;
    ULONG   cbKey;
    UCHAR   Count[4];
    UCHAR   Seed[20];
    UCHAR   q[20];
} BCRYPT_DSA_KEY_BLOB, *PBCRYPT_DSA_KEY_BLOB;

typedef struct _BCRYPT_KEY_DATA_BLOB_HEADER
{
    ULONG   dwMagic;
    ULONG   dwVersion;
    ULONG   cbKeyData;
} BCRYPT_KEY_DATA_BLOB_HEADER, *PBCRYPT_KEY_DATA_BLOB_HEADER;

#define BCRYPT_KEY_DATA_BLOB_MAGIC       0x4d42444b //Key Data Blob Magic (KDBM)

#define BCRYPT_KEY_DATA_BLOB_VERSION1    0x1

// Property Strings for DSA
#define BCRYPT_DSA_PARAMETERS       L"DSAParameters"

#define BCRYPT_DSA_PARAMETERS_MAGIC 0x4d505344  // DSPM

typedef struct _BCRYPT_DSA_PARAMETER_HEADER
{
    ULONG           cbLength;
    ULONG           dwMagic;
    ULONG           cbKeyLength;
    UCHAR           Count[4];
    UCHAR           Seed[20];
    UCHAR           q[20];
} BCRYPT_DSA_PARAMETER_HEADER;

//
// Microsoft built-in providers.
//

#define MS_PRIMITIVE_PROVIDER                   L"Microsoft Primitive Provider"

//
// Common algorithm identifiers.
//

#define BCRYPT_RSA_ALGORITHM                    L"RSA"
#define BCRYPT_RSA_SIGN_ALGORITHM               L"RSA_SIGN"
#define BCRYPT_DH_ALGORITHM                     L"DH"
#define BCRYPT_DSA_ALGORITHM                    L"DSA"
#define BCRYPT_RC2_ALGORITHM                    L"RC2"
#define BCRYPT_RC4_ALGORITHM                    L"RC4"
#define BCRYPT_AES_ALGORITHM                    L"AES"
#define BCRYPT_DES_ALGORITHM                    L"DES"
#define BCRYPT_DESX_ALGORITHM                   L"DESX"
#define BCRYPT_3DES_ALGORITHM                   L"3DES"
#define BCRYPT_3DES_112_ALGORITHM               L"3DES_112"
#define BCRYPT_MD2_ALGORITHM                    L"MD2"
#define BCRYPT_MD4_ALGORITHM                    L"MD4"
#define BCRYPT_MD5_ALGORITHM                    L"MD5"
#define BCRYPT_SHA1_ALGORITHM                   L"SHA1"
#define BCRYPT_SHA256_ALGORITHM                 L"SHA256"
#define BCRYPT_SHA384_ALGORITHM                 L"SHA384"
#define BCRYPT_SHA512_ALGORITHM                 L"SHA512"
#define BCRYPT_AES_GMAC_ALGORITHM               L"AES-GMAC"
#define BCRYPT_ECDSA_P256_ALGORITHM             L"ECDSA_P256"
#define BCRYPT_ECDSA_P384_ALGORITHM             L"ECDSA_P384"
#define BCRYPT_ECDSA_P521_ALGORITHM             L"ECDSA_P521"
#define BCRYPT_ECDH_P256_ALGORITHM              L"ECDH_P256"
#define BCRYPT_ECDH_P384_ALGORITHM              L"ECDH_P384"
#define BCRYPT_ECDH_P521_ALGORITHM              L"ECDH_P521"
#define BCRYPT_RNG_ALGORITHM                    L"RNG"
#define BCRYPT_RNG_FIPS186_DSA_ALGORITHM        L"FIPS186DSARNG"
#define BCRYPT_RNG_DUAL_EC_ALGORITHM            L"DUALECRNG"

//
// Interfaces
//

#define BCRYPT_CIPHER_INTERFACE                 0x00000001
#define BCRYPT_HASH_INTERFACE                   0x00000002
#define BCRYPT_ASYMMETRIC_ENCRYPTION_INTERFACE  0x00000003
#define BCRYPT_SECRET_AGREEMENT_INTERFACE       0x00000004
#define BCRYPT_SIGNATURE_INTERFACE              0x00000005
#define BCRYPT_RNG_INTERFACE                    0x00000006


//
// Primitive algorithm provider functions.
//

#define BCRYPT_ALG_HANDLE_HMAC_FLAG     0x00000008

__checkReturn
NTSTATUS
WINAPI
BCryptOpenAlgorithmProvider(
    __out       BCRYPT_ALG_HANDLE   *phAlgorithm,
    __in        LPCWSTR pszAlgId,
    __in_opt    LPCWSTR pszImplementation,
    __in        ULONG   dwFlags);



// AlgOperations flags for use with BCryptEnumAlgorithms()
#define BCRYPT_CIPHER_OPERATION                 0x00000001
#define BCRYPT_HASH_OPERATION                   0x00000002
#define BCRYPT_ASYMMETRIC_ENCRYPTION_OPERATION  0x00000004
#define BCRYPT_SECRET_AGREEMENT_OPERATION       0x00000008
#define BCRYPT_SIGNATURE_OPERATION              0x00000010
#define BCRYPT_RNG_OPERATION                    0x00000020

// USE EXTREME CAUTION: editing comments that contain "certenrolls_*" tokens
// could break building CertEnroll idl files:
// certenrolls_begin -- BCRYPT_ALGORITHM_IDENTIFIER
typedef struct _BCRYPT_ALGORITHM_IDENTIFIER
{
    LPWSTR  pszName;
    ULONG   dwClass;
    ULONG   dwFlags;

} BCRYPT_ALGORITHM_IDENTIFIER;
// certenrolls_end

__checkReturn
NTSTATUS
WINAPI
BCryptEnumAlgorithms(
    __in    ULONG   dwAlgOperations,
    __out   ULONG   *pAlgCount,
    __out   BCRYPT_ALGORITHM_IDENTIFIER **ppAlgList,
    __in    ULONG   dwFlags);

typedef struct _BCRYPT_PROVIDER_NAME
{
    LPWSTR  pszProviderName;
} BCRYPT_PROVIDER_NAME;

__checkReturn
NTSTATUS
WINAPI
BCryptEnumProviders(
    __in    LPCWSTR pszAlgId,
    __out   ULONG   *pImplCount,
    __out   BCRYPT_PROVIDER_NAME    **ppImplList,
    __in    ULONG   dwFlags);


// Flags for use with BCryptGetProperty and BCryptSetProperty
#define BCRYPT_PUBLIC_KEY_FLAG                  0x00000001
#define BCRYPT_PRIVATE_KEY_FLAG                 0x00000002

__checkReturn
NTSTATUS
WINAPI
BCryptGetProperty(
    __in                                        BCRYPT_HANDLE   hObject,
    __in                                        LPCWSTR pszProperty,
    __out_bcount_part_opt(cbOutput, *pcbResult) PUCHAR   pbOutput,
    __in                                        ULONG   cbOutput,
    __out                                       ULONG   *pcbResult,
    __in                                        ULONG   dwFlags);


__checkReturn
NTSTATUS
WINAPI
BCryptSetProperty(
    __inout                 BCRYPT_HANDLE   hObject,
    __in                    LPCWSTR pszProperty,
    __in_bcount(cbInput)    PUCHAR   pbInput,
    __in                    ULONG   cbInput,
    __in                    ULONG   dwFlags);


NTSTATUS
WINAPI
BCryptCloseAlgorithmProvider(
    __inout BCRYPT_ALG_HANDLE   hAlgorithm,
    __in    ULONG   dwFlags);


VOID
WINAPI
BCryptFreeBuffer(
    __in PVOID   pvBuffer);


//
// Primitive encryption functions.
//
__checkReturn
NTSTATUS
WINAPI
BCryptGenerateSymmetricKey(
    __inout                             BCRYPT_ALG_HANDLE   hAlgorithm,
    __out                               BCRYPT_KEY_HANDLE   *phKey,
    __out_bcount_full_opt(cbKeyObject)  PUCHAR   pbKeyObject,
    __in                                ULONG   cbKeyObject,
    __in_bcount(cbSecret)               PUCHAR   pbSecret,
    __in                                ULONG   cbSecret,
    __in                                ULONG   dwFlags);


__checkReturn
NTSTATUS
WINAPI
BCryptGenerateKeyPair(
    __inout BCRYPT_ALG_HANDLE   hAlgorithm,
    __out   BCRYPT_KEY_HANDLE   *phKey,
    __in    ULONG   dwLength,
    __in    ULONG   dwFlags);


__checkReturn
NTSTATUS
WINAPI
BCryptEncrypt(
    __inout                                     BCRYPT_KEY_HANDLE hKey,
    __in_bcount_opt(cbInput)                    PUCHAR   pbInput,
    __in                                        ULONG   cbInput,
    __in_opt                                    VOID    *pPaddingInfo,
    __inout_bcount_opt(cbIV)                    PUCHAR   pbIV,
    __in                                        ULONG   cbIV,
    __out_bcount_part_opt(cbOutput, *pcbResult) PUCHAR   pbOutput,
    __in                                        ULONG   cbOutput,
    __out                                       ULONG   *pcbResult,
    __in                                        ULONG   dwFlags);


__checkReturn
NTSTATUS
WINAPI
BCryptDecrypt(
    __inout                                     BCRYPT_KEY_HANDLE   hKey,
    __in_bcount_opt(cbInput)                    PUCHAR   pbInput,
    __in                                        ULONG   cbInput,
    __in_opt                                    VOID    *pPaddingInfo,
    __inout_bcount_opt(cbIV)                    PUCHAR   pbIV,
    __in                                        ULONG   cbIV,
    __out_bcount_part_opt(cbOutput, *pcbResult) PUCHAR   pbOutput,
    __in                                        ULONG   cbOutput,
    __out                                       ULONG   *pcbResult,
    __in                                        ULONG   dwFlags);


__checkReturn
NTSTATUS
WINAPI
BCryptExportKey(
    __in                                        BCRYPT_KEY_HANDLE   hKey,
    __in_opt                                    BCRYPT_KEY_HANDLE   hExportKey,
    __in                                        LPCWSTR pszBlobType,
    __out_bcount_part_opt(cbOutput, *pcbResult) PUCHAR   pbOutput,
    __in                                        ULONG   cbOutput,
    __out                                       ULONG   *pcbResult,
    __in                                        ULONG   dwFlags);


__checkReturn
NTSTATUS
WINAPI
BCryptImportKey(
    __in                                BCRYPT_ALG_HANDLE hAlgorithm,
    __in_opt                            BCRYPT_KEY_HANDLE hImportKey,
    __in                                LPCWSTR pszBlobType,
    __out                               BCRYPT_KEY_HANDLE *phKey,
    __out_bcount_full_opt(cbKeyObject)  PUCHAR   pbKeyObject,
    __in                                ULONG   cbKeyObject,
    __in_bcount(cbInput)                PUCHAR   pbInput,
    __in                                ULONG   cbInput,
    __in                                ULONG   dwFlags);


#define BCRYPT_NO_KEY_VALIDATION    0x00000008
__checkReturn
NTSTATUS
WINAPI
BCryptImportKeyPair(
    __in                            BCRYPT_ALG_HANDLE hAlgorithm,
    __in_opt                        BCRYPT_KEY_HANDLE hImportKey,
    __in                            LPCWSTR pszBlobType,
    __out                           BCRYPT_KEY_HANDLE *phKey,
    __in_bcount(cbInput)            PUCHAR   pbInput,
    __in                            ULONG   cbInput,
    __in                            ULONG   dwFlags);


__checkReturn
NTSTATUS
WINAPI
BCryptDuplicateKey(
    __in                                BCRYPT_KEY_HANDLE   hKey,
    __out                               BCRYPT_KEY_HANDLE   *phNewKey,
    __out_bcount_full_opt(cbKeyObject)  PUCHAR   pbKeyObject,
    __in                                ULONG   cbKeyObject,
    __in                                ULONG   dwFlags);


__checkReturn
NTSTATUS
WINAPI
BCryptFinalizeKeyPair(
    __inout BCRYPT_KEY_HANDLE   hKey,
    __in    ULONG   dwFlags);


NTSTATUS
WINAPI
BCryptDestroyKey(
    __inout BCRYPT_KEY_HANDLE   hKey);


NTSTATUS
WINAPI
BCryptDestroySecret(
    __inout BCRYPT_SECRET_HANDLE   hSecret);


__checkReturn
NTSTATUS
WINAPI
BCryptSignHash(
    __in                                        BCRYPT_KEY_HANDLE   hKey,
    __in_opt                                    VOID    *pPaddingInfo,
    __in_bcount(cbInput)                        PUCHAR   pbInput,
    __in                                        ULONG   cbInput,
    __out_bcount_part_opt(cbOutput, *pcbResult) PUCHAR   pbOutput,
    __in                                        ULONG   cbOutput,
    __out                                       ULONG   *pcbResult,
    __in                                        ULONG   dwFlags);


__checkReturn
NTSTATUS
WINAPI
BCryptVerifySignature(
    __in                        BCRYPT_KEY_HANDLE   hKey,
    __in_opt                    VOID    *pPaddingInfo,
    __in_bcount(cbHash)         PUCHAR   pbHash,
    __in                        ULONG   cbHash,
    __in_bcount(cbSignature)    PUCHAR   pbSignature,
    __in                        ULONG   cbSignature,
    __in                        ULONG   dwFlags);


__checkReturn
NTSTATUS
WINAPI
BCryptSecretAgreement(
    __in    BCRYPT_KEY_HANDLE       hPrivKey,
    __in    BCRYPT_KEY_HANDLE       hPubKey,
    __out   BCRYPT_SECRET_HANDLE    *phAgreedSecret,
    __in    ULONG                   dwFlags);


__checkReturn
NTSTATUS
WINAPI
BCryptDeriveKey(
    __in        BCRYPT_SECRET_HANDLE hSharedSecret,
    __in        LPCWSTR              pwszKDF,
    __in_opt    BCryptBufferDesc     *pParameterList,
    __out_bcount_part_opt(cbDerivedKey, *pcbResult) PUCHAR pbDerivedKey,
    __in        ULONG                cbDerivedKey,
    __out       ULONG                *pcbResult,
    __in        ULONG                dwFlags);


//
// Primitive hashing functions.
//
__checkReturn
NTSTATUS
WINAPI
BCryptCreateHash(
    __inout                             BCRYPT_ALG_HANDLE   hAlgorithm,
    __out                               BCRYPT_HASH_HANDLE  *phHash,
    __out_bcount_full_opt(cbHashObject) PUCHAR   pbHashObject,
    __in                                ULONG   cbHashObject,
    __in_bcount_opt(cbSecret)           PUCHAR   pbSecret,   // optional
    __in                                ULONG   cbSecret,   // optional
    __in                                ULONG   dwFlags);


__checkReturn
NTSTATUS
WINAPI
BCryptHashData(
    __inout                 BCRYPT_HASH_HANDLE  hHash,
    __in_bcount(cbInput)    PUCHAR   pbInput,
    __in                    ULONG   cbInput,
    __in                    ULONG   dwFlags);


__checkReturn
NTSTATUS
WINAPI
BCryptFinishHash(
    __inout                     BCRYPT_HASH_HANDLE hHash,
    __out_bcount_full(cbOutput) PUCHAR   pbOutput,
    __in                        ULONG   cbOutput,
    __in                        ULONG   dwFlags);


__checkReturn
NTSTATUS
WINAPI
BCryptDuplicateHash(
    __in                                BCRYPT_HASH_HANDLE  hHash,
    __out                               BCRYPT_HASH_HANDLE  *phNewHash,
    __out_bcount_full_opt(cbHashObject) PUCHAR   pbHashObject,
    __in                                ULONG   cbHashObject,
    __in                                ULONG   dwFlags);


NTSTATUS
WINAPI
BCryptDestroyHash(
    __inout BCRYPT_HASH_HANDLE  hHash);


//
// Primitive random number generation.
//

// Flags to BCryptGenRandom
#define BCRYPT_RNG_USE_ENTROPY_IN_BUFFER    0x00000001
#define BCRYPT_USE_SYSTEM_PREFERRED_RNG     0x00000002

__checkReturn
NTSTATUS
WINAPI
BCryptGenRandom(
    __in_opt                        BCRYPT_ALG_HANDLE   hAlgorithm,
    __inout_bcount_full(cbBuffer)   PUCHAR  pbBuffer,
    __in                            ULONG   cbBuffer,
    __in                            ULONG   dwFlags);


//
// Primitive key derivation functions.
//
__checkReturn
NTSTATUS
WINAPI
BCryptDeriveKeyCapi(
    __in                            BCRYPT_HASH_HANDLE  hHash,
    __in_opt                        BCRYPT_ALG_HANDLE   hTargetAlg,
    __out_bcount( cbDerivedKey )    PUCHAR              pbDerivedKey,
    __in                            ULONG               cbDerivedKey,
    __in                            ULONG               dwFlags);


__checkReturn
NTSTATUS
WINAPI
BCryptDeriveKeyPBKDF2(
    __in                            BCRYPT_ALG_HANDLE   hPrf,
    __in_bcount_opt( cbPassword )   PUCHAR              pbPassword,
    __in                            ULONG               cbPassword,
    __in_bcount_opt( cbSalt )       PUCHAR              pbSalt,
    __in                            ULONG               cbSalt,
    __in                            ULONGLONG           cIterations,
    __out_bcount( cbDerivedKey )    PUCHAR              pbDerivedKey,
    __in                            ULONG               cbDerivedKey,
    __in                            ULONG               dwFlags);


//
// Interface version control...
//
typedef struct _BCRYPT_INTERFACE_VERSION
{
    USHORT MajorVersion;
    USHORT MinorVersion;

} BCRYPT_INTERFACE_VERSION, *PBCRYPT_INTERFACE_VERSION;

#define BCRYPT_MAKE_INTERFACE_VERSION(major,minor) {(USHORT)major, (USHORT)minor}

#define BCRYPT_IS_INTERFACE_VERSION_COMPATIBLE(loader, provider)    \
    ((loader).MajorVersion <= (provider).MajorVersion)

//
// Primitive provider interfaces.
//

#define BCRYPT_CIPHER_INTERFACE_VERSION_1    BCRYPT_MAKE_INTERFACE_VERSION(1,0)


#define BCRYPT_HASH_INTERFACE_VERSION_1    BCRYPT_MAKE_INTERFACE_VERSION(1,0)


#define BCRYPT_ASYMMETRIC_ENCRYPTION_INTERFACE_VERSION_1    BCRYPT_MAKE_INTERFACE_VERSION(1,0)


#define BCRYPT_SECRET_AGREEMENT_INTERFACE_VERSION_1    BCRYPT_MAKE_INTERFACE_VERSION(1,0)


#define BCRYPT_SIGNATURE_INTERFACE_VERSION_1    BCRYPT_MAKE_INTERFACE_VERSION(1,0)


#define BCRYPT_RNG_INTERFACE_VERSION_1    BCRYPT_MAKE_INTERFACE_VERSION(1,0)


//////////////////////////////////////////////////////////////////////////////
// CryptoConfig Definitions //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// Interface registration flags
#define CRYPT_MIN_DEPENDENCIES      (0x00000001)
#define CRYPT_PROCESS_ISOLATE       (0x00010000) // User-mode only

// Processor modes supported by a provider
//
// (Valid for BCryptQueryProviderRegistration and BCryptResolveProviders):
//
#define CRYPT_UM                    (0x00000001)    // User mode only
#define CRYPT_KM                    (0x00000002)    // Kernel mode only
#define CRYPT_MM                    (0x00000003)    // Multi-mode: Must support BOTH UM and KM
//
// (Valid only for BCryptQueryProviderRegistration):
//
#define CRYPT_ANY                   (0x00000004)    // Wildcard: Either UM, or KM, or both


// Write behavior flags
#define CRYPT_OVERWRITE             (0x00000001)

// Configuration tables
#define CRYPT_LOCAL                 (0x00000001)
#define CRYPT_DOMAIN                (0x00000002)

// Context configuration flags
#define CRYPT_EXCLUSIVE             (0x00000001)
#define CRYPT_OVERRIDE              (0x00010000) // Enterprise table only

// Resolution and enumeration flags
#define CRYPT_ALL_FUNCTIONS         (0x00000001)
#define CRYPT_ALL_PROVIDERS         (0x00000002)

// Priority list positions
#define CRYPT_PRIORITY_TOP          (0x00000000)
#define CRYPT_PRIORITY_BOTTOM       (0xFFFFFFFF)

// Default system-wide context
#define CRYPT_DEFAULT_CONTEXT       L"Default"

//////////////////////////////////////////////////////////////////////////////
// CryptoConfig Structures ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//
// Provider Registration Structures
//

typedef struct _CRYPT_INTERFACE_REG
{
    ULONG dwInterface;
    ULONG dwFlags;

    ULONG cFunctions;
    PWSTR *rgpszFunctions;
}
CRYPT_INTERFACE_REG, *PCRYPT_INTERFACE_REG;

typedef struct _CRYPT_IMAGE_REG
{
    PWSTR pszImage;

    ULONG cInterfaces;
    PCRYPT_INTERFACE_REG *rgpInterfaces;
}
CRYPT_IMAGE_REG, *PCRYPT_IMAGE_REG;

typedef struct _CRYPT_PROVIDER_REG
{
    ULONG cAliases;
    PWSTR *rgpszAliases;

    PCRYPT_IMAGE_REG pUM;
    PCRYPT_IMAGE_REG pKM;
}
CRYPT_PROVIDER_REG, *PCRYPT_PROVIDER_REG;

typedef struct _CRYPT_PROVIDERS
{
    ULONG cProviders;
    PWSTR *rgpszProviders;
}
CRYPT_PROVIDERS, *PCRYPT_PROVIDERS;

//
// Context Configuration Structures
//

typedef struct _CRYPT_CONTEXT_CONFIG
{
    ULONG dwFlags;
    ULONG dwReserved;
}
CRYPT_CONTEXT_CONFIG, *PCRYPT_CONTEXT_CONFIG;

typedef struct _CRYPT_CONTEXT_FUNCTION_CONFIG
{
    ULONG dwFlags;
    ULONG dwReserved;
}
CRYPT_CONTEXT_FUNCTION_CONFIG, *PCRYPT_CONTEXT_FUNCTION_CONFIG;

typedef struct _CRYPT_CONTEXTS
{
    ULONG cContexts;
    PWSTR *rgpszContexts;
}
CRYPT_CONTEXTS, *PCRYPT_CONTEXTS;

typedef struct _CRYPT_CONTEXT_FUNCTIONS
{
    ULONG cFunctions;
    PWSTR *rgpszFunctions;
}
CRYPT_CONTEXT_FUNCTIONS, *PCRYPT_CONTEXT_FUNCTIONS;

typedef struct _CRYPT_CONTEXT_FUNCTION_PROVIDERS
{
    ULONG cProviders;
    PWSTR *rgpszProviders;
}
CRYPT_CONTEXT_FUNCTION_PROVIDERS, *PCRYPT_CONTEXT_FUNCTION_PROVIDERS;

//
// Provider Resolution Structures
//

typedef struct _CRYPT_PROPERTY_REF
{
    PWSTR pszProperty;

    ULONG cbValue;
    PUCHAR pbValue;
}
CRYPT_PROPERTY_REF, *PCRYPT_PROPERTY_REF;

typedef struct _CRYPT_IMAGE_REF
{
    PWSTR pszImage;
    ULONG dwFlags;
}
CRYPT_IMAGE_REF, *PCRYPT_IMAGE_REF;

typedef struct _CRYPT_PROVIDER_REF
{
    ULONG dwInterface;
    PWSTR pszFunction;
    PWSTR pszProvider;

    ULONG cProperties;
    PCRYPT_PROPERTY_REF *rgpProperties;

    PCRYPT_IMAGE_REF pUM;
    PCRYPT_IMAGE_REF pKM;
}
CRYPT_PROVIDER_REF, *PCRYPT_PROVIDER_REF;

typedef struct _CRYPT_PROVIDER_REFS
{
    ULONG cProviders;
    PCRYPT_PROVIDER_REF *rgpProviders;
}
CRYPT_PROVIDER_REFS, *PCRYPT_PROVIDER_REFS;

//////////////////////////////////////////////////////////////////////////////
// CryptoConfig Functions ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef KERNEL_MODE_CNG


__checkReturn
NTSTATUS
WINAPI
BCryptQueryProviderRegistration(
    __in LPCWSTR pszProvider,
    __in ULONG dwMode,
    __in ULONG dwInterface,
    __inout ULONG* pcbBuffer,
    __deref_opt_inout_bcount_part_opt(*pcbBuffer, *pcbBuffer) PCRYPT_PROVIDER_REG *ppBuffer);

__checkReturn
NTSTATUS
WINAPI
BCryptEnumRegisteredProviders(
    __inout ULONG* pcbBuffer,
    __deref_opt_inout_bcount_part_opt(*pcbBuffer, *pcbBuffer) PCRYPT_PROVIDERS *ppBuffer);

//
// Context Configuration Functions
//
__checkReturn
NTSTATUS
WINAPI
BCryptCreateContext(
    __in ULONG dwTable,
    __in LPCWSTR pszContext,
    __in_opt PCRYPT_CONTEXT_CONFIG pConfig); // Optional

__checkReturn
NTSTATUS
WINAPI
BCryptDeleteContext(
    __in ULONG dwTable,
    __in LPCWSTR pszContext);

__checkReturn
NTSTATUS
WINAPI
BCryptEnumContexts(
    __in ULONG dwTable,
    __inout ULONG* pcbBuffer,
    __deref_opt_inout_bcount_part_opt(*pcbBuffer, *pcbBuffer) PCRYPT_CONTEXTS *ppBuffer);

__checkReturn
NTSTATUS
WINAPI
BCryptConfigureContext(
    __in ULONG dwTable,
    __in LPCWSTR pszContext,
    __in PCRYPT_CONTEXT_CONFIG pConfig);

__checkReturn
NTSTATUS
WINAPI
BCryptQueryContextConfiguration(
    __in ULONG dwTable,
    __in LPCWSTR pszContext,
    __inout ULONG* pcbBuffer,
    __deref_opt_inout_bcount_part_opt(*pcbBuffer, *pcbBuffer) PCRYPT_CONTEXT_CONFIG *ppBuffer);

__checkReturn
NTSTATUS
WINAPI
BCryptAddContextFunction(
    __in ULONG dwTable,
    __in LPCWSTR pszContext,
    __in ULONG dwInterface,
    __in LPCWSTR pszFunction,
    __in ULONG dwPosition);

__checkReturn
NTSTATUS
WINAPI
BCryptRemoveContextFunction(
    __in ULONG dwTable,
    __in LPCWSTR pszContext,
    __in ULONG dwInterface,
    __in LPCWSTR pszFunction);

__checkReturn
NTSTATUS
WINAPI
BCryptEnumContextFunctions(
    __in ULONG dwTable,
    __in LPCWSTR pszContext,
    __in ULONG dwInterface,
    __inout ULONG* pcbBuffer,
    __deref_opt_inout_bcount_part_opt(*pcbBuffer, *pcbBuffer) PCRYPT_CONTEXT_FUNCTIONS *ppBuffer);

__checkReturn
NTSTATUS
WINAPI
BCryptConfigureContextFunction(
    __in ULONG dwTable,
    __in LPCWSTR pszContext,
    __in ULONG dwInterface,
    __in LPCWSTR pszFunction,
    __in PCRYPT_CONTEXT_FUNCTION_CONFIG pConfig);

__checkReturn
NTSTATUS
WINAPI
BCryptQueryContextFunctionConfiguration(
    __in ULONG dwTable,
    __in LPCWSTR pszContext,
    __in ULONG dwInterface,
    __in LPCWSTR pszFunction,
    __inout ULONG* pcbBuffer,
    __deref_opt_inout_bcount_part_opt(*pcbBuffer, *pcbBuffer) PCRYPT_CONTEXT_FUNCTION_CONFIG *ppBuffer);

__checkReturn
NTSTATUS
WINAPI
BCryptEnumContextFunctionProviders(
    __in ULONG dwTable,
    __in LPCWSTR pszContext,
    __in ULONG dwInterface,
    __in LPCWSTR pszFunction,
    __inout ULONG* pcbBuffer,
    __deref_opt_inout_bcount_part_opt(*pcbBuffer, *pcbBuffer) PCRYPT_CONTEXT_FUNCTION_PROVIDERS *ppBuffer);

__checkReturn
NTSTATUS
WINAPI
BCryptSetContextFunctionProperty(
    __in ULONG dwTable,
    __in LPCWSTR pszContext,
    __in ULONG dwInterface,
    __in LPCWSTR pszFunction,
    __in LPCWSTR pszProperty,
    __in ULONG cbValue,
    __in_bcount_opt(cbValue) PUCHAR pbValue);

__checkReturn
NTSTATUS
WINAPI
BCryptQueryContextFunctionProperty(
    __in ULONG dwTable,
    __in LPCWSTR pszContext,
    __in ULONG dwInterface,
    __in LPCWSTR pszFunction,
    __in LPCWSTR pszProperty,
    __inout ULONG* pcbValue,
    __deref_opt_inout_bcount_part_opt(*pcbValue, *pcbValue) PUCHAR *ppbValue);

#endif //#ifndef KERNEL_MODE_CNG

//
// Configuration Change Notification Functions
//

#ifdef KERNEL_MODE_CNG
__checkReturn
NTSTATUS
WINAPI
BCryptRegisterConfigChangeNotify(
    __in PRKEVENT pEvent);

#else
__checkReturn
NTSTATUS
WINAPI
BCryptRegisterConfigChangeNotify(
    __out HANDLE *phEvent);
#endif

#ifdef KERNEL_MODE_CNG
NTSTATUS
WINAPI
BCryptUnregisterConfigChangeNotify(
    __in PRKEVENT pEvent);

#else
NTSTATUS
WINAPI
BCryptUnregisterConfigChangeNotify(
    __in HANDLE hEvent);
#endif

//
// Provider Resolution Functions
//
__checkReturn
NTSTATUS WINAPI
BCryptResolveProviders(
    __in_opt LPCWSTR pszContext,
    __in_opt ULONG dwInterface,
    __in_opt LPCWSTR pszFunction,
    __in_opt LPCWSTR pszProvider,
    __in ULONG dwMode,
    __in ULONG dwFlags,
    __inout ULONG* pcbBuffer,
    __deref_opt_inout_bcount_part_opt(*pcbBuffer, *pcbBuffer) PCRYPT_PROVIDER_REFS *ppBuffer);

//
// Miscellaneous queries about the crypto environment
//
NTSTATUS
WINAPI
BCryptGetFipsAlgorithmMode(
    __out BOOLEAN *pfEnabled
    );

#ifdef __cplusplus
}
#endif

#endif // __BCRYPT_H__


