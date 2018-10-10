#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define KEY_LEN      16
/** 8 bytes time and 2 bytes validate(key[9],key[4],key[12],key[2],key[5],key[15],key[10],key[13]) */
#define TIME_LEN     10

/** use chip id as key */
uint8_t default_random[] = {0xde, 0xad, 0xbe, 0xef};
uint8_t random_key = 0xae;
uint8_t key[KEY_LEN];

uint8_t serial_number[KEY_LEN] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

void license_generate_key(const uint8_t *chip_id, uint8_t *pkey);
void key_to_serial_number(const uint8_t *pkey, uint8_t *pserial);
void encrypt_time(uint64_t time, const uint8_t *pserial, uint8_t *pencrypt_time);
bool decrypt_time(const uint8_t *pencrypt_time, const uint8_t *pserial, uint64_t *ptime);

static void dump_value(const char *title, uint8_t *data, uint8_t len)
{
    printf("%s = %02x", title, data[0]);
    for (uint8_t i = 1; i < len; ++i)
    {
        printf("-%02x", data[i]);
    }
    printf("\r\n");
}

static void reorder_data(uint8_t *data)
{
    /** out-of-order key, odd-evev, even-odd */
    uint8_t temp = 0;
    for (uint8_t i = 0; i < KEY_LEN / 2; ++i)
    {
        temp = data[i];
        if (0 != ((i + 1) % 2))
        {
            data[i] = data[i + KEY_LEN / 2 + 1];
            data[i + KEY_LEN / 2 + 1] = temp;
        }
        else
        {
            data[i] = data[i + KEY_LEN / 2 - 1];
            data[i + KEY_LEN / 2 - 1] = temp;
        }
    }
}

void license_generate_key(const uint8_t *chip_id, uint8_t *pkey)
{
    /** fill key */
    for (uint8_t i = 0; i < 12; ++i)
    {
        pkey[i] = chip_id[i];
    }

    /** fill random */
    for (uint8_t i = 12; i < KEY_LEN; ++i)
    {
        pkey[i] = default_random[i - 12];
    }

    reorder_data(pkey);
}

void key_to_serial_number(const uint8_t *pkey, uint8_t *pserial)
{
    /** fill serial number with key */
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
        pserial[i] = pkey[i];
    }

    /** use serial_number[2] to xor serial_number*/
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
        if (2 != i)
        {
            pserial[i] ^= pserial[2];
        }
    }

    /** xor random ro hide serial_number[2] */
    pserial[2] ^= random_key;

    reorder_data(pserial);
}

static void serial_number_to_key(const uint8_t *pserial, uint8_t *pkey)
{
    /** fill key with serial_number */
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
        pkey[i] = pserial[i];
    }

    reorder_data(pkey);

    /** xor random to show key[2] */
    pkey[2] ^= random_key;

    /** use key[2] to xor key */
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
        if (2 != i)
        {
            pkey[i] ^= pkey[2];
        }
    }
}

void encrypt_time(uint64_t time, const uint8_t *pserial, uint8_t *pencrypt_time)
{
    *(uint64_t *)pencrypt_time = time;

    uint8_t temp_key[KEY_LEN];
    serial_number_to_key(pserial, temp_key);


    for (uint8_t i = 0; i < 8; ++i)
    {
        for (uint8_t j = 0; j < KEY_LEN; ++j)
        {
            if (j != i)
            {
                pencrypt_time[i] ^= temp_key[j];
            }
        }
    }

    pencrypt_time[8] = temp_key[9];
    pencrypt_time[9] = temp_key[4];
    pencrypt_time[10] = temp_key[12];
    pencrypt_time[11] = temp_key[2];
    pencrypt_time[12] = temp_key[5];
    pencrypt_time[13] = temp_key[15];
    pencrypt_time[14] = temp_key[10];
    pencrypt_time[15] = temp_key[13];

    reorder_data(pencrypt_time);
}

bool decrypt_time(const uint8_t *pencrypt_time, const uint8_t *pserial, uint64_t *ptime)
{
   
    uint8_t encrypt_time[KEY_LEN];
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
        encrypt_time[i] = pencrypt_time[i];
    }

    uint8_t temp_key[KEY_LEN];
    serial_number_to_key(pserial, temp_key);

    reorder_data(encrypt_time);
    for (uint8_t i = 0; i < 8; ++i)
    {
        for (uint8_t j = 0; j < KEY_LEN; ++j)
        {
            if (j != i)
            {
                encrypt_time[i] ^= temp_key[j];
            }
        }
    }

    if ((encrypt_time[8] == temp_key[9]) &&
        (encrypt_time[9] == temp_key[4]) &&
        (encrypt_time[10] == temp_key[12]) &&
        (encrypt_time[11] == temp_key[2]) &&
        (encrypt_time[12] == temp_key[5]) &&
        (encrypt_time[13] == temp_key[15]) &&
        (encrypt_time[14] == temp_key[10]) &&
        (encrypt_time[15] == temp_key[13]))
    {
        *ptime = *((uint64_t *)encrypt_time);
        return true;
    }
    
    return false;
}


int main(int argc, char **argv)
{

    uint8_t id[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    license_generate_key(id, key);
    dump_value("key", key, KEY_LEN);

    key_to_serial_number(key, serial_number);
    dump_value("serial", serial_number, KEY_LEN);

    uint8_t pencrypt_time[16];
    encrypt_time(0x123456789abcdef0, serial_number, pencrypt_time);
    dump_value("encrypt_time", pencrypt_time, KEY_LEN);

    uint64_t time = 0;
    if (decrypt_time(pencrypt_time, serial_number, &time))
    {
        printf("decrypt time success!\r\n");
        uint32_t temp_val = (time >> 64);
        printf("time = 0x%08x\n", temp_val); 
        printf("time = 0x%08x\n", time);
    }
    else
    {
        printf("decrypt time failed!\r\n");
    }

    return 0;
}
