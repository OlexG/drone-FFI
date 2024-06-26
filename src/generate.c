#include "crypto/ed25519.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

unsigned char seed[32];
unsigned char signature[64];
unsigned char public_key[32];
unsigned char private_key[64];
unsigned char scalar[32];
unsigned char shared_secret[32];
unsigned char time_bytes[4];

void write_to_file(
  const char *filename, 
  const unsigned char *private_key, 
  const unsigned char *public_key, 
  const unsigned char *encoded_timestamp, 
  const unsigned char *timestamp,
  size_t len
) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    fprintf(file, "Private key: ");
    for (int i = 0; i < 64; i++) {
        fprintf(file, "%02x", private_key[i]);
    }
    fprintf(file, "\nPublic key: ");
    for (int i = 0; i < 32; i++) {
        fprintf(file, "%02x", public_key[i]);
    }
    fprintf(file, "\nEncoded timestamp: ");
    for (size_t i = 0; i < len; i++) {
        fprintf(file, "%02x", encoded_timestamp[i]);
    }
    fprintf(file, "\n");
    fprintf(file, "Timestamp: ");
    for (size_t i = 0; i < 4; i++) {
        fprintf(file, "%02x", timestamp[i]);
    }
    fclose(file);
}

int main() {
    // Generate a random seed
    if (ed25519_create_seed(seed)) {
      printf("error while generating seed\n");
    }
    // From this seed generate a random private key
    ed25519_create_keypair(public_key, private_key, seed);

    // Generate a timestamp
    unsigned char timestamp[4];
    int t_int = time(NULL);
    memcpy(timestamp, &t_int, 4);

    // Encode the timestamp using the private key (signing the timestamp)
    ed25519_sign(signature, timestamp, 4, public_key, private_key);

    // Write the private key, public key, and encoded timestamp to secrets.txt
    write_to_file("secrets.txt", private_key, public_key, signature, timestamp, sizeof(signature));

    // Print the private key
    printf("Private key: ");
    for (int i = 0; i < 64; i++) {
        printf("%02x", private_key[i]);
    }
    printf("\n");

    // Print the public key
    printf("Public key: ");
    for (int i = 0; i < 32; i++) {
        printf("%02x", public_key[i]);
    }
    printf("\n");

    // Print the encoded timestamp (signature)
    printf("Encoded timestamp: ");
    for (int i = 0; i < 64; i++) {
        printf("%02x", signature[i]);
    }
    printf("\n");

    int result = ed25519_verify(signature, timestamp, 4, public_key);
    if (result) {
        printf("Timestamp verified\n");
    } else {
        printf("Timestamp not verified\n");
    }
    return 0;
}
