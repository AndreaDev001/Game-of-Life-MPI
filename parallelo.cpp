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


#define WIDTH 1280
#define HEIGHT 720
ALLEGRO_DISPLAY* display = nullptr;
ALLEGRO_FONT* displayFont = nullptr;
bool finished = false;
bool useAllegro = false;
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
    Configuration(const int &id,const float &cellSize,const int &dim,const int &deadRate,const int &firstAliveRate,const int &secondAliveRate,const int &numIterations)
    {
        this->id = id;
        this->cellSize = cellSize;
        this->dim = dim;
        this->deadRate = deadRate;
        this->firstAliveRate = firstAliveRate;
        this->secondAliveRate = secondAliveRate;
        this->numIterations = numIterations;
        this->numberOfTiles = dim * dim;
        this->deadCount = (deadRate * numberOfTiles) / 100;
        this->firstAliveCount = (firstAliveRate * numberOfTiles) / 100;
        this->secondAliveCount = (secondAliveRate * numberOfTiles) / 100;
    }
};
void drawText(const int &currentDead,const int &currentFirstAliveCount,const int &currentSecondAliveCount,const int &currentIteration,const int &maxIterations,const Configuration &current)
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
void drawMatrix(vector<vector<int>> &map,const Configuration &current,const int &time,const int &maxTime)
{
    ALLEGRO_COLOR backgroundColor;
    ALLEGRO_COLOR deadColor;
    ALLEGRO_COLOR firstAlive;
    ALLEGRO_COLOR secondAlive;
    if(useAllegro)
    {
        backgroundColor = al_map_rgb(0,0,0);
        deadColor = al_map_rgb(2,2,38);
        firstAlive = al_map_rgb(2,38,3);
        secondAlive = al_map_rgb(120,5,20);
        al_clear_to_color(backgroundColor);
    }
    float cellSize = current.cellSize;
    int currentDead = 0;
    int currentFirstAlive = 0;
    int currentSecondAlive = 0;
    for(unsigned i = 0;i < map.size();i++)
    {
        vector<int> current = map[i];
        for(unsigned j = 0;j < current.size();j++)
        {
            int value = map[i][j];
            if(useAllegro)
            {
                ALLEGRO_COLOR currentColor;
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
    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    result = al_create_display(WIDTH,HEIGHT);
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
void generateMatrix(vector<vector<int>> &mainMatrix,const int &localRows,const int &world_rank,const int &world_size,const int &rows,const int &cols,const int &deadRate,const int &firstAliveRate,const int &secondAliveRate)
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
            if(deadRate > 0 && firstAliveRate > 0 && secondAliveRate > 0)
            {
                while(value == 0 && currentDead >= deadCount || value == 1 && currentFirstAlive >= firstAliveCount || value == 2 && currentSecondAlive >= secondAliveCount)
                   value = rand() % 3;
                if(value == 0)
                   currentDead++;
                else if(value == 1)
                   currentFirstAlive++;
                else if(value == 2)
                   currentSecondAlive++;
            }
            mainMatrix[i][j] = value;
        }
    }
}
int updateValue(vector<vector<int>> &mainMatrix,const int &i,const int &j)
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
void copy(vector<vector<int>> &first,vector<vector<int>> &second,const int &rows,const int &cols)
{
    for(unsigned i = 1;i < rows;i++)
    {
        for(unsigned j = 1;j < cols;j++)
            first[i][j] = second[i][j];
    }
}
void generateCostumConfiguration(vector<vector<int>> &mainMatrix,const int &world_rank,const int &localRowsWithGhost,const int &localColsWithGhost)
{
    if(world_rank == 1)
    {
        for(unsigned j = 400;j < 600;j++)
            for(unsigned i = 150;i < 180;i++)
                 mainMatrix[i][j] = 1;
        for(unsigned j = 430;j < 570;j++)
            for(unsigned i = 180;i < localRowsWithGhost - 1;i++)
                 mainMatrix[i][j] = 2;
        for(unsigned i = 150;i < localRowsWithGhost - 1;i++)
        {
            for(unsigned j = 400;j < 430;j++)
                 mainMatrix[i][j] = 1;
            for(unsigned j = 570;j < 600;j++)
                 mainMatrix[i][j] = 1;
        }
    }
    else if(world_rank == 2)
    {
        for(unsigned i = 1;i < 100;i++)
            for(unsigned j = 400;j < 430;j++)
                mainMatrix[i][j] = 1;
        for(unsigned j = 430;j < 570;j++)
            for(unsigned i = 1;i < 70;i++)
                mainMatrix[i][j] = 2;
        for(unsigned j = 400;j < 600;j++)
            for(unsigned i = 70;i < 100;i++)
                 mainMatrix[i][j] = 1;
        for(unsigned i = 1;i < 100;i++)
           for(unsigned j = 570;j < 600;j++)
               mainMatrix[i][j] = 1;

    }
}
Configuration getConfiguration()
{
    Configuration configurations[7];
    configurations[0] = Configuration(1,8,100,30,40,30,1000);
    configurations[1] = Configuration(2,8,100,20,60,20,1000);
    configurations[2] = Configuration(3,0.8,1000,60,20,20,100);
    configurations[3] = Configuration(4,0.8,1000,60,30,10,100);
    configurations[4] = Configuration(5,0.8,1000,30,65,5,100);
    configurations[5] = Configuration(6,8,100,0,0,0,1000);
    configurations[6] = Configuration(7,0.8,1000,0,0,0,100);
    int value = 0;
    while(value == 0 || value > 7)
    {
        for(Configuration current : configurations)
        {
            if(current.id != 7 && current.deadRate > 0 && current.firstAliveRate > 0 && current.secondAliveRate > 0)
                printf("Write %d to use configuration %d: \nNumber of rows: %d,Number of cols: %d,Dead Rate: %d,First Alive Rate: %d,Second Alive Rate: %d,Tile size: %f,Iterations: %d\n",current.id,current.id,current.dim,current.dim,current.deadRate,current.firstAliveRate,current.secondAliveRate,current.cellSize,current.numIterations);
            else if(current.id != 7)
                printf("Write %d to use configuration %d: \nNumber of rows: %d,Number of cols: %d,Dead Rate: Not specified,First Alive Rate: Not specified,Second Alive Rate: Not specified,Tile size: %f,Iterations: %d\n",current.id,current.id,current.dim,current.dim,current.cellSize,current.numIterations);
            else
                printf("Write 7 to use costum configuration(Square Preset)\n");
        }
        printf("Write a number:");
        std::cin >> value;
    }
    Configuration result = configurations[value - 1];
    return result;
}
void game(const int &world_rank,const int &world_size,const int &rows,const int &cols,const int &maxTime,const int &deadRate,const int &firstAliveRate,const int &secondAliveRate,const int &configurationId,const Configuration &current)
{
    int localRows = rows / world_size;
    if(world_rank == world_size - 1)
        localRows += rows % world_size;
    int localRowsWithGhost = localRows + 2;
    int localColsWithGhost = cols + 2;
    MPI_Datatype colType;
    MPI_Type_contiguous(localColsWithGhost,MPI_INT,&colType);
    MPI_Type_commit(&colType);
    vector<vector<int>> mainMatrix(localRowsWithGhost,vector<int>(localColsWithGhost,0));
    vector<vector<int>> ausMatrix(localRowsWithGhost,vector<int>(localColsWithGhost,0));
    if(configurationId != 7)
        generateMatrix(mainMatrix,localRows,world_rank,world_size,rows,cols,deadRate,firstAliveRate,secondAliveRate);
    else
        generateCostumConfiguration(mainMatrix,world_rank,localRowsWithGhost,localColsWithGhost);
    int upperNeighbour = world_rank == 0 ? world_size - 1 : world_rank - 1;
    int lowerNeighbour = world_rank == world_size - 1 ? 0 : world_rank + 1;
    for(unsigned time = 0;time < maxTime && !finished;time++)
    {
        MPI_Request drawRequests[localRowsWithGhost - 1];
        MPI_Request request;
        MPI_Status status;
        MPI_Isend(&mainMatrix[1][0],1,colType,upperNeighbour,0,MPI_COMM_WORLD,&request);
        MPI_Isend(&mainMatrix[localRows][0],1,colType,lowerNeighbour,0,MPI_COMM_WORLD,&request);
        MPI_Recv(&mainMatrix[localRows + 1][0],1,colType,lowerNeighbour,0,MPI_COMM_WORLD,&status);
        MPI_Recv(&mainMatrix[0][0],1,colType,upperNeighbour,0,MPI_COMM_WORLD,&status); 
        for(unsigned i = 0;i < localRows;i++)
        {
            mainMatrix[i][0] = mainMatrix[i][cols];
            mainMatrix[i][cols + 1] = mainMatrix[i][1];
        } 
        if(world_rank != 0 && useAllegro)
        {
            for(unsigned i = 1;i < localRowsWithGhost - 1;i++)
                MPI_Send(&mainMatrix[i][1],cols,MPI_INT,0,0,MPI_COMM_WORLD);
        }
        else if(useAllegro)
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
            if(useAllegro)
                drawMatrix(map,current,time,current.numIterations);
        }
        for(unsigned i = 1;i < localRowsWithGhost - 1;i++)
            for(unsigned j = 1;j < localColsWithGhost - 1;j++)
                ausMatrix[i][j] = updateValue(mainMatrix,i,j);
        copy(mainMatrix,ausMatrix,localRowsWithGhost -1,localColsWithGhost - 1);
        if(useAllegro)
            MPI_Bcast(&finished,1,MPI_INT,0,MPI_COMM_WORLD);
    }
    MPI_Type_free(&colType);
}
int main(int argc,char** argv)
{
    MPI_Init(&argc,&argv);
    int world_rank,world_size;
    MPI_Comm_size(MPI_COMM_WORLD,&world_size);
    MPI_Comm_rank(MPI_COMM_WORLD,&world_rank);
    if(argc == 2 && strcmp(argv[1],"-a") == 0)
        useAllegro = true;
    int configurationId,rows,cols,cellSize,deadRate,firstAliveRate,secondAliveRate,maxTime;
    double startTime,stopTime;
    Configuration current;
    if(world_rank == 0)
    {
        current = getConfiguration();
        startTime = MPI_Wtime();
        configurationId = current.id;
        rows = current.dim;
        cols = current.dim;
        cellSize = current.cellSize;
        deadRate = current.deadRate;
        firstAliveRate = current.firstAliveRate;
        secondAliveRate = current.secondAliveRate;
        maxTime = current.numIterations;
        if(useAllegro)
        {
             display = initAllegro();
             displayFont = al_load_font("RobotoCondensed-Bold.ttf",24,0);
             ALLEGRO_THREAD* thr = al_create_thread(handleInput,NULL);
             al_start_thread(thr);
        }
    }
    MPI_Bcast(&configurationId,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&rows,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&maxTime,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&deadRate,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&firstAliveRate,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&secondAliveRate,1,MPI_INT,0,MPI_COMM_WORLD);
    game(world_rank,world_size,rows,rows,maxTime,deadRate,firstAliveRate,secondAliveRate,configurationId,current);
    if(world_rank == 0)
    {
        stopTime = MPI_Wtime();
        if(useAllegro)
            al_destroy_display(display);
        printf("Execution time: %f\n",stopTime - startTime);
    }
    MPI_Finalize();
    return 0;
}