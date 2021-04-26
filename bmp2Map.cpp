#include <fstream>
#include <iostream>

#include "bmp2Map.h"


bmpMap::bmpMap(std::string& filename)
{
    // initialize dimensions
    width = 0;
    height = 0;

    // create file stream
    std::ifstream ifile (filename);

    // read rows of bitmap
    std::string line;
    
    while (ifile >> line) {
        // update height
        height++;

        // add row of bitmap
        bitmap.push_back(line);
    }

    // update width
    width = bitmap[0].length();
}

bmpMap::bmpMap(const bmpMap& rhs)
:width(rhs.width)
,height(rhs.height)
{
    //Create map
    for (std::size_t y = 0; y < height; ++y) {
        bitmap.push_back(rhs.bitmap[y]);
    }
}

// Assignment operator
bmpMap& bmpMap::operator=(const bmpMap& rhs)
{
    // 1. First check that we're not self-assigning
    if (&rhs != this)
    {
        this->width = rhs.width;
        this->height = rhs.height;
        //Create map
        for (int y = 0; y < height; ++y) {
            this->bitmap.push_back(rhs.bitmap[y]);
        }
    }
    return *this;
}

bmpMap::~bmpMap() {}

void bmpMap::printMap() {
    for (std::size_t y = 0; y < height; ++y) {
        std::cout << bitmap[y] << std::endl;
    }
}

bool bmpMap::checkFree(std::pair<int, int> q) {
    int x, y;

    x = q.first;
    y = q.second;
 
    //printf("bitmap[%d][%d]", y, x);

    return bitmap[y][x] == '0';
}

unsigned int bmpMap::get_height() {
    return height;
}

unsigned int bmpMap::get_width() {
    return width;
}
