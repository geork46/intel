#ifndef SOMESTUFF_H
#define SOMESTUFF_H

#include "BitmapImporter.h"

#include <string>
#include <list>
#include <stdint.h>

struct Parameters
{
    int nb_threads;
    std::string main_image_name;
    std::list<std::string> template_names;
    int max_scale;
};

struct Result
{
    int pattern_ID;
    int position_x;
    int position_y;
};

struct RotateHashItem
{
    int pattern_id;
    double angle;
    uint32_t *fullHash;
};

struct Point
{
    int x, y;
    Point(int x, int y) : x(x), y(y) {}
};

struct PointR
{
    double x, y;
    PointR(double x, double y) : x(x), y(y) {}
};

bool read_parameters(int argc, char* argv[], Parameters& parameters);

bool compare_results(Result& first, Result& second);

struct BigPixelStr
{
    int r;
    int g;
    int b;
    BigPixelStr();
    BigPixelStr(PixelStr const &);
};

bool operator != (BigPixelStr const & a, PixelStr const & b);

bool operator == (BigPixelStr const & a, PixelStr const & b);

BigPixelStr& operator+= (BigPixelStr & a, BigPixelStr const & b);

BigPixelStr& operator-= (BigPixelStr & a, BigPixelStr const & b);

BigPixelStr& operator/= (BigPixelStr & a, int b);

int32_t getHash(BigPixelStr *str2, int n);

bool analizeAnswer(int x, int y, int index, int num, int n, int scale, Image const & im);
#endif //SOMESTUFF_H
