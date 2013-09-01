#include "SomeStuff.h"

#include <iostream>
#include <cmath>
#include <vector>
#include <unordered_map>

using namespace std;

extern int ERROR_NORM;
extern vector<RotateHashItem> hashList;

int hammingDistance(uint32_t a, uint32_t b)
{
    int s = 0;
    a = a ^ b;
    while (a > 0)
    {
        s += a % 2;
        a /= 2;
    }
    return s;
}

bool analizeAnswer(int x, int y, int index, int num, int n, int scale, Image const & im)
{
    int sumError = 0;
    BigPixelStr * str2 = new BigPixelStr[n];
    int y0 = y - scale * num;
    for (int i = 0; i < n; ++i)
    {
        if (y0 + i * scale  - scale + 1 < 0 || y0 + i * scale >= im.get_height())
            continue;
        for (int j = 0; j < n; ++j)
        {
            str2[j] = BigPixelStr();
            for (int i1 = 0; i1 < scale; i1++)
                for (int i2 = 0; i2 < scale; i2++)
                    str2[j] += im.get_pixel(y0 + i * scale - i1, x + j *  scale + i2);
            str2[j] /= scale * scale;
        }
        uint32_t intHash = getHash(str2, n);
        sumError += hammingDistance(intHash, hashList[index].fullHash[i]);
        if (sumError > ERROR_NORM)
            return false;
    }
    delete []str2;
    return true;
}


int32_t getHash(BigPixelStr *str2, int n)
{
    double r(0), g(0), b(0);
    for (int j = 0; j < n; ++j)
    {
        r += str2[j].r;
        g += str2[j].g;
        b += str2[j].b;
    }
    r /= n;
    g /= n;
    b /= n;
    int32_t d = 0;
    for (int j = 0; j < n; ++j)
    {
        d |= (str2[j].r > r) ? 1 << (j * 3 ) : 0;
        d |= (str2[j].g > g) ? 1 << (j * 3 + 1) : 0;
        d |= (str2[j].b > b) ? 1 << (j * 3 + 2) : 0;
    }
    return d;
}

BigPixelStr::BigPixelStr() : r(0), g(0), b(0) {}

BigPixelStr::BigPixelStr(PixelStr const &o) : r(o.r), g(o.g), b(o.b)
{
}

bool operator != (BigPixelStr const & a, PixelStr const & b)
{
    return a.r != b.r || a.g != b.g || a.b != b.b;
}

bool operator == (BigPixelStr const & a, PixelStr const & b)
{
    return !(a != b);
}

BigPixelStr& operator+= (BigPixelStr & a, BigPixelStr const & b)
{
    a.r += b.r;
    a.g += b.g;
    a.b += b.b;
    return a;
}

BigPixelStr& operator-= (BigPixelStr & a, BigPixelStr const & b)
{
    a.r -= b.r;
    a.g -= b.g;
    a.b -= b.b;
    return a;
}

BigPixelStr& operator/= (BigPixelStr & a, int b)
{
    a.r = round((double) a.r / b);
    a.g = round((double) a.g / b);
    a.b = round((double) a.b / b);
    return a;
}

bool _read_parameters(int argc, char* argv[], Parameters& parameters)
{
    if(argc < 4) return false;
    if (argv == 0) return false;

    parameters.nb_threads = atoi(argv[1]);
    if(parameters.nb_threads < 0) return false;

    parameters.max_scale = atoi(argv[2]);
    if(parameters.max_scale <= 0) return false;

    parameters.main_image_name = string(argv[3]);

    for(unsigned int i=4; i<argc; i++){
        parameters.template_names.push_back(string(argv[i]));
    }
    return true;
}

bool read_parameters(int argc, char* argv[], Parameters& parameters)
{
    if (!_read_parameters(argc, argv, parameters))
    {
        cout<<"Wrong number of parameters or invalid parameters..."<<endl;
        cout<<"The program must be called with the following parameters:"<<endl;
        cout<<"\t- num_threads: The number of threads"<<endl;
        cout<<"\t- max_scale: The maximum scale that can be applied to the templates in the main image"<<endl;
        cout<<"\t- main_image: The main image path"<<endl;
        cout<<"\t- t1 ... tn: The list of the template paths. Each template separated by a space"<<endl;
        cout<<endl<<"For example : ./run 4 3 img.bmp template1.bmp template2.bmp"<<endl;
        return false;
    }
    return true;
}

bool compare_results(Result& first, Result& second)
{
    if(first.pattern_ID < second.pattern_ID) return true;
    else if(first.pattern_ID > second.pattern_ID) return false;

    if(first.position_x < second.position_x) return true;
    else if(first.position_x > second.position_x) return false;

    if(first.position_y < second.position_y) return true;
    else if(first.position_y > second.position_y) return false;

    return true;
}


//int g(bool* str1, bool *str2, int n)
//{
//    int s = 0;
//    for (int i = 3 * n * (n / 2 - 2); i <  3 * n * (n / 2 + 2); ++i)
//        if (str1[i] != str2[i])
//            s++;
//    return s;
//}

//int g(BigPixelStr* str1, PixelStr *str2, int n)
//{
//    int s = 0;
//    for (int i = 0; i < n * n; ++i)
//        if (str1[i] != str2[i])
//            s++;
//    return s;
//}
