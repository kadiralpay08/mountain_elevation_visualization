//Kadir - Nifty Mountains - September 19, 2025
//Program reads in elevation integers and prints an elevation graph (also has an algorithm to find most efficient paths based on elevation change)
//No bugs that I could identify, but I couldn't find a way to check the average change in elevation found by the Downhill Method
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>
#include "APMATRIX.H"

using namespace std;

#define ROW 480
#define COL 844
#define MAX 403200
#define FILE "Colorado.dat"

//defining common colours and rgb colour values to be used mostly in color interpolation
#define RED al_map_rgb(255, 0, 0)
#define GREEN al_map_rgb(0, 255, 0)
#define VALLEY al_map_rgb(0, 154, 23)
#define VALLEYRED 0
#define VALLEYGREEN 154
#define VALLEYBLUE 23
#define ROCK al_map_rgb(184, 113, 82)
#define ROCKRED 184
#define ROCKGREEN 113
#define ROCKBLUE 82
#define BEIGE (213, 197, 138)
#define BEIGERED 213
#define BEIGEGREEN 197
#define BEIGEBLUE 138

ALLEGRO_DISPLAY *display;
ALLEGRO_TIMER *timer;
ALLEGRO_FONT *arial;
ALLEGRO_EVENT_QUEUE *event_queue;

//function prototypes
int initializeAllegro(int width, int height, const char title[]);
bool MapDataDrawer(const char *fname, apmatrix<int> &map);
int findMin(const apmatrix<int> &map);
int findMax(const apmatrix<int> &map);
void drawMap(const apmatrix<int> &map, int min, int max);
int indexOfMinRow(const apmatrix<int> &map, const int &col);
int drawLowestElevPath(const apmatrix<int> &map, int row, ALLEGRO_COLOR colour);
int indexOfLowestElevPath(const apmatrix<int> &map);
int drawDownhillElevPath(const apmatrix<int> &map, int &minElevSum);

int main(){
    //seed the time for random number generation
    srand(time(0));

    //defining the matrix and variables
    apmatrix<int> map(ROW, COL);

    //prepare the graphics screen
    initializeAllegro(COL, ROW, "Nifty Mountains");
    srand(time(NULL));

    //check to see if file opened before drawing elevation map
    if (MapDataDrawer(FILE , map) == false){
        cout << "Error";
    } else {
        drawMap(map, findMin(map), findMax(map));
    }

    //print relevant results and do simple calculations like converting total change in elevation to average change in elevation
    int lowestElevPath = indexOfLowestElevPath(map);
    cout << "Greedy walk method found the lowest elevation path to begin on row " << lowestElevPath << " with a change in elevation of " << drawLowestElevPath(map, lowestElevPath, GREEN) << endl;
    int minElevSum = 0;
    cout << "Downhill method found the lowest elevation path to begin on row " << drawDownhillElevPath(map, minElevSum) << " with an average elevation of " << minElevSum / COL;

    //printing the title and legend of the map
    al_draw_text(arial, al_map_rgb(255, 255, 255), 10, 10, ALLEGRO_ALIGN_LEFT, "Nifty Mountains: Elevation Map of Colorado");
    al_draw_text(arial, al_map_rgb(255, 255, 255), 10, 35, ALLEGRO_ALIGN_LEFT, "Green = Low Elevation");
    al_draw_text(arial, al_map_rgb(255, 255, 255), 10, 55, ALLEGRO_ALIGN_LEFT, "Tan = High Elevation");
    al_draw_text(arial, al_map_rgb(255, 255, 255), 10, 75, ALLEGRO_ALIGN_LEFT, "Green Path = Lowest Change in Elevation Path (Greedy Walk)");
    al_draw_text(arial, al_map_rgb(255, 255, 255), 10, 95, ALLEGRO_ALIGN_LEFT, "Blue Path = Lowest Elevation Path (Downhill Method)");
    al_flip_display();

    //rest necessary to keep graphics screen open
    al_rest(99999999);
    return 0;
}

