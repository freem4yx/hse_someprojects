#include <png.h>
#include <iostream>
#include <chrono>
#include "filters.h"
#include "parallel_filters.h"

Image read_png_file(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        std::cerr << "Ошибка открытия файла для чтения: " << filename << std::endl;
        exit(1);
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        fclose(fp);
        exit(1);
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, nullptr, nullptr);
        fclose(fp);
        exit(1);
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, nullptr);
        fclose(fp);
        exit(1);
    }

    png_init_io(png, fp);
    png_read_info(png, info);

    Image image;
    image.width = png_get_image_width(png, info);
    image.height = png_get_image_height(png, info);
    image.color_type = png_get_color_type(png, info);
    image.bit_depth = png_get_bit_depth(png, info);

    if (image.bit_depth == 16)
        png_set_strip_16(png);

    if (image.color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    if (image.color_type == PNG_COLOR_TYPE_GRAY && image.bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    png_read_update_info(png, info);

    size_t rowbytes = png_get_rowbytes(png, info);
    image.row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * image.height);
    for (int y = 0; y < image.height; y++) {
        image.row_pointers[y] = (png_byte*)malloc(rowbytes);
    }

    png_read_image(png, image.row_pointers);
    png_read_end(png, nullptr);

    fclose(fp);
    png_destroy_read_struct(&png, &info, nullptr);

    return image;
}

void write_png_file(const char *filename, Image &image) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        std::cerr << "Ошибка открытия файла для записи: " << filename << std::endl;
        exit(1);
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        fclose(fp);
        exit(1);
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, nullptr);
        fclose(fp);
        exit(1);
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        exit(1);
    }

    png_init_io(png, fp);

    png_set_IHDR(
        png,
        info,
        image.width, image.height,
        image.bit_depth,
        image.color_type,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );

    png_write_info(png, info);

    png_write_image(png, image.row_pointers);
    png_write_end(png, nullptr);

    fclose(fp);
    png_destroy_write_struct(&png, &info);
}

void free_image(Image &image) {
    if (image.row_pointers) {
        for (int y = 0; y < image.height; y++) {
            free(image.row_pointers[y]);
        }
        free(image.row_pointers);
        image.row_pointers = nullptr;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Использование: " << argv[0] << " <входной файл> <выходной файл>" << std::endl;
        return 1;
    }

    Image image = read_png_file(argv[1]);

    std::cout << "Изображение загружено: " << image.width << "x" << image.height << std::endl;
    std::cout << "Выберите операцию:" << std::endl;
    std::cout << "1. Инверсия цветов (последовательно)" << std::endl;
    std::cout << "2. Добавление шума (последовательно)" << std::endl;
    std::cout << "3. Оттенки серого (последовательно)" << std::endl;
    std::cout << "4. Отразить по горизонтали" << std::endl;
    std::cout << "5. Отразить по вертикали" << std::endl;
    std::cout << "6. Повернуть по часовой" << std::endl;
    std::cout << "7. Повернуть против часовой" << std::endl;
    std::cout << "8. Параллельная инверсия цветов" << std::endl;
    std::cout << "9. Параллельное добавление шума" << std::endl;
    std::cout << "10. Параллельные оттенки серого" << std::endl;

    int choice;
    std::cin >> choice;

    Image result;

    auto start_time = std::chrono::high_resolution_clock::now();

    switch (choice) {
        case 1:
            color_inverter(image);
            result = image;
            std::cout << "Цвета инвертированы (последовательно)" << std::endl;
            break;
        case 2:
            add_noise(image, 30);
            result = image;
            std::cout << "Добавлен шум (последовательно)" << std::endl;
            break;
        case 3:
            result = gray_scale(image);
            std::cout << "Оттенки серого (последовательно)" << std::endl;
            break;
        case 4:
            flip_vertically(image);
            result = image;
            std::cout << "Отразить по вертикали" << std::endl;
            break;
        case 5:
            flip_horizontally(image);
            result = image;
            std::cout << "Отразить по горизонтали" << std::endl;
            break;
        case 6:
            result = rotate_clockwise(image);
            std::cout << "Повернуть по часовой" << std::endl;
            break;
        case 7:
            result = rotate_counterclockwise(image);
            std::cout << "Повернуть против часовой" << std::endl;
            break;
        case 8:
            ParallelFilters::parallel_color_inverter(image);
            result = image;
            std::cout << "Цвета инвертированы (параллельно)" << std::endl;
            break;
        case 9:
            ParallelFilters::parallel_add_noise(image, 30);
            result = image;
            std::cout << "Добавлен шум (параллельно)" << std::endl;
            break;
        case 10:
            result = ParallelFilters::parallel_gray_scale(image);
            std::cout << "Оттенки серого (параллельно)" << std::endl;
            break;
        default:
            std::cout << "Неверный выбор, выход без изменений" << std::endl;
            result = image;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Время выполнения: " << duration.count() << " мс" << std::endl;

    write_png_file(argv[2], result);

    if (choice == 3 || choice == 6 || choice == 7 || choice == 10) {
        free_image(result);
    }

    free_image(image);

    std::cout << "Результат сохранен в: " << argv[2] << std::endl;

    return 0;
}
