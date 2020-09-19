#include "player.h"
#include <math.h>
#include <vector>

const double pi = 3.1415926535897;
const int gravity = 1;
const int screenWidth = 960;
const int screenHeight = 544;

Player::Player(double x, double y, int width, int height, int velocity)
{
	this->x = x;
	this->y = y;
    this->width = width;
	this->height = height;
    onPlatform = false;
    this->velocity = velocity;
    direction = 0; //down=0; left=1; up=2; right=3
}

double Player::getX()
{
	return x;
}

double Player::getY()
{
	return y;
}

void Player::setX(int x)
{
    this->x = x;
}

void Player::setY(int y)
{
    this->y = y;
}

int Player::getWidth()
{
    return width;
}

int Player::getHeight()
{
    return height;
}

bool Player::isOnGround()
{
    return onPlatform;
}
bool Player::isOnPlatform()
{
    return onPlatform;
}

void Player::setOnPlatform(bool result)
{
    onPlatform = result;
}

void Player::setVelocity(int velocity)
{
    this->velocity = velocity;
}

int Player::getVelocity(){
    return velocity;
}

int Player::getDirection(){
    return direction;
}

void Player::setDirection(int direction){
    this->direction = direction;
}
