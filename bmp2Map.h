#include <vector>
#include <string>


class bmpMap {
   public:
      bmpMap(std::string& filename);
      bmpMap(const bmpMap& rhs);
      bmpMap& operator=(const bmpMap& rhs);
      ~bmpMap();
      void printMap();
      unsigned int get_width();
      unsigned int get_height();
      bool checkFree(std::pair<int, int> q);
      
      unsigned int width;
      unsigned int height;
      std::vector<std::string> bitmap;
};
