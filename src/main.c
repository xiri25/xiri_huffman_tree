#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <file.h>
#include <huffman_encoding.h>
#include <frequencies.h>
#include <huffman_tree.h>
#include <memory/arena.h>

#include <ASSERT.h>


/*
 * TODO: Deberia crear un diccionario con el huffman_code para cada caracter
 * y usar ese diccionario para en el encoding y decoding, ahorrandome atravesar
 * el arbol.
 * Puede que tambien quiera una implementacion atravesando el arbol para
 * usar menos memoria RAM
 * A la hora de crear el diccionario usar Depth‑first leaf‑only traversal
 * (es recursivo, pero hay manera de hacerlo iterativo)
 *
 * https://es.wikipedia.org/wiki/Verificaci%C3%B3n_de_redundancia_c%C3%ADclica
 *
 * En la libreria estandar de zig hay huffman encoding
 * https://ziglang.org/documentation/master/std/#std.compress.flate.Compress.Huffman.init
 * comparar/intentar descomprimir un archivo comprimido por mi con esto y viceversa
 *
 * Molaria, dentro de mucho, hecharle un vistazo a deflate (zlib) que usa huffman LZ77
 *
 * Si en algun momento, lo escribo de forma que escribba un byte cada vez, puedo intentar
 * ver si mejora cuando represento el byte como una struct en plan:
 * struct {
 *  bit1 : 1...
 * }
*/

#define PRINT_DEBUG(x) printf(x);
// #define PRINT_DEBUG(x)

static int64_t root_weight_minus_freq(const struct ht_tree* tree,
                                       const struct char_freq* char_freq,
                                       const size_t char_freq_count)
{
    int64_t root_minus_chars_freqs = (int64_t)tree->tree[tree->root_idx].weight;
    for (size_t i = 0; i < char_freq_count; i++) {
        root_minus_chars_freqs -= (int64_t)char_freq[i].freq;
    }

    return root_minus_chars_freqs;
}

static void bits_form_uint8_t_to_str(const uint8_t byte, char* str)
{
    for (uint32_t i = 0; i < 8; i++) {
        str[i] = ((byte >> (7 - i)) & 1) ? '1' : '0';
    }
    str[8] = '\0';
}

static void bits_form_uint16_t_to_str(const uint16_t bytes, char* str)
{
    for (uint32_t i = 0; i < 16; i++) {
        str[i] = ((bytes >> (15 - i)) & 1) ? '1' : '0';
    }
    str[16] = '\0';
}

// /* Numero de hilos */
// #include <unistd.h>
// static long cpu_cores_online(void)
// {
//     long n = sysconf(_SC_NPROCESSORS_ONLN);
//     if (n < 1) n = 1;
//     return n;
// }

// bit i [0, 7] starting from left set to 1
static void set_bit(uint8_t* a, const uint8_t i)
{
    ASSERT(a != NULL);
    ASSERT(i < 8);
    const uint8_t mask = (uint8_t)(1U << i);
    *a = *a | mask;
}

// bit i [0, 7] starting from left set to 0
static void clear_bit(uint8_t* a, const uint8_t i)
{
    ASSERT(a != NULL);
    ASSERT(i < 8);
    const uint8_t mask = (uint8_t)(~((1U << i)));
    *a = *a & mask;
}

// bit i [0, 7] starting from left toggle
static void toggle_bit(uint8_t* a, const uint8_t i)
{
    ASSERT(a != NULL);
    ASSERT(i < 8);
    const uint8_t mask = (uint8_t)(1U << i);
    *a = *a ^ mask;
}

// return the i [0, 7] bit (value) starting from left
static uint8_t read_bit(const uint8_t* a, const uint8_t i)
{
    ASSERT(a != NULL);
    ASSERT(i < 8);
    return (*a & (1U << i)) != 0;
}

