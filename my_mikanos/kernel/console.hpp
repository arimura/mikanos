#pragma once

#include 'graphics.hpp'

class Console {
    public:
        static const int kRows = 35, kColumns = 80;

        Console(PixelWriter& writer,
            const PixelColor& fg_color, const PixelColor& bg_color);
        void PutString(const chat* s);

        private:
            void NewLine();
            PixelWriter& writer_;
            const PixelColor fg_color, bg_color;
            char buffer_[kRows][kColumns + 1];
            int cursor_row_, cursor_column_;
};