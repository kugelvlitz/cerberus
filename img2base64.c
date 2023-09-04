#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Function to encode binary data to Base64
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char* base64_encode(const uint8_t* data, size_t input_length) {
    size_t output_length = 4 * ((input_length + 2) / 3);
    char* encoded_data = (char*)malloc(output_length + 1); // +1 for null-terminator
    if (encoded_data == NULL) {
        return NULL; // Memory allocation failed
    }

    size_t i, j = 0;
    for (i = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = base64_chars[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 0 * 6) & 0x3F];
    }

    // Add padding if necessary
    for (i = 0; i < (3 - (input_length % 3)) % 3; i++) {
        encoded_data[output_length - 1 - i] = '=';
    }

    encoded_data[output_length] = '\0'; // Null-terminate the string
    return encoded_data;
}

char* encode(char path[]) {
    // Open the image file
    FILE* image_file = fopen(path, "rb");
    if (!image_file) {
        perror("Failed to open image file");
        exit;
    }

    // Get the size of the image
    fseek(image_file, 0, SEEK_END);
    size_t image_size = ftell(image_file);
    rewind(image_file);

    // Read the image data into memory
    uint8_t* image_data = (uint8_t*)malloc(image_size);
    if (!image_data) {
        perror("Failed to allocate memory for image data");
        fclose(image_file);
        exit;
    }

    if (fread(image_data, 1, image_size, image_file) != image_size) {
        perror("Failed to read image data");
        fclose(image_file);
        free(image_data);
        exit;
    }

    // Encode the image data as Base64
    char* base64_data = base64_encode(image_data, image_size);
    if (!base64_data) {
        perror("Failed to encode image data as Base64");
        fclose(image_file);
        free(image_data);
        exit;
    }


    // Cleanup
    free(base64_data);
    free(image_data);
    fclose(image_file);

    return base64_data;
}

// Function to decode Base64 to binary data
uint8_t* base64_decode(const char* data, size_t input_length, size_t* output_length) {
    if (input_length % 4 != 0) {
        return NULL; // Invalid Base64 input
    }

    *output_length = (input_length / 4) * 3;
    if (data[input_length - 1] == '=') {
        (*output_length)--;
    }
    if (data[input_length - 2] == '=') {
        (*output_length)--;
    }

    uint8_t* decoded_data = (uint8_t*)malloc(*output_length);
    if (decoded_data == NULL) {
        return NULL; // Memory allocation failed
    }

    size_t i, j = 0;
    uint32_t sextet_a, sextet_b, sextet_c, sextet_d;
    for (i = 0; i < input_length; i += 4) {
        sextet_a = strchr(base64_chars, data[i]) - base64_chars;
        sextet_b = strchr(base64_chars, data[i + 1]) - base64_chars;
        sextet_c = strchr(base64_chars, data[i + 2]) - base64_chars;
        sextet_d = strchr(base64_chars, data[i + 3]) - base64_chars;

        uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + sextet_d;

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}

int decode(char* base64_data) {
    size_t decoded_size;
    uint8_t* decoded_data = base64_decode(base64_data, strlen(base64_data), &decoded_size);
    if (!decoded_data) {
        perror("Failed to decode Base64 data");
        free(base64_data);
        exit;
    }

    // Save the decoded binary data to a file
    FILE* decoded_image_file = fopen("decoded_image.jpg", "wb");
    if (!decoded_image_file) {
        perror("Failed to create the decoded image file");
        free(base64_data);
        free(decoded_data);
        exit;
    }

    if (fwrite(decoded_data, 1, decoded_size, decoded_image_file) != decoded_size) {
        perror("Failed to write decoded image data to file");
        fclose(decoded_image_file);
        free(base64_data);
        free(decoded_data);
        exit;
    }

    printf("Decoded image saved as 'decoded_image.jpg'\n");
  
}

void main() {
    char* result = encode("yo.jpg");
    decode(result);
}