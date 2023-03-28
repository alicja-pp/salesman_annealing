#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <random>

using namespace std;

struct City {
    double x;
    double y;
};

double count_distance(vector<int> idx, vector<City> cities) {
    long double d = 0;
    for (int i = 1; i < idx.size(); i++){
        d += sqrt(pow(cities[idx[i]].x) - cities[idx[i - 1]].x), 2) + pow(cities[idx[i]].y - cities[idx[i - 1]].y, 2));
    }
    d += sqrt(pow(cities[idx.end()].x - cities[0], 2) + pow(cities[idx.end()].y - cities[0].y, 2));
    return d;
}

void visualize(vector<int> indices, vector<double> x_coo, vector<double> y_coo){
    //wizualizacja drogi między miastami na bieżąco
    TCanvas *c = new TCanvas("c", "Ireland", 1000, 800);
    TGraph *g1 = new TGraph(indices.size(), &x_coo[0], &y_coo[0]);
    g1->SetTitle("Ireland");
    g1->SetMarkerColor(9);
    g1->SetMarkerStyle(29);
    g1->SetMarkerSize(1);

    TGraph *g = new TGraph(1, &x_coo[0], &y_coo[0]);

    for(int i=0; i<indices.size(); i++){
        g->SetPoint(i,x_coo[indices.at(i)],y_coo[indices.at(i)]);
        g1->Draw("AP");
        g->Draw("same");
        c->Modified();
        c->Update();
        gSystem->ProcessEvents();
    }
}

vector<City> load_cities(string filename){
    ifstream Cities;
    Cities.open(filename);

    vector<City> cities;

    double index, x, y;
    string coo;

    while(getline(Cities, coo)) {
        stringstream line(coo);

        if (line >> index >> x >> y)
            cities.push_back(City {x,y});
    }

    Cities.close();

    return cities;
}

void graph(){
    //czytanie współrzędnych miast z pliku, zapis do wektorów
    vector<City> cities = load_cities("ireland.txt");

    vector<int> indices = iota(cities.begin(), cities.end(), 1);

    //losujemy kolejność miast
    srand(time(NULL));
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(indices.begin(), indices.end(),  default_random_engine(seed));

    //odległość, TODO: sprawdzić, czy dobrze to jest liczone
    double distance = count_distance(indices, cities);

    cout.precision(17);
    cout << distance << endl;

    visualize(indices, x_coo, y_coo);
}