// return the i [0, 15] bit (value) starting from left
static uint8_t read_bit_uint16(const uint16_t* a, const uint8_t i)
{
    ASSERT(a != NULL);
    ASSERT(i < 16); // Change to 16 for uint16_t
    return (*a & (1U << i)) != 0; // Return 1 if the bit is set, otherwise 0
}

static void set_bit_value(uint8_t* a, const uint8_t i, const uint8_t value)
{
    ASSERT(a != NULL);
    ASSERT(i < 8);
    ASSERT(value == 0 || value == 1);
    switch (value) {
        case 0:
            clear_bit(a, i);
            break;
        case 1:
            set_bit(a, i);
            break;
    }
}

/* Si el return es menor que el code_len, volver a pasar el mismo codigo, pero con code_len = return llamada anterior */
static uint8_t append_code_bits(uint8_t* buffer,
                                const uint8_t buffer_offset,
                                const uint16_t code,
                                const uint8_t code_len)
{
    /*
    char code_str[17] = {};
    bits_form_uint16_t_to_str(code, code_str);
    printf("append_code -> %s, code_len = %u\n", code_str, code_len);
    */
    const uint8_t byte_capacity = 8 - buffer_offset;
    if (byte_capacity >= code_len) {
        // Append bits beginning at "index" 16 - code_len (of code) , append code_len bits, at buffer at buffer_offset
        for (uint8_t i = 0; i < code_len; i++) {
            ASSERT(code_len - 1 >=  i /* El offset para obtener el code_bit tiene que ser mayor de 0 */);
            const uint8_t code_bit = read_bit_uint16(&code, (uint8_t)(code_len - i - 1));
            const uint8_t index = buffer_offset + i;
            ASSERT(index < 8);
            //printf("append -> %u, buffer_ptr = %p, index = %u\n", code_bit, buffer, buffer_offset + i);
            set_bit_value(buffer, index, code_bit);
        }
        return code_len;
    } else {
        // Append bits beginning at "index" 16 - code_len (of code), append byte_capacity bits, at buffer at buffer_offset
        for (uint8_t i = 0; i < byte_capacity; i++) {
            ASSERT(code_len - 1 >=  i /* El offset para obtener el code_bit tiene que ser mayor de 0 */);
            const uint8_t code_bit = read_bit_uint16(&code, (uint8_t)(code_len - 1 - i));
            const uint8_t index = buffer_offset + i;
            ASSERT(index < 8);
            //printf("append -> %u, buffer_ptr = %p, index = %u\n", code_bit, buffer, buffer_offset + i);
            set_bit_value(buffer, index, code_bit);
        }
        return byte_capacity;
    }
}

uint64_t encode_chunk(const struct ht_dict* dict,
                      const uint64_t chunk_size,
                      const uint8_t* file_buffer,
                      const uint64_t bit_pos,
                      char* outbuf)
{
    uint64_t outbuf_pos_bits = bit_pos;
    for (uint64_t i = 0; i < chunk_size; i++) {
        const uint16_t code = dict[file_buffer[i]].code;
        const uint8_t code_len = dict[file_buffer[i]].len;

        // I need to figure out in what byte am i, and in what offset of the byte
        const uint64_t outbuf_pos_byte = outbuf_pos_bits / 8;
        const uint8_t byte_offset = outbuf_pos_bits % 8;
        uint8_t bits_written = 0;
        uint8_t bits_written_2 = 0;
        uint8_t bits_written_3 = 0;

        // Copy the last code_len bits of code to outbuf at bit outbuf_pos_byte + byte_offset
        // The max len of a code is 16 nits, there are being writen in 8 bits groups
        // so they could need 3 8 bit writes to be complete
        bits_written = append_code_bits((uint8_t*)&outbuf[outbuf_pos_byte],
                                        byte_offset,
                                        code,
                                        code_len);
        if (bits_written < code_len) {
            // No se ha appendeado todo el codigo
            bits_written_2 = append_code_bits((uint8_t*)&outbuf[outbuf_pos_byte + code_len / 8 + 1],
                                              0, // Acabo de pasar al siguiente byte
                                              code,
                                              code_len - bits_written);
        }

        if ((bits_written + bits_written_2) < code_len) {
            // No se ha appendeado todo el codigo
            bits_written_3 = append_code_bits((uint8_t*)&outbuf[outbuf_pos_byte + code_len / 8 + 2],
                                              0, // Acabo de pasar al siguiente byte
                                              code,
                                              code_len - (uint8_t)(bits_written + bits_written_2));
        }

        ASSERT(code_len == bits_written + bits_written_2 + bits_written_3);
        outbuf_pos_bits += bits_written;
        outbuf_pos_bits += bits_written_2;
        outbuf_pos_bits += bits_written_3;
    }

    const uint64_t total_bits_written = outbuf_pos_bits - bit_pos;
    return total_bits_written;
}

