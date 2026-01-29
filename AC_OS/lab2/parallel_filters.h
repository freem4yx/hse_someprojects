#pragma once
#include "filters.h"
#include "parallel_processor.h"
#include <functional>
#include <cmath>
#include <cstdlib>
#include <ctime>

static int get_channels_parallel(png_byte color_type) {
    switch (color_type) {
        case PNG_COLOR_TYPE_GRAY: return 1;
        case PNG_COLOR_TYPE_GRAY_ALPHA: return 2;
        case PNG_COLOR_TYPE_RGB: return 3;
        case PNG_COLOR_TYPE_RGBA: return 4;
        case PNG_COLOR_TYPE_PALETTE: return 3;
        default: return 3;
    }
}

class ParallelFilters {
public:
    static void parallel_color_inverter(Image& img, int num_tasks = 16) {
        ParallelImageProcessor processor;

        auto task_generator = [&img](int task_id) -> ImageTask {

            ImageTask task;
            task.task_id = task_id;

            int rows_per_task = img.height / 16;
            task.start_row = task_id * rows_per_task;
            task.end_row = (task_id == 15) ? img.height : (task_id + 1) * rows_per_task;

            task.processor = [&img, start=task.start_row, end=task.end_row](int, int) {
                int channels = get_channels_parallel(img.color_type);
                bool hasAlpha = img.color_type & PNG_COLOR_MASK_ALPHA;

                for (int y = start; y < end; ++y) {
                    png_bytep row = img.row_pointers[y];
                    for (int x = 0; x < img.width; ++x) {
                        for (int ch = 0; ch < (hasAlpha ? channels - 1 : channels); ++ch) {
                            row[x * channels + ch] = 255 - row[x * channels + ch];
                        }
                    }
                }
            };

            return task;
        };

        processor.process_tasks(task_generator, num_tasks);
    }

    static void parallel_add_noise(Image& img, int intensity, int num_tasks = 16) {

        srand(time(nullptr));

        ParallelImageProcessor processor;

        auto task_generator = [&img, intensity](int task_id) -> ImageTask {
            ImageTask task;
            task.task_id = task_id;

            int rows_per_task = img.height / 16;
            task.start_row = task_id * rows_per_task;
            task.end_row = (task_id == 15) ? img.height : (task_id + 1) * rows_per_task;

            task.processor = [&img, intensity, start=task.start_row, end=task.end_row](int, int) {
                int channels = get_channels_parallel(img.color_type);

                for (int y = start; y < end; ++y) {
                    png_bytep row = img.row_pointers[y];
                    for (int x = 0; x < img.width; ++x) {
                        png_bytep px = &(row[x * channels]);

                        int noise_r = (rand() % (intensity * 2)) - intensity;
                        int noise_g = (rand() % (intensity * 2)) - intensity;
                        int noise_b = (rand() % (intensity * 2)) - intensity;

                        px[0] = std::max(0, std::min(255, (int)px[0] + noise_r));
                        if (channels >= 2) px[1] = std::max(0, std::min(255, (int)px[1] + noise_g));
                        if (channels >= 3) px[2] = std::max(0, std::min(255, (int)px[2] + noise_b));
                    }
                }
            };

            return task;
        };

        processor.process_tasks(task_generator, num_tasks);
    }


    static Image parallel_gray_scale(Image& img, int num_tasks = 16) {

        png_byte color_type = PNG_COLOR_TYPE_GRAY;
        if (img.color_type == PNG_COLOR_TYPE_RGBA || img.color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
            color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
        }

        Image result = {
            img.width, img.height,
            color_type,
            img.bit_depth,
            nullptr
        };

        int channels_grayscale = (color_type == PNG_COLOR_TYPE_GRAY) ? 1 : 2;
        result.row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * result.height);
        for (int y = 0; y < result.height; y++) {
            result.row_pointers[y] = (png_byte*)malloc(result.bit_depth / 8 * channels_grayscale * img.width);
        }

        int src_channels = get_channels_parallel(img.color_type);

        ParallelImageProcessor processor;

        auto task_generator = [&img, &result, src_channels, channels_grayscale](int task_id) -> ImageTask {
            ImageTask task;
            task.task_id = task_id;

            int rows_per_task = result.height / 16;
            task.start_row = task_id * rows_per_task;
            task.end_row = (task_id == 15) ? result.height : (task_id + 1) * rows_per_task;

            task.processor = [&img, &result, src_channels, channels_grayscale, start=task.start_row, end=task.end_row](int, int) {
                for (int y = start; y < end; ++y) {
                    png_bytep src_row = img.row_pointers[y];
                    png_bytep dst_row = result.row_pointers[y];

                    for (int x = 0; x < img.width; ++x) {
                        if (src_channels == 1) {
                            dst_row[x * channels_grayscale] = src_row[x * src_channels];
                        } else if (src_channels == 2) {
                            dst_row[x * channels_grayscale] = src_row[x * src_channels];
                            dst_row[x * channels_grayscale + 1] = src_row[x * src_channels + 1];
                        } else {
                            png_byte red = src_row[x * src_channels];
                            png_byte green = src_row[x * src_channels + 1];
                            png_byte blue = src_row[x * src_channels + 2];
                            png_byte gray_byte = red * 0.2126 + green * 0.7152 + blue * 0.0722; //Wikipedia

                            dst_row[x * channels_grayscale] = gray_byte;
                            if (channels_grayscale == 2) {
                                dst_row[x * channels_grayscale + 1] =
                                    src_channels == 4 ? src_row[x * src_channels + 3] : 255;
                            }
                        }
                    }
                }
            };

            return task;
        };

        processor.process_tasks(task_generator, num_tasks);

        return result;
    }
};
