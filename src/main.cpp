#include <iostream>
#include <string>
#include <list>
#include <fstream>
#include <unordered_map>
#include <math.h>
#include <stdint.h>
#include <atomic>

#include "BitmapImporter.h"
#include "SomeStuff.h"
#include <vector>
#include <cilk/cilk.h>
#include <cilk/reducer_list.h>

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

const int HASH_LINE_SIZE = 10;

vector<RotateHashItem> hashList;
unordered_map<uint32_t, pair<int, int>> hashMap;
int maxSize = 1;
int minSize = 1e8;
int ERROR_NORM = 20;

inline PointR rotatePoint(Point const &p, int x, int y, double a_cos, double a_sin)
{
    Point res( -p.x + x, -p.y + y);
    return PointR(a_cos * res.x + a_sin * res.y, -a_sin * res.x + a_cos * res.y);
}

bool g(uint32_t *a, uint32_t *b, int n)
{
    for (int i = n / 2 - 3; i < n / 2 + 3; ++i)
        if (a[i] != b[i])
            return false;
    return true;
}

Image& cropBorder(Image & im)
{
    Image *res = new Image(im);
    PixelStr pix = im.get_pixel(0, 0);
    int k1 = 0;
    int k2 = 0;
    bool f = true;
    while (f)
    {
        for (int i = 0; f && i < im.get_width(); ++i)
            f &= im.get_pixel(k1, i) == pix && im.get_pixel(im.get_height() - 1 - k1, i) == pix;
        k1++;
    }
    k1--;
    f = true;
    while (f)
    {
        for (int i = 0; f && i < im.get_height(); ++i)
            f &= im.get_pixel(i, k2) == pix && im.get_pixel(i, im.get_width() - 1 - k2) == pix;
        k2++;
    }
    k2--;
    int n = im.get_height() - 2 * k1;
    int m = im.get_width() - 2 * k2;
    PixelStr *pixels = new PixelStr[m * n];
    for (int i = k1; i < k1 + n; ++i)
        for (int j = k2; j < k2 + m; ++j)
            pixels[(i - k1) * m + j - k2] = im.get_pixel(i, j);
    res->set_height(n);
    res->set_width(m);
    res->set_pixels(pixels);
    return *res;
}

Image& doubleSize(Image & im)
{
    Image *res = new Image(im);
    res->set_height(im.get_height() * 2);
    res->set_width(im.get_width() * 2);
    int n = res->get_height();
    int m = res->get_width();
    PixelStr *pixels = new PixelStr[n * m];
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
        {
            BigPixelStr t = im.get_pixel(i / 2, j / 2);
            pixels[i * m + j].r = t.r;
            pixels[i * m + j].g = t.g;
            pixels[i * m + j].b = t.b;
        }
    res->set_pixels(pixels);
    return *res;
}

void create_hash(string temp_name)
{
    Image template_image = Image::create_image_from_bitmap(temp_name);
    template_image = cropBorder(template_image);
    ofstream out("output.bmp");
//    if (min(template_image.get_width(), template_image.get_height()) < 50)
//        template_image = doubleSize(template_image);
    template_image.write(out);
    int template_id = atoi(temp_name.substr(0, 3).c_str());
    int w = template_image.get_width();
    int h = template_image.get_height();
    int m ((double) min(w, h) * sqrt(2)/ 2);
    int n = HASH_LINE_SIZE;
    double nm = (double) n / m;
    if (m / n + 1 > maxSize)
        maxSize = m/n + 1;
    if (m < minSize)
        minSize = m;
    Point p0(w / 2, h / 2);
    PixelStr *str1 = new PixelStr[n * n];
    BigPixelStr *str2 = new BigPixelStr[n * n];
    size_t *counts = new size_t[n * n];
    uint32_t *g1 = new uint32_t[n];
    uint32_t *g2 = new uint32_t[n];

    int s = 0;
    for (double a = 0; a < 360; a += 0.2)
    {
        double a_cos = cos(M_PI * a / 180);
        double a_sin = sin(M_PI * a/ 180);
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
            g2[i] = getHash(str2 + i * n, n);
        }
        if (!g(g2, g1, n) )
        {
//            cout << a << endl;
            RotateHashItem item;
            g1 = new uint32_t[n];
            for (int i = 0; i < n; ++i)
                g1[i] = g2[i];
            item.angle = a;
            item.pattern_id = template_id;
            item.fullHash = g1;
            hashList.push_back(item);
            bool f = false;
            for (int i = n / 2 - 2; i < n / 2 + 2; ++i)
            {
                if (hashMap.find(g1[i]) == hashMap.end())
                {
                    hashMap[g1[i]] = pair<int, int>(hashList.size() - 1, i);
                    f = true;
                }
            }
        }
    }
    delete []g2;
    delete []str1;
    delete []str2;
    delete []counts;
}

