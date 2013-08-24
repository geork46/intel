#ifndef SOMESTUFF_H
#define SOMESTUFF_H

#include <string>
#include <list>


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

bool read_parameters(int argc, char* argv[], Parameters& parameters);

bool compare_results(Result& first, Result& second);

#endif //SOMESTUFF_H
