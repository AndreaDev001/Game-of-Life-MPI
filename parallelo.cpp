#include <mpi/mpi.h>
#include <unistd.h>
#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <vector>
#include <iostream>
using namespace std;


ALLEGRO_DISPLAY* display = nullptr;
ALLEGRO_FONT* displayFont = nullptr;
bool finished = false;
bool showMenu = true;
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
    int numIterations;
    Configuration()
    {
        
    }
    Configuration(int id,float cellSize,int dim,int deadRate,int firstAliveRate,int secondAliveRate,int numIterations)
    {
        this->id = id;
        this->cellSize = cellSize;
        this->dim = dim;
        this->numberOfTiles = dim * dim;
        this->deadRate = deadRate;
        this->firstAliveRate = firstAliveRate;
        this->secondAliveRate = secondAliveRate;
        this->numIterations = numIterations;
        this->deadCount = (deadRate * numberOfTiles) / 100;
        this->firstAliveCount = (firstAliveRate * numberOfTiles) / 100;
        this->secondAliveCount = (secondAliveRate * numberOfTiles) / 100;
    }
};
void drawText(int currentDead,int currentFirstAliveCount,int currentSecondAliveCount,int currentIteration,int maxIterations,Configuration current)
{
    const int WIDTH = 1280;
    ALLEGRO_COLOR textColor = al_map_rgb(255,255,255);
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,10,0,"Number of tiles: %d",current.numberOfTiles);
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,40,0,"Starting count of dead tiles: %d",current.deadCount);
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,70,0,"Starting count of alive tiles with value 1: %d",current.firstAliveCount);
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,100,0,"Starting count of alive tiles with value 2: %d",current.secondAliveCount);
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,130,0,"Current number of dead tiles: %d",currentDead);
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,160,0,"Curret number of alive tiles with value 1: %d",currentFirstAliveCount);
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,190,0,"Current number of alive tiles with value 2: %d",currentSecondAliveCount);
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,220,0,"Current Iteration: %d",currentIteration);
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,250,0,"Max Iterations: %d",maxIterations);
    al_draw_textf(displayFont,textColor,(WIDTH / 2) + 100,310,0,"Current Configuration: %d",current.id);
    al_draw_text(displayFont,textColor,(WIDTH / 2) + 100,340,0,"Press ESC to exit the application");
    al_draw_text(displayFont,textColor,(WIDTH / 2) + 100,370,0,"Press 1 to toggle the visibility of the menu");
}
void drawMatrix(vector<vector<int>> map,Configuration current,int time,int maxTime)
{
    ALLEGRO_COLOR backgroundColor = al_map_rgb(0,0,0);
    ALLEGRO_COLOR deadColor = al_map_rgb(2,2,38);
    ALLEGRO_COLOR firstAlive = al_map_rgb(2,38,3);
    ALLEGRO_COLOR secondAlive = al_map_rgb(120,5,20);
    float cellSize = current.cellSize;
    int currentDead = 0;
    int currentFirstAlive = 0;
    int currentSecondAlive = 0;
    al_clear_to_color(backgroundColor);
    for(unsigned i = 0;i < map.size();i++)
    {
        vector<int> current = map[i];
        for(unsigned j = 0;j < current.size();j++)
        {
            ALLEGRO_COLOR currentColor;
            int value = map[i][j];
            int x = i * cellSize;
            int y = j * cellSize;
            if(value == 0)
            {
                currentColor = deadColor;
                currentDead++;
            }
            else if(value == 1)
            {
                currentColor = firstAlive;
                currentFirstAlive++;
            }
            else if(value == 2)
            {
                currentColor = secondAlive;
                currentSecondAlive++;
            }
            al_draw_filled_rectangle(x,y,x + cellSize,y + cellSize,currentColor);
        }
    }
    if(showMenu)
       drawText(currentDead,currentFirstAlive,currentSecondAlive,time,maxTime,current);
    al_flip_display();
}
void* handleInput(ALLEGRO_THREAD* thr,void* arg)
{
    ALLEGRO_EVENT_QUEUE* event_queue = al_create_event_queue();
    al_register_event_source(event_queue,al_get_display_event_source(display));
    al_register_event_source(event_queue,al_get_keyboard_event_source());
    while(!finished)
    {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue,&event);
        if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
           finished = true;
        else if(event.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            if(event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                finished = true;
            else if(event.keyboard.keycode == ALLEGRO_KEY_1)
                showMenu = !showMenu;
        }
    }
    al_unregister_event_source(event_queue,al_get_display_event_source(display));
    al_unregister_event_source(event_queue,al_get_keyboard_event_source());
    al_destroy_event_queue(event_queue);
    return nullptr;
}
ALLEGRO_DISPLAY* initAllegro()
{
    ALLEGRO_DISPLAY* result;
    if(!al_init())
    {
        al_show_native_message_box(NULL,"Allegro Error","Header Error","Could not init allegro correctly","",0);
        return nullptr;
    }
    al_set_new_window_title("Game of life");
    result = al_create_display(1280,720);
    if(!result)
    {
        al_show_native_message_box(NULL,"Allegro Error","Header Error","Could not create display correctly","",0);
        return nullptr;
    }
    al_install_keyboard();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    return result;
}
void generateMatrix(vector<vector<int>> &mainMatrix,int localRows,int world_rank,int world_size,int rows,int cols,int deadRate,int firstAliveRate,int secondAliveRate)
{
    srand(time(NULL) + world_rank);
    int dim = rows * cols;
    int deadCount = (deadRate * dim) / 100;
    int firstAliveCount = (firstAliveRate * dim) / 100;
    int secondAliveCount = (secondAliveRate * dim) / 100;
    deadCount = deadCount / world_size;
    firstAliveCount = firstAliveCount / world_size;
    secondAliveCount = secondAliveCount / world_size;
    int currentDead = 0;
    int currentFirstAlive = 0;
    int currentSecondAlive = 0;
    for(unsigned i = 1;i <= localRows;i++)
    {
        for(unsigned j = 1;j <= cols;j++)
        {
            int value = rand() % 3;
            while(value == 0 && currentDead > deadCount || value == 1 && currentFirstAlive > firstAliveCount || value == 2 && currentSecondAlive > secondAliveCount)
                  value = rand() % 3;
            if(value == 0)
               currentDead++;
            else if(value == 1)
               currentFirstAlive++;
            else if(value == 2)
               currentSecondAlive++;
            mainMatrix[i][j] = value;
        }
    }
}
int updateValue(vector<vector<int>> &mainMatrix,int i,int j)
{
    int firstAlive = 0;
    int secondAlive = 0;
    int aliveNeighbours = 0;
    for(int currentI = i - 1;currentI <= i + 1;currentI++)
    {
        for(int currentJ = j - 1;currentJ <= j + 1;currentJ++)
        {
            if((currentI != i || currentJ != j) && mainMatrix[currentI][currentJ] == 1)
               firstAlive++;
            else if((currentI != i || currentJ != j) && mainMatrix[currentI][currentJ] == 2)
               secondAlive++;
        }
    }
    aliveNeighbours = firstAlive + secondAlive;
    int value = mainMatrix[i][j];
    if(value == 0 && aliveNeighbours == 3)
        return firstAlive > secondAlive ? 1 : 2;
    else if(value == 1 || value == 2)
    {
        if(aliveNeighbours > 3 || aliveNeighbours < 2)
           return 0;
    }
    return value;
}
void copy(vector<vector<int>> &first,vector<vector<int>> &second,int rows,int cols)
{
    for(unsigned i = 1;i < rows;i++)
    {
        for(unsigned j = 1;j < cols;j++)
            first[i][j] = second[i][j];
    }
}
Configuration getConfiguration()
{
    Configuration configurations[6];
    configurations[0] = Configuration(1,8,100,30,60,10,1000);
    configurations[1] = Configuration(2,8,100,90,5,5,1000);
    configurations[2] = Configuration(3,8,100,20,60,20,1000);
    configurations[3] = Configuration(4,0.8,1000,60,20,20,1000);
    configurations[4] = Configuration(5,0.8,1000,60,30,10,1000);
    configurations[5] = Configuration(6,0.8,1000,30,65,5,1000);
    int value = 0;
    while(value == 0 || value > 6)
    {
        for(Configuration current : configurations)
            printf("Write %d to use configuration %d: \nNumber of rows: %d,Number of cols: %d,Dead Rate: %d,First Alive Rate: %d,Second Alive Rate: %d,Tile size: %f,Iterations: %d\n",current.id,current.id,current.dim,current.dim,current.deadRate,current.firstAliveRate,current.secondAliveRate,current.cellSize,current.numIterations);
        printf("Write a number:");
        std::cin >> value;
    }
    Configuration result = configurations[value - 1];
    return result;
}
void game(int world_rank,int world_size,int rows,int cols,int maxTime,int deadRate,int firstAliveRate,int secondAliveRate,Configuration current)
{
    int localRows = rows / world_size;
    if(world_rank == world_size - 1)
        localRows += rows % world_size;
    int localRowsWithGhost = localRows + 2;
    int localColsWithGhost = cols + 2;
    vector<vector<int>> mainMatrix(localRowsWithGhost,vector<int>(localColsWithGhost,0));
    vector<vector<int>> ausMatrix(localRowsWithGhost,vector<int>(localColsWithGhost,0));
    generateMatrix(mainMatrix,localRows,world_rank,world_size,rows,cols,deadRate,firstAliveRate,secondAliveRate);
    int upperNeighbour = world_rank == 0 ? world_size - 1 : world_rank - 1;
    int lowerNeighbour = world_rank == world_size - 1 ? 0 : world_rank + 1;
    for(unsigned time = 0;time < maxTime && !finished;time++)
    {
        MPI_Request request;
        MPI_Status status;
        MPI_Isend(&mainMatrix[1][0],localColsWithGhost,MPI_INT,upperNeighbour,0,MPI_COMM_WORLD,&request);
        MPI_Isend(&mainMatrix[localRows][0],localColsWithGhost,MPI_INT,lowerNeighbour,0,MPI_COMM_WORLD,&request);
        MPI_Recv(&mainMatrix[localRows + 1][0],localColsWithGhost,MPI_INT,lowerNeighbour,0,MPI_COMM_WORLD,&status);
        MPI_Recv(&mainMatrix[0][0],localColsWithGhost,MPI_INT,upperNeighbour,0,MPI_COMM_WORLD,&status); 
        for(unsigned i = 0;i < localRows;i++)
        {
            mainMatrix[i][0] = mainMatrix[i][cols];
            mainMatrix[i][cols + 1] = mainMatrix[i][1];
        } 
        if(world_rank != 0)
        {
            for(unsigned i = 1;i < localRowsWithGhost - 1;i++)
            {
                MPI_Request request;
                MPI_Isend(&mainMatrix[i][1],cols,MPI_INT,0,0,MPI_COMM_WORLD,&request);
            }
        }
        else
        {
            vector<vector<int>> map;
            for(unsigned i = 1;i < localRowsWithGhost - 1;i++)
            {
                vector<int> current;
                for(unsigned j = 1;j < localColsWithGhost - 1;j++)
                    current.push_back(mainMatrix[i][j]);
                map.push_back(current);
            }
            for(unsigned i = 1;i < world_size;i++)
            {
                int count = rows / world_size;
                if(i == world_size - 1)
                   count += rows % world_size;
                vector<int> buff(cols,0);
                for(unsigned j = 0;j < count;j++)
                {
                   MPI_Status status;
                   MPI_Recv(&buff[0],cols,MPI_INT,i,0,MPI_COMM_WORLD,&status);
                   map.push_back(buff);
                }
            }
            drawMatrix(map,current,time,current.numIterations);
        }
        for(unsigned i = 1;i < localRowsWithGhost - 1;i++)
            for(unsigned j = 1;j < localColsWithGhost - 1;j++)
                ausMatrix[i][j] = updateValue(mainMatrix,i,j);
        copy(mainMatrix,ausMatrix,localRowsWithGhost -1,localColsWithGhost - 1);
    }
}
int main(int argc,char** argv)
{
    MPI_Init(NULL,NULL);
    int world_rank,world_size;
    MPI_Comm_size(MPI_COMM_WORLD,&world_size);
    MPI_Comm_rank(MPI_COMM_WORLD,&world_rank);
    int rows,cols,cellSize,deadRate,firstAliveRate,secondAliveRate,maxTime;
    double startTime,stopTime;
    Configuration current;
    if(world_rank == 0)
    {
        current = getConfiguration();
        startTime = MPI_Wtime();
        rows = current.dim;
        cols = current.dim;
        cellSize = current.cellSize;
        deadRate = current.deadRate;
        firstAliveRate = current.firstAliveRate;
        secondAliveRate = current.secondAliveRate;
        maxTime = current.numIterations;
        display = initAllegro();
        displayFont = al_load_font("RobotoCondensed-Bold.ttf",24,0);
        ALLEGRO_THREAD* thr = al_create_thread(handleInput,NULL);
        al_start_thread(thr);
    }
    MPI_Bcast(&rows,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&cols,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&maxTime,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&deadRate,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&firstAliveRate,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&secondAliveRate,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&cellSize,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&maxTime,1,MPI_INT,0,MPI_COMM_WORLD);
    game(world_rank,world_size,rows,cols,maxTime,deadRate,firstAliveRate,secondAliveRate,current);
    if(world_rank == 0)
    {
        finished = true;
        MPI_Bcast(&finished,1,MPI_INT,0,MPI_COMM_WORLD);
        stopTime = MPI_Wtime();
        printf("Execution time: %f\n",stopTime - startTime);
        al_destroy_display(display);
    }
    MPI_Finalize();
    return 0;
}