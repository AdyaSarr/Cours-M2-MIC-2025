#if !defined(UTIL_H)
#define UTIL_H
#include <stdbool.h>

#define COMMAND_LIST_CREDENTIALS 0
#define COMMAND_MAKE_CREDENTIAL 1
#define COMMAND_GET_ASSERTION 2
#define COMMAND_RESET 3
#define STATUS_OK 0
#define STATUS_ERR_COMMAND_UNKNOWN 1
#define STATUS_ERR_CRYPTO_FAILED 2
#define STATUS_ERR_BAD_PARAMETER 3
#define STATUS_ERR_NOT_FOUND 4
#define STATUS_ERR_STORAGE_FULL 5
#define STATUS_ERR_APPROVAL 6



#define CREDENTIAL_ID_SIZE 16
#define PUBLIC_KEY_SIZE 40
#define APP_ID_SIZE 20
#define SIGNATURE_SIZE 40

//Les differents etats de l'authenticator
typedef enum {
    STATE_IDLE,//etat inital de repos
    STATE_RECEIVING,//etat de reception du premier octet MakeCredential
    STATE_WAIT_CONSENT,//etat d'attente du consentement de l'utilisateur
    STATE_PROCESS_CRYPTO,//le consentement a ete obtenu geneartion de la paire de cle
} authenticator_state_t;

typedef struct {
    uint8_t start; 
    // 20 octets pour SHA1(app_id), transférés en little-endian.
    uint8_t SHA1_app_id[APP_ID_SIZE]; 
} MakeCredential_t;


// Structure d'une entrée stockée dans l'EEPROM
typedef struct {
    bool is_used; // Flag d'utilisation (1 octet)
    uint8_t SHA1_app_id[APP_ID_SIZE]; // 20 octets
    uint8_t credential_id[CREDENTIAL_ID_SIZE]; // 16 octets
    uint8_t private_key[21]; // 21 octets (pour secp160r1)
    // Taille totale : 1 + 20 + 16 + 21 = 58 octets
} CredentialEntry_t;

typedef struct {
    uint8_t status;
    // 16 octets de credential_id
    uint8_t credential_id[CREDENTIAL_ID_SIZE]; 
    // 40 octets de clé publique
    uint8_t public_key[PUBLIC_KEY_SIZE];
} MakeCredentialResponse_t;

typedef struct {
    uint8_t start;
    // 20 octets de SHA1(app_id)
    uint8_t SHA1_app_id[APP_ID_SIZE];
    // 20 octets de clientDataHash
    uint8_t clientDataHash[APP_ID_SIZE];
} GetAssertion_t;



// Déclaration nécessaire pour le linkage avec micro-ecc
// Le compilateur doit savoir que cette fonction existe.
// Elle doit être définie selon la logique ADC que nous avons vue.
int micro_ecc_random_bytes(uint8_t *p_dest, unsigned p_size);

// Déclaration des fonctions EEPROM (définies dans l'explication précédente)
uint8_t write_credential(const uint8_t sha1_app_id[APP_ID_SIZE], const uint8_t credential_id[CREDENTIAL_ID_SIZE], const uint8_t private_key[21]);

void send_make_credential_response(const uint8_t credential_id[CREDENTIAL_ID_SIZE], const uint8_t public_key[PUBLIC_KEY_SIZE]);

// Déclaration de la fonction d'initialisation ADC
void ADC_init();
#endif // UTIL_H