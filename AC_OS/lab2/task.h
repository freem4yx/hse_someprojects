#pragma once
#include <functional>
#include <vector>
#include <memory>

struct ProcessingContext {
    const void* input_image;

    void* output_image;

    void* buffer;

    int param1;
    float param2;
};
