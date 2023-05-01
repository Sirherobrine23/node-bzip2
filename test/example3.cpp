#include <stdio.h>
#include <stdlib.h>
#include <bzlib.hh>

#define BUFFER_SIZE 4096

int input_callback(void *buf, int size, void *user_data)
{
    return fread(buf, 1, size, (FILE*)user_data);
}

int output_callback(void *buf, int size, void *user_data)
{
    return fwrite(buf, 1, size, (FILE*)user_data);
}

int main(int argc, char *argv[])
{
    FILE *input_file, *output_file;
    int bzerror;
    bz_stream stream;

    char input_buffer[BUFFER_SIZE];
    char output_buffer[BUFFER_SIZE];
    int input_size, output_size;

    // Abrir o arquivo de entrada
    input_file = fopen(argv[1], "rb");
    if (!input_file) {
        fprintf(stderr, "Erro ao abrir o arquivo de entrada\n");
        exit(1);
    }

    // Abrir o arquivo de saída
    output_file = fopen(argv[2], "wb");
    if (!output_file) {
        fprintf(stderr, "Erro ao abrir o arquivo de saída\n");
        exit(1);
    }

    // Inicializar o stream de compressão
    stream.bzalloc = NULL;
    stream.bzfree = NULL;
    stream.opaque = NULL;
    stream.next_in = NULL;
    stream.avail_in = 0;
    stream.next_out = output_buffer;
    stream.avail_out = BUFFER_SIZE;

    BZ2_bzCompressInit(&stream, 9, 0, 30);

    // Comprimir o arquivo de entrada
    do {
        // Ler os dados de entrada utilizando o callback
        input_size = input_callback(input_buffer, BUFFER_SIZE, input_file);
        if (input_size == 0)
            break;

        stream.next_in = input_buffer;
        stream.avail_in = input_size;

        // Comprimir os dados utilizando o callback
        do {
            output_size = output_callback(output_buffer, BUFFER_SIZE - stream.avail_out, output_file);
            if (output_size == 0)
                break;

            stream.next_out = output_buffer;
            stream.avail_out = BUFFER_SIZE;

            bzerror = BZ2_bzCompress(&stream, BZ_RUN);

        } while (stream.avail_out == 0);

    } while (bzerror == BZ_RUN_OK);

    // Finalizar a compressão
    do {
        output_size = output_callback(output_buffer, BUFFER_SIZE - stream.avail_out, output_file);
        if (output_size == 0)
            break;

        stream.next_out = output_buffer;
        stream.avail_out = BUFFER_SIZE;

        bzerror = BZ2_bzCompress(&stream, BZ_FINISH);

    } while (bzerror == BZ_FINISH_OK);

    BZ2_bzCompressEnd(&stream);
}