#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char *base64_encode(const unsigned char *data, long input_length, long *output_length) {
    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *encoded_data;
    int i, j;

    *output_length = 4 * ((input_length + 2) / 3);

    encoded_data = (char *)malloc(*output_length + 1);
    if (!encoded_data) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    for (i = 0, j = 0; i < input_length;) {
        unsigned octet_a = i < input_length ? data[i++] : 0;
        unsigned octet_b = i < input_length ? data[i++] : 0;
        unsigned octet_c = i < input_length ? data[i++] : 0;

        unsigned triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        encoded_data[j++] = base64_chars[(triple >> 18) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 12) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 6) & 0x3F];
        encoded_data[j++] = base64_chars[triple & 0x3F];
    }

    
    for (i = 0; i < (3 - input_length % 3) % 3; i++) {
        encoded_data[*output_length - 1 - i] = '=';
    }

    encoded_data[*output_length] = '\0';
    return encoded_data;
}

unsigned char *base64_decode(const char *data, long input_length, long *output_length) {
    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i, j;
    unsigned char *decoded_data;
    unsigned int sextet_a, sextet_b, sextet_c, sextet_d;

    
    *output_length = input_length / 4 * 3;

    
    decoded_data = (unsigned char *)malloc(*output_length); // بدون إضافة 1 لأننا نتعامل مع بيانات ثنائية
    if (!decoded_data) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    for (i = 0, j = 0; i < input_length;) {
        
        sextet_a = data[i] == '=' ? 0 : strchr(base64_chars, data[i]) - base64_chars;
        sextet_b = data[i + 1] == '=' ? 0 : strchr(base64_chars, data[i + 1]) - base64_chars;
        sextet_c = data[i + 2] == '=' ? 0 : strchr(base64_chars, data[i + 2]) - base64_chars;
        sextet_d = data[i + 3] == '=' ? 0 : strchr(base64_chars, data[i + 3]) - base64_chars;

        
        unsigned triple = (sextet_a << 18) | (sextet_b << 12) | (sextet_c << 6) | sextet_d;

        if (j < *output_length) decoded_data[j++] = (triple >> 16) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = triple & 0xFF;

        i += 4;
    }

    return decoded_data;
}


void write_to_file(const char *filename, const char *data){
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%s", data);
    fclose(file);
}


char *read_from_file(const char *filename, long *file_length) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file for reading");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    *file_length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = (char *)malloc(*file_length + 1);
    if (!data) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    fread(data, 1, *file_length, file);
    data[*file_length] = '\0';
    fclose(file);
    
    return data;
}


unsigned char *read_audio_file(const char *filename, long *file_length) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening audio file for reading");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    *file_length = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *data = (unsigned char *)malloc(*file_length);
    if (!data) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    fread(data, 1, *file_length, file);
    fclose(file);
    
    return data;
}

void write_audio_file(const char *filename, const unsigned char *data, long length) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening audio file for writing");
        exit(EXIT_FAILURE);
    }

    fwrite(data, 1, length, file);
    fclose(file);
}

int main() {
    int choice;
    printf("اختر عملية:\n1. تشفير ملف صوتي\n2. فك تشفير ملف نصي\n");
    printf("الاختيار: ");
    scanf("%d", &choice);
    getchar();

    if (choice == 1) {
        char audio_filename[256];
        printf("أدخل اسم ملف الصوت (مثال: input.mp3): ");
        fgets(audio_filename, sizeof(audio_filename), stdin);
        audio_filename[strcspn(audio_filename, "\n")] = '\0';

        long input_length;
        unsigned char *audio_data = read_audio_file(audio_filename, &input_length);
        long output_length;

        char *encoded_data = base64_encode(audio_data, input_length, &output_length);
        write_to_file("encoded_data.txt", encoded_data);

        printf("تم تشفير البيانات وحفظها في 'encoded_data.txt'.\n");

        free(audio_data);
        free(encoded_data);
    } else if (choice == 2) {

        char encoded_filename[256];
        printf("أدخل اسم ملف النص المشفر (مثال: encoded_data.txt): ");
        fgets(encoded_filename, sizeof(encoded_filename), stdin);
        encoded_filename[strcspn(encoded_filename, "\n")] = '\0';

        long file_length;
        char *encoded_data = read_from_file(encoded_filename, &file_length);
        long output_length;

       
        unsigned char *decoded_data = base64_decode(encoded_data, file_length, &output_length);

       
        write_audio_file("decoded_output.mp3", decoded_data, output_length);

        printf("تم فك تشفير البيانات وحفظها في 'decoded_output.mp3'.\n");

        
        free(encoded_data);
        free(decoded_data);
    } else {
        printf("خيار غير صحيح. يرجى إدخال 1 أو 2.\n");
    }

    return 0;
}