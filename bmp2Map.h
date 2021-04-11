#include <vector>
#include <string>
#include "bitmap_image.hpp"

class bmpMap
{
private:
   bitmap_image img;
   std::vector<std::vector<int>> map;
   unsigned int height;
   unsigned int width;
public:
   bmpMap(std::string& filename);
   ~bmpMap();
   void printMap();
   bool checkFree(int x, int y);
};