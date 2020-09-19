#include "platform.h"
#include <math.h>
const int screenWidth = 960;
const int screenHeight = 544;
Platform::Platform(int x, int y)
{
    width = 64;
    height = 64;
    this->x = x;
    this->y = y;
}
Platform::Platform(){
    width = 64;
    height = 64;
}
double Platform::getX()
{
    return x;
}

double Platform::getY()
{
    return y;
}

int Platform::getWidth()
{
    return width;
}

int Platform::getHeight()
{
    return height;
}

void Platform::setX(int x){
    this->x = x;
}
void Platform::setY(int y){
    this->y = y;
}