#pragma once

#include "graphics.hpp"

class MouseCursor {
    public:
        MouseCursor(PixelWriter* writer, PixelColor erase_color,
                    Vector2D<int> initial_posotion);
        void MoveRelative(Vector2D<int> displacemnet);

    private:
        PixelWriter* pixel_writer_ = nullptr;
        PixelColor erase_color_;
        Vector2D<int> position_;
};
