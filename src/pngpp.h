#ifndef SRC_PAINTER_H
#define SRC_PAINTER_H

#include <array>
#include <filesystem>

#include <png.h>

class pngpp
{
private:
    std::size_t width_;
    std::size_t height_;
    std::size_t bytes_per_sample_;
    int number_of_passes_;
    png_bytep* row_pointers_;
    png_bytep bitmap_;

public:
    pngpp(const std::filesystem::path& source_path);
    ~pngpp();
    void save(const std::filesystem::path& source_path);
    const std::size_t& get_width() const;
    const std::size_t& get_height() const;
    const unsigned char* get_pixel(
        const std::size_t x, const std::size_t y) const;
};

#endif // SRC_PAINTER_H
