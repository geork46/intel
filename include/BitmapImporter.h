/**
 * \file BitmapImporter.h  
 * \brief Contains declarations used to parse and hold the .bmp data.
 * \author Cédric Andreolli (Intel)
 * \date 10 April 2013
 */
#ifndef BITMAP_IMPORTER_H
#define BITMAP_IMPORTER_H

#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <stdint.h>

namespace Accelerate{

struct HeaderStr{
    //BMP header
    uint16_t magic_number;
    uint32_t size;
    uint32_t reserved;
    uint32_t offset;
    //DIB header
    uint32_t dibSize;
    uint32_t width;
    uint32_t height;
    uint16_t plane;
    uint16_t bit_per_pixel;
    uint32_t compression;
    uint32_t data_size;
    uint32_t hor_res;
    uint32_t vert_res;
    uint32_t color_number;
    uint32_t important;
} __attribute__((packed));

struct PixelStr{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} __attribute__((packed));


class Image {
protected:
    PixelStr* pixel_data;
    unsigned int width;
    unsigned int height;
    Image();
    Accelerate::HeaderStr header;


	void copy_column(Accelerate::Image& result, unsigned int column_number, unsigned int scale) const;	
public:
    ~Image();

	PixelStr get_pixel(unsigned int i, unsigned int j) const;

    Image scale_image(unsigned int scale) const;


	void set_pixels(PixelStr* data){ this->pixel_data = data; }
	PixelStr* get_pixels(){ return pixel_data; }
	
	void set_width(const unsigned int width){ this->width = width; }
	unsigned int get_width()const { return this->width; }

	void set_height(const unsigned int height){ this->height = height; }
	unsigned int get_height()const { return this->height; }

    static Image create_image_from_bitmap(const std::string file_name);
    void write(std::ostream &out);
};

}

std::ostream& operator<<(std::ostream &o, const Accelerate::PixelStr& p);
std::ostream& operator<<(std::ostream &o, Accelerate::Image& im);
#endif