//opens and initializes the allegro graphics screen (written by Mr. Creelman)
int initializeAllegro(int width, int height, const char title[]) {

    const float FPS = 16;           // set frame rate
    ALLEGRO_COLOR background = al_map_rgb(255, 255, 255);       // make background white.

    // Initialize Allegro
    al_init();


    // initialize display
    display = al_create_display(width, height);
    if (!display) {
        al_show_native_message_box(display, "Error", "Error", "Failed to initialize display!",
                                 nullptr, ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }
    al_set_window_title(display, title);

    // Initialize keyboard routines
    if (!al_install_keyboard()) {
        al_show_native_message_box(display, "Error", "Error", "failed to initialize the keyboard!",
                                 nullptr, ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }

    // need to add image processor
    if (!al_init_image_addon()) {
        al_show_native_message_box(display, "Error", "Error", "Failed to initialize image addon!",
                                 nullptr, ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }
    // 2. setup timer
    timer = al_create_timer(1.0 / FPS);
    if (!timer) {
        al_show_native_message_box(display, "Error", "Error", "Failed to create timer!",
                                 nullptr, ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }
    // Add fonts


   al_init_font_addon(); // initialize the font addon
   al_init_ttf_addon();// initialize the ttf (True Type Font) addon

   arial = al_load_ttf_font("C:/Windows/Fonts/arial.ttf", 16, 0);
   if (!arial){
      al_show_native_message_box(display, "Error", "Error", "Could not load arial.ttf",
                                    nullptr, ALLEGRO_MESSAGEBOX_ERROR);
      return -1;
   }
    // Initialize primative add on
    if (!al_init_primitives_addon()) {
        al_show_native_message_box(display, "Error", "Error", "Failed to initialize primatives addon!",
                                 nullptr, ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }

    // set up event queue
    event_queue = al_create_event_queue();
    if (!event_queue) {
        al_show_native_message_box(display, "Error", "Error", "Failed to create event_queue!",
                                 nullptr, ALLEGRO_MESSAGEBOX_ERROR);
        al_destroy_display(display);
        return -1;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(timer));        // 3. register timer events

    al_clear_to_color(background);

    al_flip_display();
    //al_start_timer(timer);
//  std::cout << "Init return zero " << std::endl;
    return 0;
}

//read data from given file (fname) into a 2D array (use apmatrix or vector of a vector)
bool MapDataDrawer(const char *fname, apmatrix<int> &map){
    ifstream inFile(fname);

    //check if file opened before reading in values
    if (!inFile.is_open()){
        return false;
    } else {
        for (int i = 0; i < ROW; i++){
            for (int j = 0; j < COL; j++){
                inFile >> map[i][j];
            }
        }
    }
    return true;
}

//return the minimum value in the map
int findMin(const apmatrix<int> &map){
    int min = map[0][0];

    //loop through each value and find the minimum
    for (int i = 0; i < ROW; i++){
        for (int j = 0; j < COL; j++){
            if (map[i][j] < min){
                min = map[i][j];
            }
        }
    }
    return min;
}

//return the max value in the map
int findMax(const apmatrix<int> &map){
    int max = map[0][0];

    //loop through each value and find the maximum
    for (int i = 0; i < ROW; i++){
        for (int j = 0; j < COL; j++){
            if (map[i][j] > max){
                max = map[i][j];
            }
        }
    }
    return max;
}

//draw this map in B&W using given graphics context, you may want more parameters, and may require more edits by you.
void drawMap(const apmatrix<int> &map, int min, int max){
    //commented out code was used to generate the monochromatic map
    //int diff = max - min;
    //float interval = diff / 256.0;

    //c will be a number between 0 and 1 that represents how much the color tan and green affect the resulting color value of a square(color interpolation)
    float c;
    ALLEGRO_COLOR colour;
    for (int i = 0; i < ROW; i++){
        for (int j = 0; j < COL; j++){
            //c = (map[i][j] - min) / interval; //calculated grayscale value
            c = (float)(map[i][j] - min)/(max - min);

            //if the elevation is closer to the minimum
            if (c < 0.5){
                //colour is mostly affected by green due to color interpolation math
                colour = al_map_rgb(VALLEYRED + c*(BEIGERED - VALLEYRED), VALLEYGREEN + c*(BEIGEGREEN - VALLEYGREEN), VALLEYBLUE + c*(BEIGEBLUE - VALLEYBLUE));
            //if the elevation is closer to the maximum
            } else {
                //colour is mostly affected by tan due to color interpolation math
                colour = al_map_rgb(BEIGERED + c*(ROCKRED - BEIGERED), BEIGEGREEN + c*(ROCKGREEN - BEIGEGREEN), BEIGEBLUE + c*(ROCKBLUE - BEIGEBLUE));
            }
            //draw in the square
            al_draw_filled_rectangle(j, i, j+1, i+1, colour);
        }
    }
    al_flip_display();
}

//given a column, find the index of the row with min elevation and return row number of
int indexOfMinRow(const apmatrix<int> &map, const int &col){
    int min = map[0][col];

    //loop through the given column and find minimum value
    for (int i = 0; i < ROW; i++){
        if (map[i][col] < min){
            min = map[i][col];
        }
    }
    return min;
}

//draw the lowest elevation path starting from the given row and return total elev change from the path
int drawLowestElevPath(const apmatrix<int> &map, int row, ALLEGRO_COLOR colour){
    int totalElevChange = 0, currentRow = row, rowUp, rowSame, rowDown;

    //draw first square
    al_draw_filled_rectangle(0, row, 0+1, row+1, colour);

    //greedy algorithm for each column
    for (int i = 1; i < COL; i++){
        //define upcoming choices for simplicity and readability of code, also check for valid values
        rowSame = abs(map[currentRow][i] - map[currentRow][i - 1]);
        if (currentRow != 0){
            rowUp = abs(map[currentRow - 1][i] - map[currentRow][i - 1]);
        }
        if (currentRow != ROW - 1){
            rowDown = abs(map[currentRow + 1][i] - map[currentRow][i - 1]);
        }

        //find the smallest change in elevation and take that path
        if ((currentRow - 1 < 0 && rowSame <= rowDown) || (currentRow + 1 >= ROW && rowSame <= rowUp) || (rowSame <= rowUp && rowSame <= rowDown)){
            al_draw_filled_rectangle(i, currentRow, i + 1, currentRow + 1, colour);
            totalElevChange += rowSame;
        } else if (currentRow + 1 >= ROW || (currentRow - 1 >= 0 && rowUp < rowDown)){
            al_draw_filled_rectangle(i, currentRow - 1, i + 1, (currentRow - 1) + 1, colour);
            totalElevChange += rowUp;
            currentRow -= 1;
        } else if (currentRow - 1 < 0 || rowDown < rowUp){
            al_draw_filled_rectangle(i, currentRow + 1, i + 1, (currentRow + 1) + 1, colour);
            totalElevChange += rowDown;
            currentRow += 1;
        } else {
            int random = rand() % 2;
            if (random == 0){
                al_draw_filled_rectangle(i, currentRow - 1, i + 1, (currentRow - 1) + 1, colour);
                totalElevChange += rowUp;
                currentRow -= 1;
            } else {
                al_draw_filled_rectangle(i, currentRow + 1, i + 1, (currentRow + 1) + 1, colour);
                totalElevChange += rowDown;
                currentRow += 1;
            }
        }
    }
    al_flip_display();
    return totalElevChange;
}

//find the lowest elev change path in the map and return the row it starts on
int indexOfLowestElevPath(const apmatrix<int> &map){
    //call the drawLowestElevPath function for each row and find the path with minimum elevation change
    int startRow = 0, min = drawLowestElevPath(map, 0, RED);
    for (int i = 1; i < ROW; i++){
        int rowElev = drawLowestElevPath(map, i, RED);
        if (rowElev < min){
            min = rowElev;
            startRow = i;
        }
    }
    return startRow;
}

//find the lowest average elevation path in the map and return the row it starts on
int drawDownhillElevPath(const apmatrix<int> &map, int &minElevSum){
    //set minElevSum to a high number so that the first value checked against it becomes the new minimum
    minElevSum = 9999999;
    int elevSum, bestRow, rowUp, rowSame, rowDown;

    for (int i = 0; i < ROW; i++){
        //set elevSum to the first value of the row so that we can skip adding that to the total elevation
        elevSum = map[i][0];
        int currentRow = i;
        for (int j = 0; j < COL - 1; j++){
            //define upcoming choices for simplicity and readability of code, also check for valid values
            rowSame = map[currentRow][j + 1];
            if (currentRow != 0){
                rowUp = map[currentRow - 1][j + 1];
            }
            if (currentRow != ROW - 1){
                rowDown = map[currentRow + 1][j + 1];
            }

            //find the path that takes you the most downhill
            if (currentRow != 0 && (rowUp <= rowSame && rowUp <= rowDown)){
                elevSum += rowUp;
                currentRow -= 1;
            } else if (currentRow != ROW - 1 && rowDown <= rowSame){
                elevSum += rowDown;
                currentRow += 1;
            } else {
                elevSum += rowSame;
            }
        }
        //find minimum average elevation change path
        if (elevSum < minElevSum){
            minElevSum = elevSum;
            bestRow = i;
        }
    }

    //print out the minimum elevation change path
    ALLEGRO_COLOR colour = al_map_rgb(0, 0, 255);
    int currentRow = bestRow;
    for (int i = 0; i < COL - 1; i++){
        //define upcoming choices for simplicity and readability of code, also check for valid values
        rowSame = map[currentRow][i + 1];
        if (currentRow != 0){
            rowUp = map[currentRow - 1][i + 1];
        }
        if (currentRow != ROW - 1){
            rowDown = map[currentRow + 1][i + 1];
        }

        //draw the path that goes the most downhill
        if (currentRow != 0 && (rowUp <= rowSame && rowUp <= rowDown)){
            al_draw_filled_rectangle(i, currentRow - 1, i + 1, (currentRow - 1) + 1, colour);
            currentRow -= 1;
        } else if (currentRow != ROW - 1 && rowDown <= rowSame){
            al_draw_filled_rectangle(i, currentRow + 1, i + 1, (currentRow + 1) + 1, colour);
            currentRow += 1;
        } else {
            al_draw_filled_rectangle(i, currentRow, i + 1, currentRow + 1, colour);
        }
    }
    al_flip_display();
    return bestRow;
}
