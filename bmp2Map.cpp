#include "bmp2Map.h"

bmpMap::bmpMap(bitmap_image img)
: img(img),
  height(img.height()),
  width(img.width())
{
   //Create map
   for (std::size_t y = 0; y < height; ++y)
   {
      std::vector<int> newRow;
      map.push_back(newRow);
      for (std::size_t x = 0; x < width; ++x)
      {
         rgb_t color;
         img.get_pixel(x, y, color);
         //if the color is darker than medium grey then its a zero
         if((color.red < 128)&&(color.green < 128)&&(color.blue < 128))
            map[y].push_back(0);
         else
            map[y].push_back(1);
      }
   }
}

bmpMap::~bmpMap()
{
}

void bmpMap::printMap()
{
   for (std::size_t y = 0; y < height; ++y)
   {
      for (std::size_t x = 0; x < width; ++x)
      {
         printf("%d ", map[y][x]);
      }
      printf("\n");
   }
}

bool bmpMap::checkFree(int x, int y)
{
   return map[y][x] == 1;
}