#pragma once
#include<png.h>
#include<cstdlib>
#include<algorithm> //std::min, std::max

#include<iostream> //debug

struct Image {
    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep *row_pointers = nullptr;
};

int get_channels(png_byte color_type);
void color_inverter(Image&);
void add_noise(Image&, int);
Image gray_scale(Image&);
void flip_vertically(Image&);
void flip_horizontally(Image&);
Image rotate_clockwise(Image&);
Image rotate_counterclockwise(Image&);
