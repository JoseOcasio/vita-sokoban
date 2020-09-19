#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/audioout.h>

#include <vita2d.h>

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <fstream>
#include "player.h"
#include "platform.h"

#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_speech.h"

#define click() (touch.reportNum == 1 & !(oldtouch.reportNum == 1))
#define release() (oldtouch.reportNum == 1 && !(touch.reportNum == 1))
#define hold() (touch.reportNum == 1)
#define start() (ctrl.buttons & SCE_CTRL_CROSS)

const double pi = 3.1415926535897;
const int gravity = 1;

const int screenWidth = 960;
const int screenHeight = 544;

Platform platforms[200];
Platform movableObjects[1] = {Platform(screenWidth/2, screenHeight/2)};
Platform endpoints[1] = {Platform(screenWidth - 32*6, screenHeight - 32*6)};
Player player(100, 100, 37, 59, 3);

bool titleScreen = true;
bool playCoinFX = false;

bool endGame = false;

void log(const std::string& log)
{
     std::ofstream save;
     save.open("ux0:data/sokoban.txt");
     save << log;
     save.close();
}

void resetGame(){
    int endpointsCount = sizeof(endpoints)/sizeof(endpoints[0]);
    int movableObjectsCount = sizeof(movableObjects)/sizeof(movableObjects[0]);

    for (int i=0; i<movableObjectsCount; i++){
        movableObjects[i] = {Platform(screenWidth/2, screenHeight/2)};
    }

    for (int i=0; i<endpointsCount; i++){
        endpoints[i] = {Platform(screenWidth - 32*6, screenHeight - 32*6)};
    }
    // player = Player(100, 100, 37, 59, 3);
}

// TODO: BUG: Overlaps with collision
void checkPlayerCollision() {
    bool collision = false;
    int platformsCount = sizeof(platforms)/sizeof(platforms[0]);
    for (int i = 0; i < platformsCount; i++){
        if (player.getX() < platforms[i].getX() + platforms[i].getWidth() && 
            player.getX() + player.getWidth() > platforms[i].getX() && 
            player.getY() + player.getHeight() >= platforms[i].getY() && 
            player.getY() < platforms[i].getY() + platforms[i].getHeight()){
            
            collision = true;
        }
    }
    player.setOnPlatform(collision);
}

int checkPlayerCollisionWithMovableObjects() {
    int crateIndex = 100;
    int movableObjectsCount = sizeof(movableObjects)/sizeof(movableObjects[0]);
    for (int i = 0; i < movableObjectsCount; i++){
        if (player.getX() < movableObjects[i].getX() + movableObjects[i].getWidth() && 
            player.getX() + player.getWidth() > movableObjects[i].getX() && 
            player.getY() + player.getHeight() >= movableObjects[i].getY() && 
            player.getY() < movableObjects[i].getY() + movableObjects[i].getHeight()){
            
            crateIndex = i;
        }
    }
    return crateIndex;
}

bool checkEndpointCollision(){
    bool collision = false;
    int endpointsCount = sizeof(endpoints)/sizeof(endpoints[0]);
    for (int i = 0; i < endpointsCount; i++){
        if (movableObjects[i].getX() < endpoints[i].getX() + endpoints[i].getWidth() && 
            movableObjects[i].getX() + movableObjects[i].getWidth() > endpoints[i].getX() && 
            movableObjects[i].getY() + movableObjects[i].getHeight() >= endpoints[i].getY() && 
            movableObjects[i].getY() < endpoints[i].getY() + endpoints[i].getHeight()){

            collision = true;
        }
    }
    return collision;
}

void createObjects(vita2d_texture *crateBeige, vita2d_texture *crateRed, vita2d_texture *endpointRed){
    //Draw edges
    int counter = 0; // counter to keep track of position in the objects array
    //Fill boxes horiztonal; y=0
    for (int i = 0; i<screenHeight/64; i++){
        vita2d_draw_texture(crateBeige, 0, i*64);
        platforms[i] = Platform(0, i*64);
        counter += 1;
    }

    //Fill boxes horizontal; y=max-64
    for (int i = 0; i<screenWidth/64; i++){
        vita2d_draw_texture(crateBeige, i*64, screenHeight - 64);
        platforms[i+counter] = Platform(i*64, screenHeight - 64);
        counter += 1;
    }

    //Fill boxes vertical; x=max-64
    for (int i = 0; i<screenHeight/64; i++){
        vita2d_draw_texture(crateBeige, screenWidth - 64, i*64);
        platforms[i+counter] = Platform(screenWidth - 64, i*64);
        counter += 1;
    }

    //Fill boxes vertical; x=0
    for (int i = 0; i<screenWidth/64; i++){
        vita2d_draw_texture(crateBeige, i*64, 0);
        platforms[i+counter] = Platform(i*64, 0);
        counter += 1;
    }

    //draw movable objects
    int movableObjectsCount = sizeof(movableObjects)/sizeof(movableObjects[0]);
    for (int i = 0; i<movableObjectsCount; i++){
        vita2d_draw_texture(crateRed, movableObjects[i].getX(), movableObjects[i].getY());
        movableObjects[i] = Platform(movableObjects[i].getX(), movableObjects[i].getY());
    }

    //create endpoints
    int endpointsCount = sizeof(endpoints)/sizeof(endpoints[0]);
    for (int i = 0; i<endpointsCount; i++){
        vita2d_draw_texture(endpointRed, endpoints[i].getX(),  endpoints[i].getY());
        endpoints[i] = Platform(endpoints[i].getX(), endpoints[i].getY());
    }
}

