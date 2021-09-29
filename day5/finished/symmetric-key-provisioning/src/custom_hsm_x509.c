#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "hsm_client_data.h"
#include "my_device_certs.h"
#include "iot_config.h"

// Provided for sample only
static const char *const SYMMETRIC_KEY = "Symmetric Key value";
static const char *const REGISTRATION_NAME = "Registration Name";

// Provided for sample only, canned values
static const unsigned char EK[] = {0x45, 0x6e, 0x64, 0x6f, 0x72, 0x73, 0x65, 0x6d, 0x65, 0x6e, 0x74, 0x20, 0x6b, 0x65, 0x79, 0x0d, 0x0a};
static const size_t EK_LEN = sizeof(EK) / sizeof(EK[0]);
static const unsigned char SRK[] = {0x53, 0x74, 0x6f, 0x72, 0x65, 0x20, 0x72, 0x6f, 0x6f, 0x74, 0x20, 0x6b, 0x65, 0x79, 0x0d, 0x0a};
static const size_t SRK_LEN = sizeof(SRK) / sizeof(SRK[0]);

typedef struct CUSTOM_HSM_SAMPLE_INFO_TAG
{
    const char *certificate;
    const char *common_name;
    const char *key;
    const unsigned char *endorsement_key;
    size_t ek_length;
    const unsigned char *storage_root_key;
    size_t srk_length;
    const char *symm_key;
    const char *registration_name;
} CUSTOM_HSM_SAMPLE_INFO;

int hsm_client_x509_init()
{
    return 0;
}

void hsm_client_x509_deinit()
{
}

HSM_CLIENT_HANDLE custom_hsm_create()
{
    HSM_CLIENT_HANDLE result;
    CUSTOM_HSM_SAMPLE_INFO *hsm_info = malloc(sizeof(CUSTOM_HSM_SAMPLE_INFO));
    if (hsm_info == NULL)
    {
        (void)printf("Failed allocating hsm info\r\n");
        result = NULL;
    }
    else
    {
        hsm_info->certificate = my_device_certs;
        hsm_info->key = my_device_private_key;
        hsm_info->common_name = AZURE_DPS_DEVICE_COMMON_NAME;
        hsm_info->endorsement_key = EK;
        hsm_info->ek_length = EK_LEN;
        hsm_info->storage_root_key = SRK;
        hsm_info->srk_length = SRK_LEN;
        hsm_info->symm_key = SYMMETRIC_KEY;
        hsm_info->registration_name = REGISTRATION_NAME;
        result = hsm_info;
    }

    return result;
}

void custom_hsm_destroy(HSM_CLIENT_HANDLE handle)
{
    if (handle != NULL)
    {
        CUSTOM_HSM_SAMPLE_INFO *hsm_info = (CUSTOM_HSM_SAMPLE_INFO *)handle;
        free(hsm_info);
    }
}

char *custom_hsm_get_certificate(HSM_CLIENT_HANDLE handle)
{
    char *result;
    if (handle == NULL)
    {
        (void)printf("Invalid handle value specified\r\n");
        result = NULL;
    }
    else
    {
        CUSTOM_HSM_SAMPLE_INFO *hsm_info = (CUSTOM_HSM_SAMPLE_INFO *)handle;
        size_t len = strlen(hsm_info->certificate);
        if ((result = (char *)malloc(len + 1)) == NULL)
        {
            (void)printf("Failure allocating certificate!\r\n");
            result = NULL;
        }
        else
        {
            strcpy(result, hsm_info->certificate);
        }
    }

    return result;
}

char *custom_hsm_get_key(HSM_CLIENT_HANDLE handle)
{
    char *result;
    if (handle == NULL)
    {
        (void)printf("Invalid handle value specified\r\n");
        result = NULL;
    }
    else
    {
        CUSTOM_HSM_SAMPLE_INFO *hsm_info = (CUSTOM_HSM_SAMPLE_INFO *)handle;
        size_t len = strlen(hsm_info->key);
        if ((result = (char *)malloc(len + 1)) == NULL)
        {
            (void)printf("Failure allocating key!\r\n");
            result = NULL;
        }
        else
        {
            strcpy(result, hsm_info->key);
        }
    }

    return result;
}

char *custom_hsm_get_common_name(HSM_CLIENT_HANDLE handle)
{
    char *result;
    if (handle == NULL)
    {
        (void)printf("Invalid handle value specified\r\n");
        result = NULL;
    }
    else
    {
        CUSTOM_HSM_SAMPLE_INFO *hsm_info = (CUSTOM_HSM_SAMPLE_INFO *)handle;
        size_t len = strlen(hsm_info->common_name);
        if ((result = (char *)malloc(len + 1)) == NULL)
        {
            (void)printf("Failure allocating common name!\r\n");
            result = NULL;
        }
        else
        {
            strcpy(result, hsm_info->common_name);
        }
    }

    return result;
}

static const HSM_CLIENT_X509_INTERFACE x509_interface = {
    custom_hsm_create,
    custom_hsm_destroy,
    custom_hsm_get_certificate,
    custom_hsm_get_key,
    custom_hsm_get_common_name,
};

const HSM_CLIENT_X509_INTERFACE *hsm_client_x509_interface()
{
    return &x509_interface;
}