#ifndef PLATFORM_H
#define PLATFORM_H
class Platform
{
  private:
  double x;
  double y;
  int width;
  int height;
	
  public:
	Platform(int x, int y);
  Platform();
	double getX();
	double getY();
  int getWidth();
  int getHeight();
  void setX(int x);
  void setY(int y);
};
#endif