int po (Result a, Result b)
{
    return (a.position_x - b.position_x) * (a.position_x - b.position_x) + (a.position_y - b.position_y) * (a.position_y - b.position_y);
}

int main(int argc, char* argv[]){
    struct timeval start, end;
    long mtime, seconds, useconds;
    gettimeofday(&start, NULL);

    cilk::reducer_list_append<Result> result_list;
    Parameters parameters;
    if(!read_parameters(argc, argv, parameters))
        return -1;

    char buffer[4];
    sprintf(buffer, "%d", parameters.nb_threads);
    __cilkrts_set_param((void*)"nworkers", buffer);
    Image main_image = Image::create_image_from_bitmap(parameters.main_image_name);

    for(string temp_name : parameters.template_names)
    {
        create_hash(temp_name);
    }
    if (minSize < 50)
        main_image = doubleSize(main_image);


    int w = main_image.get_width();
    int h = main_image.get_height();
    int n = HASH_LINE_SIZE;
    BigPixelStr*** war = new BigPixelStr**[parameters.nb_threads];
    BigPixelStr **hash = new BigPixelStr*[parameters.nb_threads];
    for (int i = 0; i < parameters.nb_threads; ++i)
    {
        war[i] = new BigPixelStr*[w];
        for (int j = 0; j < w; ++j)
        {
            war[i][j] = new BigPixelStr[maxSize * parameters.max_scale + 1];
        }
        hash[i] = new BigPixelStr[n];
    }
    std::atomic_bool *g = new std::atomic_bool[w * h];
    for (int i = 0; i < w * h; ++i)
        g[i] = true;

    cilk_for (int l= 0; l < w - n; l +=2)
    {
        int thread_num = __cilkrts_get_worker_number();
        int pp = 0;
        for (int j = 0; j < h; ++j)
        {
            if (!g[j * w + l])
            {
                pp = 0;
                continue;
            }
            for (int k = 1; (l + n * k < w) && k < j + 2 && k <= maxSize * parameters.max_scale; ++k)
            {
                int r = l + n * k;
                for (int i = 0; i < n; ++i)
                    hash[thread_num][i] = BigPixelStr();
                for (int i = 1; i <= n; ++i)
                {
                    if (j < k)
                    {
                        war[thread_num][l + k * i - 1][k] = BigPixelStr();
                        for (int i1 = 0; i1 < k; ++i1)
                            for (int i2 = 0; i2 < k; ++i2)
                                war[thread_num][l + k * i - 1][k] += main_image.get_pixel(j - i1, l + k * i - 1 - i2);
                    } else
                    {
                        for (int ii = 0; ii < k; ++ii)
                        {
                            war[thread_num][l + k * i - 1][k] += main_image.get_pixel(j, l + k * i - 1 - ii);//war[j - ii][l + k*i - 1][1];
                            war[thread_num][l + k * i - 1][k] -= main_image.get_pixel(j - k, l + k * i - 1 - ii);//war[j][l + k*i - ii - 1][1];
                        }
                    }
                    hash[thread_num][i - 1] = war[thread_num][l + k * i - 1][k];
                    hash[thread_num][i - 1] /= (k * k);
                }
                int32_t intHash = getHash(hash[thread_num], n);
                if (hashMap.find(intHash) != hashMap.end())
                {
                    pair<int, int> item = hashMap[intHash];
                    int x = l + k * n / 2;
                    int y = j + n / 2 - item.second;
                    bool flag = analizeAnswer(l, j, item.first, item.second, n, k, main_image);
                    if (flag)
                    {
                        for (int ix = x - k * n / 2; ix < x + k * n / 2; ++ix)
                            for (int iy = y + k * n / 2; iy < y + k * n / 2; ++iy)
                                g[iy * w + ix] = false;
                        int id = hashList[item.first].pattern_id;
                        if (minSize < 50)
                        {
                            result_list->push_back(Result {id, x / 2, y / 2});
//                            cout << x / 2 << "\t" << y / 2 << "\t" << hashList[hashMap[intHash].first].angle ;
                        }else
                        {
                            result_list->push_back(Result {id, x, y});
//                            cout << x << "\t" << y << "\t" << hashList[hashMap[intHash].first].angle ;
                        }
//                        cout << "\t" << k << std::endl;
                    }
                }
                pp++;
            }
        }
    }

    cilk_sync;
    list<Result> result = result_list.get_value();
    result.sort(compare_results);
    Result res2;
    for(Result res : result)
    {
        if (po(res, res2) > 50 * 50)
        cout << res.pattern_ID << "\t" << res.position_x << "\t" << res.position_y  << endl;
        res2 = res;
    }

    gettimeofday(&end, NULL);
    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
//    printf("Elapsed time: %ld milliseconds\n", mtime);

    return 0;
}
