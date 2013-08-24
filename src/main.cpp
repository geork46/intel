#include <iostream>
#include "BitmapImporter.h"
#include <omp.h>
#include <string>
#include <list>
#include <math.h>
#include <fstream>

using namespace std;

struct Parameters
{
	int nb_threads;
	string main_image_name;
	list<std::string> template_names;
	int max_scale;
};

struct Point
{
    double x, y;
    Point(double x, double y) : x(x), y(y) {}
};

struct BigPixelStr
{
    int r;
    int g;
    int b;
};

Point f(Point p, double x, double y, double a)
{
    Point res( -p.x + x, -p.y + y);
    a = a * M_PI / 180;
    x = res.x * cos(a) - res.y * sin(a);
    y = res.x * sin(a) + res.y * cos(a);
    return Point(x, y);
}

bool read_parameters(int argc, char* argv[], Parameters& parameters){
	if(argc < 4) return false;

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

bool operator != (BigPixelStr const & a, Accelerate::PixelStr const & b)
{
    return a.r != b.r || a.g != b.g || a.b != b.b;
}
int g(BigPixelStr* str1, Accelerate::PixelStr *str2, int n)
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
    for (int i = 3 * 5 * n; i <  3 * 7 * n; ++i)
        if (str1[i] != str2[i])
            s++;
    return s;
}

int main(int argc, char* argv[]){
	if(argc<=1) return 0;
	if(argv == NULL) return 0;
	Parameters parameters;
	if(!read_parameters(argc, argv, parameters)){
		cout<<"Wrong number of parameters or invalid parameters..."<<endl;
		cout<<"The program must be called with the following parameters:"<<endl;
		cout<<"\t- num_threads: The number of threads"<<endl;
		cout<<"\t- max_scale: The maximum scale that can be applied to the templates in the main image"<<endl;
		cout<<"\t- main_image: The main image path"<<endl;
		cout<<"\t- t1 ... tn: The list of the template paths. Each template separated by a space"<<endl;
		cout<<endl<<"For example : ./run 4 3 img.bmp template1.bmp template2.bmp"<<endl;
		return -1;
	}
	Accelerate::Image main_image = Accelerate::Image::create_image_from_bitmap(parameters.main_image_name);
	for(string temp_name : parameters.template_names){ 
        Accelerate::Image template_image = Accelerate::Image::create_image_from_bitmap(temp_name);

        int template_id = atoi(temp_name.substr(0, 3).c_str());
        int w = template_image.get_width();
        int h= template_image.get_height();
        int m ((double) min(w, h) * sqrt(2)/ 2);
        int n = 12;
        int s = 0;
        cout << m << endl;
        Point p0(w / 2, h / 2);
        double b = 0;
        Accelerate::PixelStr *str1 = new Accelerate::PixelStr[n * n];
        bool *g1 = new bool[3 * n * n];
        for (double a = 0; a < 10; a += 0.01)
        {
            BigPixelStr *str2 = new BigPixelStr[n * n];
            bool *g2 = new bool[3 * n * n];
            int *counts = new int[n * n];
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
                    Point t = f(p0, j, i, a);
                    double dx = (t.x + m / 2);
                    double dy = (t.y + m / 2);
                    if (dx < 0 || dx >=m || dy < 0 || dy >= m)
                        continue;
                    int tx = (double)dx * n / m;
                    int ty = (double)dy * n / m;
                    str2[ty * n + tx].r += template_image.get_pixel(i, j).r;
                    str2[ty * n + tx].g += template_image.get_pixel(i, j).g;
                    str2[ty * n + tx].b += template_image.get_pixel(i, j).b;
                    counts[ty * n + tx]++;
                }
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    if (counts[i * n + j] != 0)
                    {
                        str2[i * n + j].r /= counts [i * n + j];
                        str2[i * n + j].g /= counts [i * n + j];
                        str2[i * n + j].b /= counts [i * n + j];
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
                for (int j = 0; j < n; ++j)
                {
                    g2[(i * n + j) * 3] = (str2[i * n + j].r > r);
                    g2[(i * n + j) * 3 + 1] = (str2[i * n + j].g > g);
                    g2[(i * n + j) * 3 + 2] = (str2[i * n + j].b > b);
                }
            }
            if (g(g2, g1, n) )
            {
                s++;
                cout << a << " " << g(str2, str1, n) << endl;
                b = a;
                for (int i = 0; i < n * n; ++i)
                {
                    str1[i].r = str2[i].r;
                    str1[i].g = str2[i].g;
                    str1[i].b = str2[i].b;
                    g1[3 * i] = g2[3 * i];
                    g1[3 * i + 1] = g2[3 * i + 1];
                    g1[3 * i + 2] = g2[3 * i + 2];
                }
                char name[20];
                snprintf(name, 20, "output%d.bmp", (int)a);
                ofstream out(name);
                Accelerate::Image imout = Accelerate::Image::create_image_from_bitmap(temp_name);
                imout.set_height(n);
                imout.set_width(n);
                Accelerate::PixelStr *p = imout.get_pixels();
                imout.set_pixels(str1);
                imout.write(out);
                imout.set_pixels(p);
                out.close();
            }
            delete []str2;
            delete []counts;
        }
        cout << s << endl;
    }

	return 0;
}
