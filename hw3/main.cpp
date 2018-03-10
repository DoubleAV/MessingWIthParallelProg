#include <iostream>
#include <math.h>
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>
#include <tuple>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#define earthRadiusKm 6371.0

using namespace std;

double deg2rad(double deg);
double rad2deg(double rad);
double haversineDistance(double lat1d, double lon1d, double lat2d, double lon2d);


int main(int argc, char* argv[1]) {

    using namespace std::chrono;
    using namespace boost::numeric::ublas;

    //Start timer
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    if (argc != 2){
        cout<<"usage: "<< argv[0] <<" <filename>\n";
    } else {

        ifstream data(argv[1]);

        //Make sure the file opens correctly
        if (!data.is_open()) std::cout << "Error on: File Open" << '\n';

        std::string str;

        //declare multimap containing argmax and it's corresponding (x,y) pair.
        std::unordered_multimap<int, std::pair<double, double>> arg_mm;


        double x;
        double y;
        double lon;
        double lat;
        double delta_x;
        double delta_y;
        double feature;
        std::vector<double> features;
        int argMax;

        double lines_count = 0;

       // while (data.good()) {
    for (int i = 0; i<20; i++){

            //clears the temporary feature vector at the start of each loop so that each line's features can be added to the map
            //as one vector and the other stuff isn't included from the previous iteration.
            features.clear();
            argMax = 0;

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

            //increment line counter
            lines_count++;

            //gets argmax for each feature vector
            argMax = std::distance(features.begin(), std::max_element(features.begin(), features.end()));

            //calculate X and Y
            x = (delta_x / 221200) * cos(lat) + lon;
            y = (delta_y / 221200) + lat;

            //make x and y into a pair
            std::pair<double, double> xyPair = make_pair(x, y);

            //add argMax and it's corresponding x,y pairs to multimap
            arg_mm.insert(make_pair(argMax, xyPair));
        }
        //Close file
        data.close();

        //Record parse finish time
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        //calculate time to parse
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);



        std::cout << "Time to Parse: " << time_span.count() << " seconds." << "\n";
        std::cout << "Number of Lines Parsed: " + std::to_string(lines_count) << "\n";
        std::cout << "Number of Buckets in Multimap: " + std::to_string(arg_mm.bucket_count()) << "\n";

        //Start time for getting the count of buckets
        high_resolution_clock::time_point bucket_count_start = high_resolution_clock::now();

        std::vector<matrix<double>> matrices;
        matrix<double> h_dist_matrix;

        //build haversine distance matrix for each bucket under 5001 elements
        for (auto& i: {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}) {
            if (arg_mm.count(i) < 5001){
                int size = arg_mm.count(i);
                std::cout << i << ": " << size << " entries.\n";
                h_dist_matrix.resize(size, size, false);
                //equal_range returns a pair of bounds for the items in the container of the 
                //specified key.
                auto j_its = arg_mm.equal_range(i);
                auto k_its = arg_mm.equal_range(i);
                auto j_it = j_its.first;
                auto k_it = k_its.first;

                for (unsigned int j = 0; j < h_dist_matrix.size1(); ++j) {
                    
                    //conditional to check if it's gone through every item in the bucket
                    if (j_it != j_its.second){
                        for (unsigned k = 0; k < h_dist_matrix.size2(); ++k) {

                            if (k_it != k_its.second){
                                h_dist_matrix(j, k) = haversineDistance(j_it->second.first, j_it->second.second,
                                 k_it->second.first, k_it->second.second);
                                 ++k_it;
                            }
                        }
                        ++j_it;
                    }
                }//finish building matrix

                matrices.push_back(h_dist_matrix);
                //std::cout << h_dist_matrix << '\n';
                h_dist_matrix.clear();
            }
        }

        //bucket count end time
        high_resolution_clock::time_point bucket_count_end = high_resolution_clock::now();
        //calculate time to count buckets
        duration<double> count_time = duration_cast<duration<double>>(bucket_count_end - bucket_count_start);
        std::cout << "Bucket Count Time: " << count_time.count() << " seconds." << "\n";
    }

    return 0;
}
//converts a degree value to radians
double deg2rad(double deg) {
    return (deg * M_PI / 180);
}
//converts a radian value to degrees
double rad2deg(double rad) {
    return (rad * 180 / M_PI);
}
//given two sets of latitudes and longitudes, returns the haversine distance between them
double haversineDistance(double lat1d, double lon1d, double lat2d, double lon2d) {
    double lat1r, lon1r, lat2r, lon2r, u, v;
    lat1r = deg2rad(lat1d);
    lon1r = deg2rad(lon1d);
    lat2r = deg2rad(lat2d);
    lon2r = deg2rad(lon2d);
    u = sin((lat2r - lat1r) / 2);
    v = sin((lon2r - lon1r) / 2);

    //divide by 1000 to get meters
    return 2.0 * earthRadiusKm / 1000.0 * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
}