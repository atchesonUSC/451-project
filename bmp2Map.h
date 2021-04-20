#include <vector>
#include <string>
#include "bitmap_image.hpp"


class bmpMap {
   public:
      bmpMap(std::string& filename);
      bmpMap(const bmpMap& rhs);
      bmpMap& operator=(const bmpMap& rhs);
      ~bmpMap();
      void printMap();
      unsigned int get_width();
      unsigned int get_height();
      bool checkFree(int x, int y);
      
      bitmap_image img;
      unsigned int width;
      unsigned int height;
      std::vector<std::vector<int> > map;
   private:

};