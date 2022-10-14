#include <algorithm>

#include "pngpp.h"

pngpp::pngpp(const std::filesystem::path& source_path)
{
    unsigned char header[8];
    FILE *fp = fopen(std::move(source_path).c_str(), "rb");
    if (!fp)
    {
        throw std::runtime_error("Pngpp: Open Input");
    }
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8))
    {
        throw std::runtime_error("Pngpp: Signature");
    }
    auto png_ptr = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
    {
        throw std::runtime_error("Convereter: Read Struct");
    }
    auto info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        throw std::runtime_error("Pngpp: Info Struct");
    }
    if (setjmp(png_jmpbuf(png_ptr)) != 0)
    {
        throw std::runtime_error("Pngpp: Set Jump");
    }
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    width_ = png_get_image_width(png_ptr, info_ptr);
    height_ = png_get_image_height(png_ptr, info_ptr);
    switch (png_get_color_type(png_ptr, info_ptr))
    {
    case PNG_COLOR_TYPE_RGB:
        bytes_per_sample_ = 3;
        break;
    case PNG_COLOR_TYPE_RGBA:
        bytes_per_sample_ = 4;
        break;
    default:
        throw std::runtime_error("Pngpp: Color Type");
        break;
    }
    if (png_get_bit_depth(png_ptr, info_ptr) != 8)
    {
        throw std::runtime_error("Pngpp: Bit Depth");
    }
    number_of_passes_ = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        throw std::runtime_error("Pngpp: Set Jump");
    }
    if (height_ * png_get_rowbytes(png_ptr, info_ptr) > 0x4000000)
    {
        throw std::runtime_error("Pngpp: Too Big Image");
    }
    bitmap_ = (png_byte*) malloc(
        height_ * png_get_rowbytes(png_ptr, info_ptr));
    row_pointers_ = (png_bytep*) malloc(sizeof(png_bytep) * height_);
    for (int y = 0; y < height_; y++)
        row_pointers_[y] = bitmap_ + y * png_get_rowbytes(png_ptr, info_ptr);
    png_read_image(png_ptr, row_pointers_);
    fclose(fp);
    free(info_ptr);
    free(png_ptr);
}

pngpp::~pngpp()
{
    free(row_pointers_);
    free(bitmap_);
}

void pngpp::save(const std::filesystem::path& source_path)
{
    FILE *fp = fopen(std::move(source_path).c_str(), "wb");
    if (!fp)
    {
        throw std::runtime_error("Pngpp: Open Output");
    }
    auto png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
    {
        throw std::runtime_error("png_create_write_struct failed");
    }
    auto info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        throw std::runtime_error("png_create_info_struct failed");
    }
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        throw std::runtime_error("Error during init_io");
    }
    png_init_io(png_ptr, fp);
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        throw std::runtime_error("Error during writing header");
    }
    png_set_IHDR(png_ptr, info_ptr, width_, height_, 8,
        bytes_per_sample_ == 3 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        throw std::runtime_error("Error during writing bytes");
    }
    png_write_image(png_ptr, row_pointers_);
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        throw std::runtime_error("Error during end of write");
    }
    png_write_end(png_ptr, NULL);
    fclose(fp);
    free(info_ptr);
    free(png_ptr);
}

const std::size_t& pngpp::get_width() const
{
    return width_;
}

const std::size_t& pngpp::get_height() const
{
    return height_;
}

const unsigned char* pngpp::get_pixel(
    const std::size_t x, const std::size_t y) const
{
    return bitmap_ + y * width_ + x * bytes_per_sample_;
}
