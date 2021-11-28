#include "game.hpp"
#include "assets.hpp"

using namespace blit;

const uint8_t BALL_RADIUS = 2;

uint8_t BLOCK_WIDTH;
uint8_t BLOCK_HEIGHT;
uint8_t lives = 3;
uint16_t score = 0;

const Pen blockColours[6] = {
    Pen(197, 76, 81), // Red
    Pen(203, 120, 76), // Orange
    Pen(181, 137, 56), // Dark yellow
    Pen(166, 182, 52), // Yellow/lime
    Pen(83, 182, 82), // Green
    Pen(82, 83, 202) // Blue
};

// Ball code
struct Ball {
    Vec2 pos;
    Vec2 vel;

    Rect boundingBox() {
        return Rect(pos.x - BALL_RADIUS, pos.y - BALL_RADIUS, BALL_RADIUS * 2, BALL_RADIUS * 2);
    }

    void update() {
        pos += vel;

        // We've hit the sides of the screen
        if (pos.x > screen.bounds.w || pos.x < 0) {
            vel.x *= -1;
        }

        // Y 0-20 is the top bar/scoring area
        if (pos.y < 20) {
            vel.y *= -1;
        }

        if (pos.y > screen.bounds.h) {
            // We've gone off the bottom of the screen
            vel.y *= -1;
        }   
    }

    void render() {
        screen.pen = Pen(255, 255, 255);
        screen.circle(Point(pos.x, pos.y), BALL_RADIUS);
    }

    void onCollision(Rect bounds) {
        // Back the ball out first to prevent any weird behaviour
        vel *= -1;
        while(bounds.intersects(boundingBox())){
            pos += vel;
        }
        vel *= -1;

        if (pos.y > bounds.tr().y || pos.y < bounds.br().y)
        {
            // Ball hit on the top or bottom
            vel.y *= -1;
        }

        if (pos.x > bounds.br().x || pos.x < bounds.bl().x)
        {
            // Ball hit on the right or left
            vel.x *= -1;
        }   
    }
};

// Paddle code
struct Paddle {
    Vec2 pos;
    bool launched = false;

    Paddle() {
        pos = Vec2(150, 200);
    }

    Rect paddleBounds() {
        return Rect(pos.x, 200, 30, 5);
    }

    void update(Ball& ball) {
        if (!launched) {
            ball.pos = Vec2(pos.x + 15, pos.y - 4);

            if(pressed(Button::A))
            {
                launched = true;
                ball.vel = Vec2(0.5, -1);
            }
        }

        if (pressed(Button::DPAD_LEFT) && pos.x > 0) {
            pos.x -= 1;
        }

        if (pressed(Button::DPAD_RIGHT) && pos.x < (screen.bounds.w - 30)) {
            pos.x += 1;
        }

        if (paddleBounds().intersects(ball.boundingBox())) {
            ball.onCollision(paddleBounds());
        }
    }

    void render() {
        screen.pen = Pen(255, 255, 255);
        screen.rectangle(Rect(pos.x, 200, 30, 5));
    }
};

// Blocks code
struct Block {
    Vec2 pos;
    bool isActive = true;
    int rowNum;

    Rect blockBounds() {
        return Rect(pos.x, pos.y, BLOCK_WIDTH, BLOCK_HEIGHT);
    }

    void update(Ball& ball) {
        if (blockBounds().intersects(ball.boundingBox()) && isActive) {
            // The ball is within our bounds
            isActive = false;
            ball.onCollision(blockBounds());
            score += (6 - rowNum);
        }
    }

    void render() {
        if (isActive) {
            screen.pen = blockColours[rowNum];
            screen.rectangle(blockBounds());
        }
    }
};

///////////////////////////////////////////////////////////////////////////
//
// init()
//
// setup your game here
//
Paddle paddle;
Ball ball;
Ball& ballRef = ball;
Block blocks[84];

void init() {
    set_screen_mode(ScreenMode::hires);

    BLOCK_WIDTH = screen.bounds.w / 14;
    BLOCK_HEIGHT = BLOCK_WIDTH / 2;

    uint8_t block_count = 0;

    // Populate blocks
    for (int row = 0; row<6; row++) {
        uint8_t destY = (row * BLOCK_HEIGHT) + 20;
        for (int col = 0; col<14; col++) {
            uint16_t destX = col * BLOCK_WIDTH;
            blocks[block_count].pos = Vec2(destX, destY);
            blocks[block_count].rowNum = row;
            block_count++;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
//
// render(time)
//
// This function is called to perform rendering of the game. time is the 
// amount if milliseconds elapsed since the start of your game
//
void render(uint32_t time) {
    // reset the screen alpha and clipping mask
    screen.alpha = 255;
    screen.mask = nullptr;

    // clear the screen -- screen is a reference to the frame buffer and can be used to draw all things with the 32blit
    screen.pen = Pen(0, 0, 0);
    screen.clear();

    // Render all the things
    paddle.render();
    ball.render();
    for (Block block : blocks) {
        block.render();
    }

    // Render score and lives info
    screen.pen = Pen(255, 255, 255);
    for (int i = 0; i < lives; i++) {
        screen.circle(Point((i * 8) + 10, 8), BALL_RADIUS);
    }

    char scoreChar[4];
    sprintf(scoreChar, "%03d", score);
    screen.text(scoreChar, minimal_font, Point(screen.bounds.w - 30, 5));
}

///////////////////////////////////////////////////////////////////////////
//
// update(time)
//
// This is called to update your game state. time is the 
// amount if milliseconds elapsed since the start of your game
//
void update(uint32_t time) {
    ball.update();
    paddle.update(ballRef);
    for (int i = 0; i<84; i++) {
        blocks[i].update(ballRef);
    }
}