int32_t file_raw_open_to_write(struct file_raw_t* file, const char* filepath)
{
    const int fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, 0660);
    if (fd < 0) {
        perror("Error opening file");
        return 1;
    }

    // Ensure the file is extended to 4096 bytes
    if (ftruncate(fd, 4096) == -1) {
        perror("ftruncate error");
        close(fd);
        return 2;
    }

    file->fd = fd;
    file->size = 4096;

    return 0;
}

/* Make some texts to make sure that it keeps working throught the changes */
int main(int32_t argc, char** argv)
{
    (void)argc;
    const uint32_t path_max_len = 256;
    const char* filepath = argv[1];
    const size_t filepath_len = strnlen(filepath, path_max_len);
    if (filepath_len == path_max_len) {
        printf("Un path mayor de %u caracteres?????\n", path_max_len);
        return 1;
    }

    const size_t sorted_freq_len = UINT8_MAX + 1;
    struct char_freq* sorted_freq = char_freq_buffer_alloc();

    struct file_raw_t file = {};
    ASSERT(file_raw_open(&file, filepath) == 0  /* Error opening the file */);
    huffman_tree_calc_freq_from_file(&file, sorted_freq, (uint16_t)sorted_freq_len);

    char_freq_buffer_print_truncated(sorted_freq, sorted_freq_len);

    arena_t ht_arena = arena_create(8192 * 8);

    /*
     * NOTE: The tree can be as large as 2*(leaf_nodes) - 1
     * x + x/2 + x/4 + x/8 ..... + 1
    */
    const size_t ht_tree_size = sizeof(struct ht_node) * (sorted_freq_len * 2 - 1);
    struct ht_node* ht_tree_buffer = arena_alloc(&ht_arena, ht_tree_size);

    struct ht_tree tree = huffman_tree_create(sorted_freq, sorted_freq_len, ht_tree_buffer);

    ht_tree_print(&tree, false);

    ASSERT(root_weight_minus_freq(&tree, sorted_freq, sorted_freq_len) == 0);

    struct ht_dict* dict = arena_alloc(&ht_arena, sizeof(struct ht_dict) * 256);

    // FIXME: without memset it fails in some files, idk why just in some files
    memset(dict, 0, 256 * sizeof(struct ht_dict));
    huffman_dict_create(&tree, dict);

    ht_dict_print_truncated(dict, 256);
    
    /*
     * NOTE: To compare
     * https://suhaan-bhandary.github.io/Huffman-Coding/
     * No lo ordena como yo, por lo que los codigos generados
     * sin distintos, pero equivalentes (creo)
     */
    /*
     * NOTE: This structs are not packet, so maybe a little smaller
     * after serialization
     */
    const uint64_t original_size_bits = tree.tree[tree.root_idx].weight * sizeof(uint8_t) * 8;
    const uint8_t tree_header_size_bits = (sizeof(struct ht_tree) - sizeof(struct ht_node*)) * 8;
    const uint64_t tree_size_bits = tree.node_count * sizeof(struct ht_node) * 8;
    uint64_t compress_size_wo_tree_bits = 0;

    for (uint16_t i = 0; i < 256; i++) {
        uint64_t repeticions = 0;
        // XD
        for (uint16_t j = 0; j < sorted_freq_len; j++) {
            if (sorted_freq[j].c == i) {
                repeticions = sorted_freq[j].freq;
            }
        }
        compress_size_wo_tree_bits += dict[i].len * repeticions;
    }

    /*
     * TODO: Check size of original file
     * FIXME: El video grande produce todos los codigos con len 8 ??? idk what should happen
     * FIXME: Cat file | wc -c no siempre coincide con original_size_bits / 8
     */

    struct file_raw_t out_file = {};
    char outpath[path_max_len];
    snprintf(outpath, path_max_len - strlen(".huf\0"), "%s.huf", filepath);
    ASSERT(file_raw_open_to_write(&out_file, outpath) == 0);
    {
        /*
         * TODO: Try:
         * Appendear bytes enteros al buffer, generar los códigos juntos no bit por bit
         * Intentar escribir set_bit para 0 y 1 y probar a ver si es mejor
         */
        size_t chars_read = 0;

        /* TODO: Magic Numbers */
        const size_t CHUNK_SIZE = 1024LU * 1024 * 4;
        const size_t min_mmap_size = 4096;

        ASSERT((CHUNK_SIZE * 2 + 1) < UINT64_MAX);

        /*
         * NOTE: CHUNK_SIZE * 2 bc at most is using double space than original per char
         *       + 1 for the \0
         */

        uint64_t outbuf_pos_bits = 0;
        while (chars_read < file.size) {
            size_t map_size_in = 0;
            size_t chars_to_read = 0;

            if ((file.size - chars_read) > CHUNK_SIZE) {
                map_size_in = CHUNK_SIZE;
                chars_to_read = CHUNK_SIZE;
            } else if (((file.size - chars_read) < CHUNK_SIZE) && ((file.size - chars_read) > min_mmap_size)) {
                map_size_in = file.size - chars_read;
                chars_to_read = file.size - chars_read;
            } else {
                map_size_in = min_mmap_size;
                chars_to_read = file.size - chars_read;
            }

            ASSERT(map_size_in >= min_mmap_size);
            ASSERT(chars_to_read <= map_size_in);

            /* NOTE:
             * 2 * map_size_in + 1 porque cada codigo puede ser de
             * hasta 16bits y uno de los bytes puede no estar completo
             */
            const size_t map_size_out = 2 * map_size_in + 1;

            /*
             * NOTE:
             * EL offset en el archivo tiene que ser multiplo de page_size
             * por lo que tengo que tener en cuenta cuantos bytes he escrito
             * para saber que region del archivo mmap, donde he dejado de escribir
             * y incluso cuanto he de mapear para no quedarme sin espacio
             * En el man no pone nada de que el offset tenga que ser multiplo
             * (que yo halla leido) pero si no pongo 4096 * n perror print
             * Invalid Argument
             */
            ASSERT(chars_read % 4096 == 0);

            uint8_t* file_buffer = mmap(NULL, map_size_in, PROT_READ, MAP_PRIVATE, file.fd, (int64_t)chars_read);
            if (file_buffer == MAP_FAILED) {
                perror("mmap error");
                close(file.fd);
                break; // Aqui habia un return
            }

            char* outbuf = mmap(NULL, map_size_out, PROT_READ | PROT_WRITE, MAP_SHARED, out_file.fd, 0);
            if (outbuf == MAP_FAILED) {
                perror("mmap error");
                close(out_file.fd);
                //return 3;
            }

            /*
             * NOTE: It shouldn't overflow bc (CHUNK_SIZE * 2 + 1) * 8 bits < UINT64_MAX
             */
            outbuf_pos_bits += encode_chunk(dict, chars_to_read, file_buffer, outbuf_pos_bits, outbuf);
            /* FIXME: Estoy syncing de mas (todo el buffer, independientemente de lo que he escrito) */
            msync(outbuf, map_size_out, MS_SYNC);
            munmap(outbuf, map_size_out);

            if (munmap(file_buffer, map_size_in) == -1) {
                perror("munmap error");
                close(file.fd);
                break; // Aqui habia un return
            }

            chars_read += chars_to_read;
            printf("Encoded = %f%%\n", (float)chars_read / (float)file.size * 100);
        }

        char* outbuf = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, out_file.fd, 0);
        if (outbuf == MAP_FAILED) {
            perror("Padding mmap error");
            close(out_file.fd);
            /* FIXME: Handle the error */
            //return 3;
        }
        //printf("Padding\n");
        const uint8_t padding_needed_len = 8 - (outbuf_pos_bits % 8);
        append_code_bits((uint8_t*)&outbuf,
                         8 - padding_needed_len,
                         (uint16_t)0,
                         padding_needed_len);

        msync(outbuf, 4096, MS_SYNC);
        ASSERT((outbuf_pos_bits + padding_needed_len) % 8 == 0);
        const int64_t bytes_written = (int64_t)(outbuf_pos_bits + padding_needed_len) / 8;
        out_file.size = (size_t)bytes_written;
        printf("bytes_written = %ld (%ldbits)\n", bytes_written, bytes_written * 8);
        ftruncate(out_file.fd, bytes_written);
        munmap(outbuf, 4096);
    }

    printf("orginal file size bits = %lu\n", file.size * 8);
    printf("original_size_bits =  %lu, compress_size_wo_tree_bits = %lu, compress_size_w_tree_bits = %lu\n",
           original_size_bits, compress_size_wo_tree_bits,
           compress_size_wo_tree_bits + tree_size_bits + tree_header_size_bits);

    printf("arena: offset(used) = %lu Bytes, capacity = %lu Bytes, used(%%) = %lf %%\n",
           ht_arena.offset, ht_arena.capacity,
           ((double)ht_arena.offset / (double)ht_arena.capacity) * 100.0);

    /*
     * Comprobación
     */
    {
        /* TODO: Tener en cuenta el Padding */
        ASSERT(out_file.size <= 4096);
        const uint8_t* outbuf = mmap(NULL, 4096, PROT_READ, MAP_SHARED, out_file.fd, 0);

        const struct ht_node root = tree.tree[tree.root_idx];

        struct ht_node actual_node = root;
        uint16_t actual_node_idx = tree.root_idx;

        const uint64_t file_size_bits = out_file.size * 8;
        uint64_t i = 0;
        while (i < file_size_bits) {
            const uint64_t outbuf_idx = i / 8;
            const uint8_t bit_idx = i % 8;

            //printf("i = %lu, outbuf_idx = %lu, bit_idx = %u\n", i, outbuf_idx, bit_idx);
            //printf("%p, %u\n", &outbuf[outbuf_idx], bit_idx);
            
            if (ht_node_is_leaf(&actual_node)) {
                printf("%c", (unsigned char)actual_node.c);
                actual_node = root;
                /*
                 * NOTE: Es un bucle while porque aqui hay
                 * no se toma una nueva rama del arbol, si
                 * no que se vuelve al root, y por lo tanto,
                 * en esta iteracción no se ha avanzado en
                 * la lectura del archivo
                */
                continue;
            }

            const uint8_t bit_value = read_bit(&outbuf[outbuf_idx], bit_idx);

            if (bit_value == 0) {
                ASSERT(actual_node.left_node >= 0);
                //printf("left\n");
                actual_node_idx = (uint16_t)actual_node.left_node;
            } else if (bit_value == 1) {
                ASSERT(actual_node.right_node >= 0);
                //printf("right\n");
                actual_node_idx = (uint16_t)actual_node.right_node;
            } else {
                ASSERT(0);
            }
            actual_node = tree.tree[actual_node_idx];
            i++;
        }

        munmap((void*)outbuf, 4096);
    }

    arena_destroy(&ht_arena);

    char_freq_buffer_free(sorted_freq);

    file_raw_close(&file);
    file_raw_close(&out_file);

    return 0;
}
