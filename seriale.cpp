#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <vector>
using namespace std;

#define WIDTH 1280
#define HEIGHT 720
bool useAllegro = false;
bool done = false;
bool showMenu = true;
ALLEGRO_FONT* displayFont;
ALLEGRO_DISPLAY* display;
struct Configuration
{
    int id;
    float cellSize;
    int dim;
    int deadRate;
    int firstAliveRate;
    int secondAliveRate;
    int numberOfTiles;
    int deadCount;
    int firstAliveCount;
    int secondAliveCount;
    int maxIterations;
    Configuration()
    {

    }
    Configuration(const int &id,const float &cellSize,const int &dim,const int &deadRate,const int &firstAliveRate,const int &secondAliveRate,const int &maxIterations)
    {
        this->id = id;
        this->cellSize = cellSize;
        this->dim = dim;
        this->deadRate = deadRate;
        this->firstAliveRate = firstAliveRate;
        this->secondAliveRate = secondAliveRate;
        this->numberOfTiles = dim * dim;
        this->deadCount = (this->deadRate * this->numberOfTiles) / 100;
        this->firstAliveCount = (this->firstAliveRate * this->numberOfTiles) / 100;
        this->secondAliveCount = (this->secondAliveRate * this->numberOfTiles) / 100;
        this->maxIterations = maxIterations;
    }
};
int getValue(const vector<vector<int>> &matrix,const int &x,const int &y,const Configuration &current)
{
    int firstAlive = 0;
    int secondAlive = 0;
    for(int i = x - 1;i <= x + 1;i++)
    {
        for(int j = y - 1;j <= y + 1;j++)
        {
            if(i >= 0 && i < current.dim && j >= 0 && j < current.dim)
            {
                if(matrix[i][j] == 1)
                   firstAlive++;
                if(matrix[i][j] == 2)
                   secondAlive++;
            }
        }
    }
    int countAlive = firstAlive + secondAlive;
    int value = matrix[x][y];
    if(value == 0)
    {
        if(countAlive == 3)
           return firstAlive > secondAlive ? 1 : 2;
        return 0;
    }
    else
    {
        countAlive--;
        if(countAlive >= 4 || countAlive < 2)
           return 0;
        return value;
    }
}
void initAllegro()
{
    if(!al_init())
    {
        al_show_native_message_box(NULL,"Allegro Error","Header Error","Could not init allegro","",0);
        return;
    }
    al_set_new_window_title("Game of Life");
    al_set_app_name("Game of Life");
    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    display = al_create_display(WIDTH,HEIGHT);
    if(!display)
    {
        al_show_native_message_box(NULL,"Allegro Error","Header Error","Could not create display correcly","",0);
        return;
    }
    al_install_keyboard();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    displayFont = al_load_font("RobotoCondensed-Bold.ttf",24,0);
}
void drawText(const int &currentDead,const int &currentIteration,const int &maxIterations,const int &currentFirstAliveCount,const int &currentSecondAliveCount,const int &time,const int &maxTime,const Configuration &current)
{
    ALLEGRO_COLOR textColor = al_map_rgb(255,255,255);
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,10,0,"Number of tiles: %d",current.numberOfTiles);
    int currentDistance = 10;
    if(current.deadRate > 0 && current.firstAliveRate > 0 && current.secondAliveRate > 0)
    {
        al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,40,0,"Starting count of dead tiles: %d",current.deadCount);
        al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,70,0,"Starting count of alive tiles with value 1: %d",current.firstAliveCount);
        al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,100,0,"Starting count of alive tiles with value 2: %d",current.secondAliveCount);
        currentDistance += 90;
    }
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,currentDistance + 30,0,"Current number of dead tiles: %d",currentDead);
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,currentDistance + 60,0,"Curret number of alive tiles with value 1: %d",currentFirstAliveCount);
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,currentDistance + 90,0,"Current number of alive tiles with value 2: %d",currentSecondAliveCount);
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,currentDistance + 120,0,"Current Iteration: %d",currentIteration);
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,currentDistance + 150,0,"Max Iterations: %d",maxIterations);
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,currentDistance + 180,0,"Current Configuration: %d",current.id);
    al_draw_text(displayFont,textColor,(WIDTH / 2) + 100,currentDistance + 240,0,"Press ESC to exit the application");
    al_draw_text(displayFont,textColor,(WIDTH / 2) + 100,currentDistance + 270,0,"Press 1 to toggle the visibility of the menu");
}
void drawMatrix(const vector<vector<int>> &matrix,const Configuration &current,const int &time,const int &maxTime)
{
    ALLEGRO_COLOR backgroundColor;
    ALLEGRO_COLOR deadColor;
    ALLEGRO_COLOR firstAlive;
    ALLEGRO_COLOR secondAlive;
    ALLEGRO_COLOR textColor;
    if(useAllegro)
    {
        backgroundColor = al_map_rgb(0,0,0);
        deadColor = al_map_rgb(2,2,38);
        firstAlive = al_map_rgb(2,38,3);
        secondAlive = al_map_rgb(120,5,20);
        textColor = al_map_rgb(255,255,255);
        al_clear_to_color(backgroundColor);
    }
    float cellSize = current.cellSize;
    int currentDead = 0;
    int currentFirstAliveCount = 0;
    int currentSecondAliveCount = 0;
    for(unsigned i = 0;i < current.dim;i++)
    {
        for(unsigned j = 0;j < current.dim;j++)
        {
            int value = matrix[i][j];
            int x = i * cellSize;
            int y = j * cellSize;
            ALLEGRO_COLOR currentColor;
            if(value == 0)
            {
                currentColor = deadColor;
                currentDead++;
            }
            else if(value == 1)
            {
                currentColor = firstAlive;
                currentFirstAliveCount++;
            }
            else if(value == 2)
            {
                currentColor = secondAlive;
                currentSecondAliveCount++;
            }
                al_draw_filled_rectangle(x,y,x + cellSize,y + cellSize,currentColor);
        }
    }
    drawText(currentDead,time,maxTime,currentFirstAliveCount,currentSecondAliveCount,time,maxTime,current);
    al_flip_display();
}
void* handleInput(ALLEGRO_THREAD* thr,void* arg)
{
    ALLEGRO_EVENT_QUEUE* event_queue = al_create_event_queue();
    al_register_event_source(event_queue,al_get_keyboard_event_source());
    al_register_event_source(event_queue,al_get_display_event_source(display));
    while(!done)
    {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue,&event);
        if(event.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            if(event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
               done = true;
            else if(event.keyboard.keycode == ALLEGRO_KEY_1)
               showMenu = !showMenu;
        }
        else if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
          done = true;
    }
    al_unregister_event_source(event_queue,al_get_keyboard_event_source());
    al_unregister_event_source(event_queue,al_get_display_event_source(display));
    al_destroy_event_queue(event_queue);
    return nullptr;
}
void generateMatrix(const Configuration &current,vector<vector<int>> &mainMatrix,vector<vector<int>> &ausMatrix)
{
    srand(time(NULL));
    int currentDead = 0;
    int currentFirstAliveCount = 0;
    int currentSecondAliveCount = 0;
    for(unsigned i = 0;i < current.dim;i++)
    {
        for(unsigned j = 0;j < current.dim;j++)
        {
            int value = rand() % 3;
            if(current.deadRate + current.firstAliveRate + current.secondAliveRate == 100)
            {
               while(value == 0 && currentDead >= current.deadCount || value == 1 && currentFirstAliveCount >= current.firstAliveCount || value == 2 && currentSecondAliveCount >= current.secondAliveCount)
                   value = rand() % 3;
               if(value == 0)
                  currentDead++;
               else if(value == 1)
                  currentFirstAliveCount++;
               else
                  currentSecondAliveCount++;
            }
            mainMatrix[i][j] = value;
        }
    }
}
void game(const Configuration &current,vector<vector<int>> &mainMatrix,vector<vector<int>> &ausMatrix)
{
    int time = 0;
    int maxTime = current.maxIterations;
    while(time < maxTime && !done)
    {
        if(useAllegro)
            drawMatrix(mainMatrix,current,time,maxTime);
        for(unsigned i = 0;i < current.dim;i++)
        {
            for(unsigned j = 0;j < current.dim;j++)
            {         
                ausMatrix[i][j] = getValue(mainMatrix,i,j,current);
            }
        }
        for(unsigned i = 0;i < current.dim;i++)
        {
            for(unsigned j = 0;j < current.dim;j++)
               mainMatrix[i][j] = ausMatrix[i][j];
        }
        time++;
    }
    done = true;
}
void copy(vector<vector<int>> &first,vector<vector<int>> &second,const int &rows,const int &cols)
{
    for(unsigned i = 0;i < rows;i++)
    {
        for(unsigned j = 0;j < cols;j++)
            first[i][j] = second[i][j];
    }
}
void generateConfigured(vector<vector<int>> &mainMatrix)
{
    for(unsigned j = 400;j < 600;j++)
    {
        for(unsigned i = 400;i < 430;i++)
            mainMatrix[i][j] = 1;
        for(unsigned i = 570;i < 600;i++)
            mainMatrix[i][j] = 1;
    }
    for(unsigned i = 400;i < 600;i++)
    {
        for(unsigned j = 400;j < 430;j++)
            mainMatrix[i][j] = 1;
        for(unsigned j = 570;j < 600;j++)
            mainMatrix[i][j] = 1;
    }
    for(unsigned j = 430;j < 570;j++)
        for(unsigned i = 430;i < 570;i++)
            mainMatrix[i][j] = 2;
}
int main(int argc,char** argv)
{
    //Creates default configurations
    Configuration configurations[7];
    configurations[0] = Configuration(1,8,100,30,40,30,1000);
    configurations[1] = Configuration(2,8,100,20,60,20,1000);
    configurations[2] = Configuration(3,0.8,1000,60,20,20,100);
    configurations[3] = Configuration(4,0.8,1000,60,30,10,100);
    configurations[4] = Configuration(5,0.8,1000,30,65,5,100);
    configurations[5] = Configuration(6,8,100,0,0,0,1000);
    configurations[6] = Configuration(7,0.8,1000,0,0,0,100);
    if(argc == 2 && strcmp(argv[1],"-a") == 0)
        useAllegro = true;
    int value = 0;
    time_t startTime,endTime;
    while(value == 0 || value > 8)
    {
        for(Configuration current : configurations)
        {
            if(current.id == 7)
            {
                printf("Write 7 to use special configuration(square preset)\n");
                continue;
            }
            if(current.deadRate > 0 || current.firstAliveRate > 0 || current.secondAliveRate > 0)
                 printf("Write %d to use configuration %d: \nNumber of rows: %d,Number of cols: %d,Dead Rate: %d,First Alive Rate: %d,Second Alive Rate: %d,Tile size: %f\n",current.id,current.id,current.dim,current.dim,current.deadRate,current.firstAliveRate,current.secondAliveRate,current.cellSize);
            else
               printf("Write %d to use configuration %d: \nNumber of rows: %d,Number of cols: %d,Dead Rate: Not specified,First Alive Rate: Not specified,Second Alive Rate: Not specified,Tile size: %f\n",current.id,current.id,current.dim,current.dim,current.cellSize);
        }
        printf("Write a number:");
        std::cin >> value;
    }
    time(&startTime);
    Configuration current = configurations[value - 1];
    int dim  = current.dim;
    vector<vector<int>> mainMatrix(dim,vector<int>(dim,0));
    vector<vector<int>> ausMatrix(dim,vector<int>(dim,0));
    //Init input tread used to quit the application and toggle the visibility of the menu
    if(useAllegro)
    {
        initAllegro();
        ALLEGRO_THREAD* inputThread = al_create_thread(handleInput,NULL);
        al_start_thread(inputThread);
    }
    //Inits the game
    if(value == 7)
        generateConfigured(mainMatrix);
    else
       generateMatrix(current,mainMatrix,ausMatrix);
    game(current,mainMatrix,ausMatrix);
    if(useAllegro)
        al_destroy_display(display);
    time(&endTime);
    double executionTime = double(endTime - startTime);
    printf("Execution time: %f\n",executionTime);
    return 0;
}