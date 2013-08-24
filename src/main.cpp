#include <iostream>
#include <string>
#include <list>
#include <fstream>
#include <unordered_map>
#include <math.h>
#include <stdint.h>

#include "BitmapImporter.h"
#include "SomeStuff.h"
#include <vector>

#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

const int HASH_LINE_SIZE = 10;

struct RotateHashItem
{
    int pattern_id;
    double angle;
    uint32_t *fullHash;
};

vector<RotateHashItem> hashList;
unordered_map<uint32_t, pair<int, int>> hashMap;


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

struct BigPixelStr
{
    int r;
    int g;
    int b;
};

inline PointR rotatePoint(Point const &p, int x, int y, double a_cos, double a_sin)
{
    Point res( -p.x + x, -p.y + y);
    return PointR(a_cos * res.x - a_sin * res.y, a_sin * res.x + a_cos * res.y);
}

bool operator != (BigPixelStr const & a, PixelStr const & b)
{
    return a.r != b.r || a.g != b.g || a.b != b.b;
}
int g(BigPixelStr* str1, PixelStr *str2, int n)
{
    int s = 0;
    for (int i = 0; i < n * n; ++i)
        if (str1[i] != str2[i])
            s++;
    return s;
}

int g(bool* str1, bool *str2, int n)
{
    int s = 0;
    for (int i = 3 * n * (n / 2 - 2); i <  3 * n * (n / 2 + 2); ++i)
        if (str1[i] != str2[i])
            s++;
    return s;
}

bool g(uint32_t *a, uint32_t *b, int n)
{
    for (int i = n / 2 - 2; i < n / 2 + 2; ++i)
        if (a[i] != b[i])
            return false;
    return true;
}


void create_hash(string temp_name)
{
    Image template_image = Image::create_image_from_bitmap(temp_name);
    int template_id = atoi(temp_name.substr(0, 3).c_str());
    int w = template_image.get_width();
    int h = template_image.get_height();
    int m ((double) min(w, h) * sqrt(2)/ 2);
    int n = HASH_LINE_SIZE;
    cout << m << endl;
    double nm = (double) n / m;
    Point p0(w / 2, h / 2);
    PixelStr *str1 = new PixelStr[n * n];
    BigPixelStr *str2 = new BigPixelStr[n * n];
    size_t *counts = new size_t[n * n];
    uint32_t *g1 = new uint32_t[n];
    uint32_t *g2 = new uint32_t[n];

    int s = 0;
    for (double a = 0; a < 360; a += 0.2)
    {
        double a_cos = cos(a * M_PI / 180);
        double a_sin = sin(a * M_PI / 180);
        for (int i = 0; i < n * n; ++i)
        {
            counts[i] = 0;
            str2[i].r = 0;
            str2[i].g = 0;
            str2[i].b = 0;
        }
        for (int i = 0; i < h; ++i)
            for (int j = 0; j < w; ++j)
            {
                if ((p0.x - j) * (p0.x - j) + (p0.y - i) * (p0.y - i) > m * m)
                    continue;
                PointR t = rotatePoint(p0, j, i, a_cos, a_sin);
                t.x += m / 2;
                t.y += m / 2;
                if (t.x < 0 || t.x >=m || t.y < 0 || t.y >= m)
                    continue;
                int tx = t.x * nm;
                int ty = t.y * nm;
                int k = ty * n + tx;
                PixelStr const &pix = template_image.get_pixel(i, j);
                str2[k].r += pix.r;
                str2[k].g += pix.g;
                str2[k].b += pix.b;
                counts[k]++;
            }
        for (int i = 0; i < n * n; ++i)
                if (counts[i] != 0)
                {
                    str2[i].r /= counts [i];
                    str2[i].g /= counts [i];
                    str2[i].b /= counts [i];
                }
        for (int i = 0; i < n; ++i)
        {
            double r(0), g(0), b(0);
            for (int j = 0; j < n; ++j)
            {
                r += str2[i * n + j].r;
                g += str2[i * n + j].g;
                b += str2[i * n + j].b;
            }
            r /= n;
            g /= n;
            b /= n;
            g2[i] = 0;
            for (int j = 0; j < n; ++j)
            {
                g2[i] |= (str2[i * n + j].r > r) ? 1 << (j * 3 ) : 0;
                g2[i] |= (str2[i * n + j].g > g) ? 1 << (j * 3 + 1) : 0;
                g2[i] |= (str2[i * n + j].b > b) ? 1 << (j * 3 + 2) : 0;
            }
        }
        if (!g(g2, g1, n) )
        {
            cout << a << endl;
            RotateHashItem item;
            g1 = new uint32_t[n];
            for (int i = 0; i < n; ++i)
                g1[i] = g2[i];
            item.angle = a;
            item.pattern_id = template_id;
            item.fullHash = g1;
            hashList.push_back(item);
            bool f = false;
            for (int i = n / 2 - 3; i < n / 2 + 3; ++i)
            {
                if (hashMap.find(g1[i]) == hashMap.end())
                {
                    hashMap[g1[i]] = pair<int, int>(hashList.size() - 1, i);
                    f = true;
                }
            }
            if (!f)
            {
                cout << "--- " << a << endl;
                s++;
            }
//            for (int i = 0; i < n * n; ++i)
//            {
//                str1[i].r = str2[i].r;
//                str1[i].g = str2[i].g;
//                str1[i].b = str2[i].b;
//            }
//            char name[20];
//            snprintf(name, 20, "output%d.bmp", (int)a);
//            ofstream out(name);
//            Image imout = Image::create_image_from_bitmap(temp_name);
//            imout.set_height(n);
//            imout.set_width(n);
//            PixelStr *p = imout.get_pixels();
//            imout.set_pixels(str1);
//            imout.write(out);
//            imout.set_pixels(p);
//            out.close();
        }
    }
    cout << s << endl;
    delete []g2;
    delete []str1;
    delete []str2;
    delete []counts;
}

int main(int argc, char* argv[]){

	Parameters parameters;
    if(!read_parameters(argc, argv, parameters))
        return -1;

    list<Result> result_list;
    Image main_image = Image::create_image_from_bitmap(parameters.main_image_name);

    struct timeval start, end;

    long mtime, seconds, useconds;

    gettimeofday(&start, NULL);

    for(string temp_name : parameters.template_names)
    {
        create_hash(temp_name);
    }

    gettimeofday(&end, NULL);

    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;

    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

    printf("Elapsed time: %ld milliseconds\n", mtime);


    int w = main_image.get_width();
    int h = main_image.get_height();
    BigPixelStr ***war;

    result_list.sort(compare_results);

    for(Result res : result_list)
    {
        cout << res.pattern_ID << "\t" << res.position_x << "\t" << res.position_y << endl;
    }
	return 0;
}
