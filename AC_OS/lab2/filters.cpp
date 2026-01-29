#include"filters.h"
#include <png.h>
#include <pngconf.h>

int get_channels(png_byte color_type) {
    switch (color_type) {
        case PNG_COLOR_TYPE_GRAY: return 1;
        case PNG_COLOR_TYPE_GRAY_ALPHA: return 2;
        case PNG_COLOR_TYPE_RGB: return 3;
        case PNG_COLOR_TYPE_RGBA: return 4;
        case PNG_COLOR_TYPE_PALETTE: return 3;
        default: return 3;
    }
}

void color_inverter(Image& img) {
    int channels = get_channels(img.color_type);
    bool hasAlpha = img.color_type & PNG_COLOR_MASK_ALPHA;

    for (size_t i = 0; i < img.height; ++i) {
        png_bytep row = img.row_pointers[i];
        for (size_t j = 0; j < img.width; ++j) {
            for (size_t ch = 0; ch < (hasAlpha ? (channels - 1) : channels); ++ch) {
                row[j * channels + ch] = 255 - row[j * channels + ch];
            }
        }
    }
}

Image gray_scale(Image& img) {
    png_byte color_type = PNG_COLOR_TYPE_GRAY;
    if (img.color_type == PNG_COLOR_TYPE_RGBA || img.color_type == PNG_COLOR_TYPE_GRAY_ALPHA) color_type = 4;
    Image result = {
        img.width, img.height,
        color_type,
        img.bit_depth,
        0
    };

    int channels_grayscale = (color_type == 0) ? 1 : 2;
    result.row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * result.height);
    for (int y = 0; y < result.height; y++) {
        result.row_pointers[y] = (png_byte*)malloc(result.bit_depth / 8 * channels_grayscale * img.width);
    }

    int channels = get_channels(img.color_type);

    for (size_t i = 0; i < img.height; ++i) {
        png_bytep row = img.row_pointers[i];
        for (size_t j = 0; j < img.width; ++j) {
            if (channels == 1) {
                result.row_pointers[i][j * channels_grayscale] = row[j * channels];
            } else if (channels == 2) {
                result.row_pointers[i][j * channels_grayscale] = row[j * channels];
                result.row_pointers[i][j * channels_grayscale + 1] = row[j * channels + 1];
            } else {
                png_byte red = row[j * channels];
                png_byte green = row[j * channels + 1];
                png_byte blue = row[j * channels + 2];
                png_byte gray_byte = red * 0.2126 + green * 0.7152 + blue * 0.0722; // Wikipedia
                result.row_pointers[i][j * channels_grayscale] = gray_byte;
                result.row_pointers[i][j * channels_grayscale + 1] = row[j * channels + (channels - 1)]; //alpha channel is the last one
            }
        }
    }
    return result;
}

template<typename T>
void swap(T& left, T& right) {
    T temp = left;
    left = right;
    right = temp;
}

void flip_horizontally(Image& img) {
    int channels = get_channels(img.color_type);

    for (size_t i = 0; i < img.height; ++i) {
        png_bytep row = img.row_pointers[i];
        for (size_t j = 0; j < img.width / 2; ++j) {
            for (size_t ch = 0; ch < channels; ++ch) {
                swap(row[j * channels + ch], row[(img.width - j - 1) * channels + ch]);
            }
        }
    }
}

void flip_vertically(Image& img) {
    int channels = get_channels(img.color_type);

    for (size_t i = 0; i < img.height / 2; ++i) {
        png_bytep row_upper = img.row_pointers[i];
        png_bytep row_lower = img.row_pointers[img.height - i - 1];
        for (size_t j = 0; j < img.width; ++j) {
            for (size_t ch = 0; ch < channels; ++ch) {
                swap(row_upper[j * channels + ch], row_lower[j * channels + ch]);
            }
        }
    }
}

void add_noise(Image& img, int intensity) {
    int channels = get_channels(img.color_type);

    for (int y = 0; y < img.height; y++) {
        png_bytep row = img.row_pointers[y];
        for (int x = 0; x < img.width; x++) {
            png_bytep px = &(row[x * channels]);

            int noise_r = (rand() % (intensity * 2)) - intensity;
            int noise_g = (rand() % (intensity * 2)) - intensity;
            int noise_b = (rand() % (intensity * 2)) - intensity;

            px[0] = std::max(0, std::min(255, (int)px[0] + noise_r));
            px[1] = std::max(0, std::min(255, (int)px[1] + noise_g));
            px[2] = std::max(0, std::min(255, (int)px[2] + noise_b));
        }
    }
}

template<typename T>
void swap(T& first, T& second, T& third, T& fourth) {
    T temp = first;
    first = fourth;
    fourth = third;
    third = second;
    second = temp;
}

Image create_rotated(Image& img) {
    Image result = {
        img.height, img.width,
        img.color_type,
        img.bit_depth,
        0
    };

    result.row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * result.height);
    for (int y = 0; y < result.height; y++) {
        result.row_pointers[y] = (png_byte*)malloc(result.bit_depth / 8 * get_channels(result.color_type) * result.width);
    }
    return result;
}

Image rotate_clockwise(Image& img) {
    int channels = get_channels(img.color_type);
    Image result = create_rotated(img);

    for (size_t i = 0; i < result.height; ++i) {
        for (size_t j = 0; j < result.width; ++j) {
            for (size_t k = 0; k < channels; ++k) {
                result.row_pointers[i][j * channels + k] = img.row_pointers[result.width - j - 1][(i) * channels + k];
            }
        }
    }
    return result;
}

Image rotate_counterclockwise(Image& img) {
    int channels = get_channels(img.color_type);
    Image result = create_rotated(img);

    for (size_t i = 0; i < result.height; ++i) {
        for (size_t j = 0; j < result.width; ++j) {
            for (size_t k = 0; k < channels; ++k) {
                result.row_pointers[i][j * channels + k] = img.row_pointers[j][(result.height - i - 1) * channels + k];
            }
        }
    }
    return result;
}
