//g++ NSD.cc -o NSD.out
//./NSD.out

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cmath>

//Read data from STK
#define CHINA_BASE_STATION_X 1000
#define CHINA_BASE_STATION_Y 500
#define USA_BASE_STATION_X 3000
#define USA_BASE_STATION_Y 3800
#define HEIGHT 1500
#define SEEABLE 2000

const int NETWORK_ROWS = 5;
const int NETWORK_COLS = 8;
const int SATELLITE_SPACING_SAME_ORBIT = 400;
const int SATELLITE_SPACING_BETWEEN_ORBIT = 1000;
const int SATELLITE_SPEED = 50;

const int DELTA_T = 30;
const int SIMULATION_KAISU = 50;
const int TOTAL_SIMULATION_TIME = DELTA_T * SIMULATION_KAISU;

int main() {

    int i,j,k;
    double SAT_X[NETWORK_ROWS][NETWORK_COLS];
    double SAT_Y[NETWORK_ROWS][NETWORK_COLS];
    int service_time[NETWORK_ROWS][NETWORK_COLS];
    std::srand(std::time(nullptr));
    
    //Initialization (x,y)
    int startX = 0;
    int startY = 0;
    for(i=0; i<=NETWORK_ROWS-1; i++)
    {
        for(j=0; j<=NETWORK_COLS-1; j++)
        {
            SAT_X[i][j] = startX + j * SATELLITE_SPACING_SAME_ORBIT;
            SAT_Y[i][j] = startY + i * SATELLITE_SPACING_BETWEEN_ORBIT;
            //std::cout << "x:" << SAT_X[i][j] << "  y:" << SAT_Y[i][j] << std::endl;
            int random_value = std::round((std::rand() % (SIMULATION_KAISU/2)*100) / 100.0);
            service_time[i][j] = random_value;
            //std::cout << "service_time:" << service_time[i][j] << std::endl;
        }
    }
    //std::cout << "max_x:" << SAT_X[NETWORK_ROWS-1][NETWORK_COLS-1] << std::endl;
    //std::cout << "max_y:" << SAT_Y[NETWORK_ROWS-1][NETWORK_COLS-1] << std::endl;

    double min_distance = 99999;
    int min_x,min_y,min_number = 0;//DEFAULT
    int fll = 0;
    int CHINA_connected_num[SIMULATION_KAISU];
    int CHINA_connected_x[SIMULATION_KAISU];
    int CHINA_connected_y[SIMULATION_KAISU];
    int USA_connected_num[SIMULATION_KAISU];
    int USA_connected_x[SIMULATION_KAISU];
    int USA_connected_y[SIMULATION_KAISU];
    for(k=0; k<=TOTAL_SIMULATION_TIME; k+=DELTA_T)
    {
        //satellite movement
        for(i=0; i<=NETWORK_ROWS-1; i++)
        {
            for(j=0; j<=NETWORK_COLS-1; j++)
            {
                SAT_X[i][j] += SATELLITE_SPEED * 1;   //only move in x-direction
                SAT_Y[i][j] += 0;
                service_time[i][j] -= 1;
            }
        }

        //CHINA linking-logic
        for(i=0; i<=NETWORK_ROWS-1; i++)
        {
            for(j=0; j<=NETWORK_COLS-1; j++)
            {
                if(service_time[i][j] <= 0)     continue;

                double distance = std::sqrt(std::pow(SAT_X[i][j]-CHINA_BASE_STATION_X,2)+
                                            std::pow(SAT_Y[i][j]-CHINA_BASE_STATION_Y,2)+
                                            std::pow(HEIGHT,2));
                if(distance < min_distance){
                    min_distance = distance;
                    min_x = i;
                    min_y = j;
                    min_number = i*NETWORK_ROWS + j;
                }
            }
        }
        CHINA_connected_num[fll] = min_number;
        CHINA_connected_x[fll] = min_x;
        CHINA_connected_y[fll] = min_y;
        //Hold or Switch
        if(fll>0){
            double last_distance = std::sqrt(std::pow(SAT_X[CHINA_connected_x[fll-1]][CHINA_connected_y[fll-1]]-CHINA_BASE_STATION_X,2)+
                                             std::pow(SAT_Y[CHINA_connected_x[fll-1]][CHINA_connected_y[fll-1]]-CHINA_BASE_STATION_Y,2)+
                                             std::pow(HEIGHT,2));
            CHINA_connected_num[fll] = (last_distance > SEEABLE) ?
                                        min_number :
                                        CHINA_connected_num[fll-1];
            CHINA_connected_x[fll] = (last_distance > SEEABLE) ?
                                        min_x :
                                        CHINA_connected_x[fll-1];
            CHINA_connected_y[fll] = (last_distance > SEEABLE) ?
                                        min_y :
                                        CHINA_connected_y[fll-1];
        }

        //------------------------------------------------------------------------
        min_distance = 99999;
        min_x = 0;
        min_y = 0;
        //------------------------------------------------------------------------

        //USA linking-logic
        for(i=0; i<=NETWORK_ROWS-1; i++)
        {
            for(j=0; j<=NETWORK_COLS-1; j++)
            {
                if(service_time[i][j] <= 0)     continue;

                double distance = std::sqrt(std::pow(SAT_X[i][j]-USA_BASE_STATION_X,2)+
                                            std::pow(SAT_Y[i][j]-USA_BASE_STATION_Y,2)+
                                            std::pow(HEIGHT,2));
                double last_distance = std::sqrt(std::pow(SAT_X[min_x][min_y]-USA_BASE_STATION_X,2)+
                                                 std::pow(SAT_Y[min_x][min_y]-USA_BASE_STATION_Y,2)+
                                                 std::pow(HEIGHT,2));
                if(distance < min_distance){
                    min_distance = distance;
                    min_x = i;
                    min_y = j;
                    min_number = i*NETWORK_ROWS + j;
                }
            }
        }
        USA_connected_num[fll] = min_number;
        USA_connected_x[fll] = min_x;
        USA_connected_y[fll] = min_y;
        //Hold or Switch
        if(fll>0){
            double last_distance = std::sqrt(std::pow(SAT_X[USA_connected_x[fll-1]][USA_connected_y[fll-1]]-CHINA_BASE_STATION_X,2)+
                                             std::pow(SAT_Y[USA_connected_x[fll-1]][USA_connected_y[fll-1]]-CHINA_BASE_STATION_Y,2)+
                                             std::pow(HEIGHT,2));
            USA_connected_num[fll] = (last_distance > SEEABLE) ?
                                        min_number :
                                        USA_connected_num[fll-1];
            USA_connected_x[fll] = (last_distance > SEEABLE) ?
                                        min_x :
                                        USA_connected_x[fll-1];
            USA_connected_y[fll] = (last_distance > SEEABLE) ?
                                        min_y :
                                        USA_connected_y[fll-1];
        }

        min_distance = 99999;
        min_x = 0;
        min_y = 0;
        
        fll++;
    }



    std::cout << "Total kaisu:" << SIMULATION_KAISU << std::endl;
    std::cout << "--------NSD--------" << std::endl;
    //OUTPUT to txt
    int CHINA_size = sizeof(CHINA_connected_num) / sizeof(CHINA_connected_num[0]);
    std::ofstream chinaFile("NSD_sat1.txt");
    if (chinaFile.is_open()) {
        for (int i = 0; i < CHINA_size; ++i) {
            chinaFile << CHINA_connected_num[i] << "\n";
        }
        chinaFile.close();
        std::cout << "写入文件成功：NSD_sat1.txt" << std::endl;
    } else {
        std::cerr << "无法打开文件：NSD_sat1.txt" << std::endl;
    }

    int USA_size = sizeof(USA_connected_num) / sizeof(USA_connected_num[0]);
    std::ofstream usaFile("NSD_sat2.txt");
    if (usaFile.is_open()) {
        for (int i = 0; i < USA_size; ++i) {
            usaFile << USA_connected_num[i] << "\n";
        }
        usaFile.close();
        std::cout << "写入文件成功：NSD_sat2.txt" << std::endl;
    } else {
        std::cerr << "无法打开文件：NSD_sat2.txt" << std::endl;
    }

    return 0;
}
