#include <png.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
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

void free_image(Image &image) {
    if (image.row_pointers) {
        for (int y = 0; y < image.height; y++) {
            free(image.row_pointers[y]);
        }
        free(image.row_pointers);
        image.row_pointers = nullptr;
    }
}

int main() {
    std::cout << "=== БЕНЧМАРК ПАРАЛЛЕЛЬНОЙ ОБРАБОТКИ ===" << std::endl;

    Image image = read_png_file("large_input.png");
    std::cout << "Изображение загружено: " << image.width << "x" << image.height << std::endl;
    std::cout << "Количество потоков в системе: " << std::thread::hardware_concurrency() << std::endl;

    std::vector<int> task_counts = {1, 2, 4, 8, 16, 32, 64};

    std::cout << "\n=== Тест инверсии с разным количеством задач ===" << std::endl;
    for (int tasks : task_counts) {

        Image test_img = read_png_file("large_input.png");

        auto start = std::chrono::high_resolution_clock::now();

        ParallelImageProcessor processor(std::thread::hardware_concurrency());

        auto task_generator = [&test_img, tasks](int task_id) -> ImageTask {
            ImageTask task;
            task.task_id = task_id;

            int rows_per_task = test_img.height / tasks;
            task.start_row = task_id * rows_per_task;
            task.end_row = (task_id == tasks - 1) ? test_img.height : (task_id + 1) * rows_per_task;

            task.processor = [&test_img, start=task.start_row, end=task.end_row](int, int) {
                int channels = 3;
                for (int y = start; y < end; ++y) {
                    png_bytep row = test_img.row_pointers[y];
                    for (int x = 0; x < test_img.width; ++x) {
                        for (int ch = 0; ch < channels; ++ch) {
                            row[x * channels + ch] = 255 - row[x * channels + ch];
                        }
                    }
                }
            };

            return task;
        };

        processor.process_tasks(task_generator, tasks);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "Задач: " << tasks << ", Время: " << duration.count() << " мс" << std::endl;

        free_image(test_img);
    }

    std::cout << "\n=== Сравнение последовательной и параллельной обработки ===" << std::endl;


    Image seq_img = read_png_file("large_input.png");
    auto start_seq = std::chrono::high_resolution_clock::now();
    color_inverter(seq_img);
    auto end_seq = std::chrono::high_resolution_clock::now();
    auto seq_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_seq - start_seq);
    std::cout << "Последовательная: " << seq_time.count() << " мс" << std::endl;
    free_image(seq_img);


    std::vector<int> thread_counts = {1, 2, 4, 8, 12, 16};
    for (int threads : thread_counts) {
        if (threads > std::thread::hardware_concurrency() * 2) continue;

        Image par_img = read_png_file("large_input.png");
        auto start_par = std::chrono::high_resolution_clock::now();

        ParallelImageProcessor processor(threads);

        auto task_generator = [&par_img](int task_id) -> ImageTask {
            ImageTask task;
            task.task_id = task_id;

            int rows_per_task = par_img.height / 16;
            task.start_row = task_id * rows_per_task;
            task.end_row = (task_id == 15) ? par_img.height : (task_id + 1) * rows_per_task;

            task.processor = [&par_img, start=task.start_row, end=task.end_row](int, int) {
                int channels = 3;
                for (int y = start; y < end; ++y) {
                    png_bytep row = par_img.row_pointers[y];
                    for (int x = 0; x < par_img.width; ++x) {
                        for (int ch = 0; ch < channels; ++ch) {
                            row[x * channels + ch] = 255 - row[x * channels + ch];
                        }
                    }
                }
            };

            return task;
        };

        processor.process_tasks(task_generator, 16);

        auto end_par = std::chrono::high_resolution_clock::now();
        auto par_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_par - start_par);

        std::cout << "Параллельная (" << threads << " потоков): " << par_time.count()
                  << " мс, Ускорение: " << (double)seq_time.count() / par_time.count() << "x" << std::endl;

        free_image(par_img);
    }

    free_image(image);
    std::cout << "\n=== БЕНЧМАРК ЗАВЕРШЕН ===" << std::endl;

    return 0;
}