vita2d_texture * animationState(bool animateFlag, vita2d_texture *animation1, vita2d_texture *animation2){
    if (animateFlag){
        return animation1;
    }
    else {
        return animation2;
    }
}

int main(void)
{
    srand (time(NULL));
    
    int mouseDownX;
    int mouseDownY;
    double timer = 0; 
    double animationTimer = 0;
    double splashTimer = 0;
    bool playedSplash = false;
    bool playedSelect = false;
    bool firstTime = true;
    bool animationToggle = false;
        
    vita2d_texture *playerSprite;
    vita2d_texture *platformSprite;

    //objects
    vita2d_texture *crateBeige;
    vita2d_texture *crateRed;
    vita2d_texture *endpointRed;

    //character
    vita2d_texture *characterLeft;
    vita2d_texture *characterRight;
    vita2d_texture *characterRightWalking;
    vita2d_texture *characterFront;
    vita2d_texture *characterFrontWalking;
    vita2d_texture *characterFrontWalking2;
    vita2d_texture *characterBack;
    vita2d_texture *characterBackWalking;
    vita2d_texture *characterBackWalking2;
    vita2d_texture *characterLeftWalking;
    vita2d_texture *playerDirection;

    //font
    vita2d_font *font;
    vita2d_font *bigFont;
    
    vita2d_init();
    vita2d_set_clear_color(RGBA8(238, 228, 225, 255));
    
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
    sceTouchEnableTouchForce(SCE_TOUCH_PORT_FRONT);

    /* to enable analog sampling */
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    SceCtrlData ctrl;
    
    playerSprite = vita2d_load_PNG_file("app0:resources/egg.png");
    platformSprite = vita2d_load_PNG_file("app0:resources/platform.png");

    //
    crateBeige = vita2d_load_PNG_file("app0:resources/sokoban/Crate_Beige.png");
    crateRed = vita2d_load_PNG_file("app0:resources/sokoban/Crate_Red.png");
    endpointRed = vita2d_load_PNG_file("app0:resources/sokoban/EndPoint_Red.png");

    characterLeft = vita2d_load_PNG_file("app0:resources/sokoban/Character1.png");
    characterRight = vita2d_load_PNG_file("app0:resources/sokoban/Character2.png");
    characterRightWalking = vita2d_load_PNG_file("app0:resources/sokoban/Character3.png");
    characterFront = vita2d_load_PNG_file("app0:resources/sokoban/Character4.png");
    characterFrontWalking = vita2d_load_PNG_file("app0:resources/sokoban/Character5.png");
    characterFrontWalking2 = vita2d_load_PNG_file("app0:resources/sokoban/Character6.png");
    characterBack = vita2d_load_PNG_file("app0:resources/sokoban/Character7.png");
    characterBackWalking = vita2d_load_PNG_file("app0:resources/sokoban/Character8.png");
    characterBackWalking2 = vita2d_load_PNG_file("app0:resources/sokoban/Character9.png");
    characterLeftWalking = vita2d_load_PNG_file("app0:resources/sokoban/Character10.png");

    //init player direction to front
    playerDirection = vita2d_load_PNG_file("app0:resources/sokoban/Character4.png");
    
    vita2d_set_vblank_wait(1);
    
    SceTouchData touch, oldtouch;
	sceTouchPeek(0, &oldtouch, 1);
    
    font = vita2d_load_font_file("app0:resources/font.otf");
    bigFont = vita2d_load_font_file("app0:resources/font.otf");
       
    while (1)   
    {
        //capture controls
        sceCtrlPeekBufferPositive(0, &ctrl, 1);

        //capture peek
        sceTouchPeek(0, &touch, 1);
        if (titleScreen)
        {   if (splashTimer > 120)
            {
                if (!playedSelect)
                {
                    playedSelect = true;
                }
                vita2d_start_drawing();		
                vita2d_clear_screen();
                if (endGame) {
                    resetGame();
                    vita2d_font_draw_text(font, screenWidth/2 - 50, screenHeight/2 + 3 + 26, RGBA8(213, 128, 90, 255), 32, "YOU WIN!");
                }
                vita2d_font_draw_text(font, screenWidth/2 - 146, screenHeight/2 + 50 + 32, RGBA8(178, 150, 125, 102), 32, "CLICK CROSS(X) TO BEGIN");
                vita2d_end_drawing();
                vita2d_swap_buffers();

                if (start())
                {
                    titleScreen = false;
                }

            } else {
                vita2d_start_drawing();		
                vita2d_clear_screen();
                vita2d_font_draw_text(font, screenWidth/2 - 50, screenHeight/2 + 3 + 26, RGBA8(213, 128, 90, 255), 32, "SAMURAY");
                vita2d_end_drawing();
                vita2d_swap_buffers();
                splashTimer += 1;
            }
        } 
        else
        {            
            checkPlayerCollision();
            int crateToBeMoved = checkPlayerCollisionWithMovableObjects();

            timer += 0.05;
            animationTimer += 0.05;
            vita2d_start_drawing();		
            vita2d_clear_screen();

            createObjects(crateBeige, crateRed, endpointRed);

            //toggle between walking animations
            if (animationTimer <= 0.5){
                animationToggle = false;
            }
            else if(animationTimer <= 1){
                animationToggle = true;
            }
            else {
                animationTimer = 0;
            }
        
            if (ctrl.buttons & SCE_CTRL_UP){
                if (!player.isOnPlatform()){
                    player.setY(player.getY() - player.getVelocity());
                    playerDirection = animationState(animationToggle, characterBackWalking, characterBackWalking2);
                    player.setDirection(2);
                }
                else {
                    player.setY(player.getY() + player.getVelocity()*2);
                }  
            }
            
            if (ctrl.buttons & SCE_CTRL_DOWN){
                if (!player.isOnPlatform()){
                    player.setY(player.getY() + player.getVelocity());
                    playerDirection = animationState(animationToggle, characterFrontWalking, characterFrontWalking2);
                    player.setDirection(0);
                }
                else {
                    player.setY(player.getY() - player.getVelocity()*2);
                }
            }
            if (ctrl.buttons & SCE_CTRL_LEFT){
                if (!player.isOnPlatform()){
                    player.setX(player.getX() - player.getVelocity());
                    playerDirection = animationState(animationToggle, characterLeft, characterLeftWalking);
                    player.setDirection(1);
                }
                else {
                    player.setX(player.getX() + player.getVelocity()*2);
                }
            }
            if (ctrl.buttons & SCE_CTRL_RIGHT){
                if (!player.isOnPlatform()){
                    player.setX(player.getX() + player.getVelocity());
                    playerDirection = animationState(animationToggle, characterRight, characterRightWalking);
                    player.setDirection(3);
                }
                else {
                    player.setX(player.getX() - player.getVelocity()*2);
                }
            }
            
            if (crateToBeMoved != 100){
                if (player.getDirection() == 0){ //down
                    movableObjects[crateToBeMoved].setX(movableObjects[crateToBeMoved].getX());
                    movableObjects[crateToBeMoved].setY( movableObjects[crateToBeMoved].getY() + 64);
                }
                else if (player.getDirection() == 1){ //left
                    movableObjects[crateToBeMoved].setX(movableObjects[crateToBeMoved].getX() - 64);
                    movableObjects[crateToBeMoved].setY( movableObjects[crateToBeMoved].getY());
                }
                else if (player.getDirection() == 2){ //up
                    movableObjects[crateToBeMoved].setX(movableObjects[crateToBeMoved].getX());
                    movableObjects[crateToBeMoved].setY(movableObjects[crateToBeMoved].getY() - 64);
                }
                else if (player.getDirection() == 3){ //right
                    movableObjects[crateToBeMoved].setX(movableObjects[crateToBeMoved].getX() + 64);
                    movableObjects[crateToBeMoved].setY(movableObjects[crateToBeMoved].getY());
                    
                }
                
                //check endpoints collision
                if (checkEndpointCollision()){
                    endGame = true;
                    titleScreen = true;

                }
            }

            vita2d_draw_texture(playerDirection, player.getX(), player.getY()); 

            vita2d_end_drawing();
            vita2d_swap_buffers();
        }  
    }
    
    vita2d_fini();
    vita2d_free_texture(crateBeige);
    vita2d_free_texture(crateRed);
    vita2d_free_texture(endpointRed);
    vita2d_free_texture(characterLeft);
    vita2d_free_texture(characterRight);
    vita2d_free_texture(characterRightWalking);
    vita2d_free_texture(characterFront);
    vita2d_free_texture(characterFrontWalking);
    vita2d_free_texture(characterFrontWalking2);
    vita2d_free_texture(characterBack);
    vita2d_free_texture(characterBackWalking);
    vita2d_free_texture(characterBackWalking2);
    vita2d_free_texture(characterLeftWalking);

    vita2d_free_texture(platformSprite);
    vita2d_free_font(font);
    vita2d_free_font(bigFont);
          
    sceKernelExitProcess(0);
    return 0;
}