#ifndef PLAYER_H
#define PLAYER_H
#include <vector>
class Player
{
    private:
    double x;
    double y;
    int width;
    int height;
    bool onPlatform;
    int velocity;
    int direction;
	
  public:
	Player(double x, double y, int width, int height, int velocity);
    double getX();
    double getY();
    void setX(int x);
    void setY(int y);
    int getWidth();
    int getHeight();
    bool isOnGround();
    bool isOnPlatform();
    void setOnPlatform(bool result);
    int getVelocity();
    void setVelocity(int velocity);
    int getDirection();
    void setDirection(int direction);
};
#endif