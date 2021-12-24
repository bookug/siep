#include"FileUtility.h"


std::ifstream::pos_type FileUtility::filesize(const char* filename){
    std::ifstream in(filename, std::ifstream::in | std::ifstream::binary);
    in.seekg(0, std::ifstream::end);
    return in.tellg(); 
}