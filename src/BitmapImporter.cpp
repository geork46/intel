/*!
 * \file BitmapImporter.cpp
 * \brief Contains the implementation of the functions declared in BitmapImporter.h
 * \author Cédric Andreolli (Intel)
 * \date 10 April 2013
 */
#include "BitmapImporter.h" 

using namespace std;

Image Image::create_image_from_bitmap(const std::string file_name){
    Image result;
	ifstream file(file_name.c_str(), ios::in|ios::binary|ios::ate);
	if (file.is_open())
  	{
    		file.seekg (0, ios::beg);
		//Read the header first
            file.read ((char*)&result.header, sizeof(HeaderStr));

        result.width = result.header.width;
        result.height = result.header.height;

		//Put the cursor on the BMP data
        file.seekg(result.header.offset, ios::beg);
        int bytes_per_pixel = result.header.bit_per_pixel / 8;
        int padding = ((result.header.width*bytes_per_pixel) % 4 == 0) ? 0 : 4 - ((result.header.width*bytes_per_pixel) % 4);
		//Allocate the size needed to read the BMP data
        int size = sizeof(char)*(result.header.height*(result.header.width + padding))*bytes_per_pixel;
		unsigned char* data = (unsigned char*) malloc(size);
		//Read the data
		file.read((char*)data, size);
        //Create the Bitmap object
        result.pixel_data = (PixelStr*) malloc(result.header.width*result.header.height*sizeof(PixelStr));
		unsigned int offset = 0;
		//In the Bitmap format, pixels are in a reversed order
        for(int i=result.header.height-1; i>=0; i--){
            for(int j=0; j<result.header.width; j++){
                result.pixel_data[i*result.header.width + j].b = data[offset++];
                result.pixel_data[i*result.header.width + j].g = data[offset++];
                result.pixel_data[i*result.header.width + j].r = data[offset++];
			}
			offset+=padding;			
		}
		file.close();
		free(data);
	}
    return result;
}

void Image::write(ostream &out)
{
    header.height = height;
    header.width = width;
    header.size = sizeof(HeaderStr) + height * width * 3;
    header.dibSize = 40;
    header.offset = sizeof(HeaderStr);
    header.compression = 0;
    header.data_size = 0;
    header.bit_per_pixel = 24;
    int padding = ((header.width*3) % 4 == 0) ? 0 : 4 - ((header.width*3) % 4);
    int t = sizeof(PixelStr);
    out.write((char*)&header, sizeof(HeaderStr));
    for(int i=header.height-1; i>=0; i--){
        for(int j=0; j<header.width; j++){
            out.put(pixel_data[i*header.width + j].b);
            out.put(pixel_data[i*header.width + j].g);
            out.put(pixel_data[i*header.width + j].r);
        }
        for (int j = 0; j < padding; ++j)
            out.put(0);
    }
}

Image::Image(){
		
}

Image::~Image(){
    free(pixel_data);
}

PixelStr Image::get_pixel(unsigned int i, unsigned int j) const{
	return this->pixel_data[this->width*i + j];
}

std::ostream& operator<<(std::ostream &o, const PixelStr& p){
    return o << "[" << (int)p.r << ", " << (int)p.g << ", " << (int)p.b  << "] ";
}

Image Image::scale_image(unsigned int scale) const{
    Image result;

    result.width = width * scale;
    result.height = height * scale;
	
    result.pixel_data = (PixelStr*) malloc(result.width*result.height*sizeof(PixelStr));

	for(unsigned int w=0; w<this->width; w++){
		for(unsigned int i=0; i<scale; i++)
            copy_column(result, w, scale);
	}
	
    return result;
}

void Image::copy_column(Image& result, unsigned int column_number, unsigned int scale) const{
	//retrieve the column indice in the result image
	unsigned int first_column_indice = column_number * scale;
	for(unsigned int i=0; i<scale; i++){
		for(unsigned int row=0; row<height; row++){
			for(unsigned int j=0; j<scale; j++){
				result.pixel_data[(row*scale + j)*result.width + first_column_indice + i] = this->pixel_data[row*this->width + column_number]; 
			}
		}
	}
} 


std::ostream& operator<<(std::ostream &o, Image& im){
    PixelStr* pixels = im.get_pixels();
    for(unsigned int i=0; i<im.get_height(); i++){
        for(unsigned int j=0; j<im.get_width(); j++){
            PixelStr pixel = pixels[i*im.get_width() + j];
            o<<pixel<<" ";
        }
        o<<std::endl;
    }

    return o;
}
