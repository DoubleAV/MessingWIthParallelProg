#include <iostream>
#include <math.h>
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>
#include <tuple>
#include <map>

using namespace std;



//First of all I apologize for doing this all in main, if I were to do this again
//I would definitely modularize parts of this program.
int main(int argc, char* argv[1]) {

    using namespace std::chrono;

    //Start timer
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    //ifstream data("hpc_data_starter.csv");

    if (argc != 2){
        cout<<"usage: "<< argv[0] <<" <filename>\n";
    } else {

        ifstream data(argv[1]);



        //Make sure the file opens correctly
        if (!data.is_open()) std::cout << "Error on: File Open" << '\n';

        std::string str;
        std::getline(data, str); //skips the first line so I can get to the actual data


        std::map<std::pair<double, double>, std::vector<double> > info;

        std::map<std::pair<double, double>, std::vector<double> >::iterator itr;

        double x;
        double y;
        double lon;
        double lat;
        double delta_x;
        double delta_y;
        double feature;
        std::vector<double> features;

        double lines_count = 0;

        while (data.good()) {
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
            for (int j = 0; j < 20; j++) {
                std::getline(data, str, ':');
                feature = atof(str.c_str());
                features.push_back(feature);

            }

            //handles the '\n' on the last feature column
            std::getline(data, str, '\n');
            feature = atof(str.c_str());
            features.push_back(feature);

            //calculate X and Y
            x = (delta_x / 221200) * cos(lat) + lon;
            y = (delta_y / 221200) + lat;

            //make x and y into a pair
            std::pair<double, double> xyPair = make_pair(x, y);

            //add the xy pair and features vector into the map
            info.insert(make_pair(xyPair, features));

            lines_count++;
        }
        //Close file
        data.close();

        //Record parse finish time
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        //calculate time to parse
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);


        std::cout << "Time to Parse: " << time_span.count() << " seconds." << "\n";
        std::cout << "Number of Lines Parsed: " + std::to_string(lines_count) << "\n";
        std::cout << "Number of Entries in Map: " + std::to_string(info.size()) << "\n";


        //boundary calculation time begin
        high_resolution_clock::time_point t3 = high_resolution_clock::now();
        double xmax = info.rbegin()->first.first;
        double xmin = info.begin()->first.first;
        double ymax = info.rbegin()->first.second;
        double ymin = info.begin()->first.second;

        //finish time for bounds, calculate difference
        high_resolution_clock::time_point t4 = high_resolution_clock::now();
        duration<double> time_span2 = duration_cast<duration<double>>(t4 - t3);
        std::cout << "Time to Find Bounds: " << time_span2.count() << " seconds." << "\n";

        std::cout << "X Max = " << xmax << " X Min = " << xmin << "\n";
        std::cout << "Y Max = " << ymax << " Y Min = " << ymin << "\n";

    }


    return 0;
}