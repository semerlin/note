#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define KEY_LEN      16

/** use chip id as key */
uint8_t default_random[] = {0xde, 0xad, 0xbe, 0xef};
uint8_t random_key = 0xae;
uint8_t key[KEY_LEN];

uint8_t serial_number[KEY_LEN] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

uint64_t time;

void license_generate_key(uint8_t *chip_id)
{
    /** fill key */
    for (uint8_t i = 0; i < 12; ++i)
    {
        key[i] = chip_id[i];
    }

    /** read random from flash */
    uint8_t random[4];
    flash_read_random(random);

    /** fill random if all zero */
    uint32_t sum = 0;
    for (uint8_t i = 0; i < 4; ++i)
    {
        sum += random[i];
    }

    if (0 == sum)
    {
        for (uint8_t i = 0; i < 4; ++i)
        {
            random[i] = default_random[i];
        }
    }

    /** fill key again */
    for (uint8_t i = 12; i < KEY_NUM; ++i)
    {
        key[i] = random[i - 12];
    }

    /** use key[2] to xor key */
    for (uint8_t i = 0; i < KEY_NUM; ++i)
    {
        if (2 != i)
        {
            key[i] ^= key[2];
        }
    }

    /** xor random ro hide key[2] */
    key[2] ^= random_key;

    /** out-of-order key */
    uint8_t temp = 0;
    for (uint8_t i = 0; i < KEY_NUM / 2; ++i)
    {
        temp = key[i];
        key[i] = key[i]
    }
}

void key_to_serial_number(const uint8_t *key, uint8_t *serial_number)
{
}

void serial_number_to_key(const uint8_t *serial_number, uint8_t *key)
{
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
    }
}

bool validate_license(uint8_t *license)
{
}


int main(int argc, char **argv)
{
    printf("hello world!\r\n");
    return 0;
}
