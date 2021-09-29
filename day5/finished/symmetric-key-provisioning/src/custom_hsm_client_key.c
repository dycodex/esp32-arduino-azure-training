#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hsm_client_data.h"
#include "iot_config.h"

typedef struct CUSTOM_HSM_SAMPLE_INFO_TAG
{
    const char *symm_key;
    const char *registration_id;
} CUSTOM_HSM_SAMPLE_INFO;

HSM_CLIENT_HANDLE custom_hsm_symm_key_client_create()
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
        // (void)mallocAndStrcpy_s(&hsm_info->symm_key, AZURE_DPS_SYMM_KEY);
        // (void)mallocAndStrcpy_s(&hsm_info->registration_id, AZURE_DPS_DEVICE_COMMON_NAME);
        // strcpy(hsm_info->symm_key, AZURE_DPS_SYMM_KEY);
        // strcpy(hsm_info->registration_id, AZURE_DPS_DEVICE_COMMON_NAME);
        // hsm_info->symm_key = (char *)malloc(sizeof(char) * 128);
        // hsm_info->registration_id = (char *)malloc(sizeof(char) * 128);
        hsm_info->symm_key = AZURE_DPS_SYMM_KEY;
        hsm_info->registration_id = AZURE_DPS_DEVICE_COMMON_NAME;
        result = hsm_info;
    }

    return result;
}

void custom_hsm_symm_key_client_destroy(HSM_CLIENT_HANDLE handle)
{
    if (handle != NULL)
    {
        CUSTOM_HSM_SAMPLE_INFO *hsm_info = (CUSTOM_HSM_SAMPLE_INFO *)handle;
        free(hsm_info);
    }
}

char *custom_hsm_get_symmkey(HSM_CLIENT_HANDLE handle)
{
    CUSTOM_HSM_SAMPLE_INFO *hsm_info = (CUSTOM_HSM_SAMPLE_INFO *)handle;
    return (char*)hsm_info->symm_key;
}

char *custom_hsm_get_registration_name(HSM_CLIENT_HANDLE handle)
{
    CUSTOM_HSM_SAMPLE_INFO *hsm_info = (CUSTOM_HSM_SAMPLE_INFO *)handle;
    return (char*)hsm_info->registration_id;
}

int custom_hsm_set_symmkey_info(HSM_CLIENT_HANDLE handle, const char *reg_name, const char *symm_key)
{
    CUSTOM_HSM_SAMPLE_INFO *hsm_info = (CUSTOM_HSM_SAMPLE_INFO *)handle;
    hsm_info->registration_id = reg_name;
    hsm_info->symm_key = symm_key;

    return 0;
}

static const HSM_CLIENT_KEY_INTERFACE client_key_interface = {
    custom_hsm_symm_key_client_create,
    custom_hsm_symm_key_client_destroy,
    custom_hsm_get_symmkey,
    custom_hsm_get_registration_name,
    custom_hsm_set_symmkey_info,
};

const HSM_CLIENT_KEY_INTERFACE *hsm_client_key_interface()
{
    return &client_key_interface;
}