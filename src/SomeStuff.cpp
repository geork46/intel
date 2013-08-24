#include "SomeStuff.h"

#include <iostream>

using namespace std;

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
