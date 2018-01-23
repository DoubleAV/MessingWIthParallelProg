#include <iostream>
#include <math.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <tuple>
#include <map>

using namespace std;



//First of all I apologize for doing this all in main, if I were to do this again
//I would definitely modularize parts of this program.
int main() {

    ifstream data("hpc_data_starter.csv");

    //Make sure the file opens correctly
    if (!data.is_open()) std::cout << "Error on: File Open" << '\n';

    std::string str;
    std::getline(data, str); //skips the first line so I can get to the actual data


    std::map<std::pair<double,double>,std::vector<double> > info;

    double x;
    double y;
    double lon;
    double lat;
    double delta_x;
    double delta_y;
    double feature;
    std::vector<double> features;

    while(data.good()){
//    for (int i = 0; i<2; i++){

        //clears the temporary feature vector so that each line's features can be added to the map
        //as one vector
        features.clear();

        //grab longitude from file
        std::getline(data, str, ':');
        lon = atof(str.c_str()); // convert string to double

        //get latitude
        std::getline(data, str, ':');
        lat = atof(str.c_str());

        //get delta X
        std::getline(data, str, ':');
        delta_x = atof(str.c_str());

        //get delta Y
        std::getline(data, str, ':');
        delta_y = atof(str.c_str());

        //loop through the feature columns 0-19
        for (int j = 0; j<20; j++){
            std::getline(data, str, ':');
            feature = atof(str.c_str());
            features.push_back(feature);

        }

        //handles the '\n' on the last feature column
        std::getline(data, str, '\n');
        feature = atof(str.c_str());
        features.push_back(feature);

        //calculate X and Y
        x = (delta_x/221200)*cos(lat)+lon;
        y = (delta_y/221200)+lat;

        //make x and y into a pair
        std::pair<double,double> xyPair = make_pair(x,y);

        //add the xy pair and features vector into the map
        info.insert(make_pair(xyPair, features));
    }

    data.close();


    return 0;
}