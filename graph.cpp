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

struct State {
    double temperature;
    double total_length;
    vector<int> indices;
    vector<City> cities;
};

double total_distance(vector<int> idx, vector<City> cities) {
    long double d = 0;
    for (int i = 1; i < idx.size(); i++){
        d += sqrt(pow(cities[idx[i]].x - cities[idx[i - 1]].x, 2) + pow(cities[idx[i]].y - cities[idx[i - 1]].y, 2));
    }
    d += sqrt(pow(cities[idx.size()-1].x - cities[0].x, 2) + pow(cities[idx.size()-1].y - cities[0].y, 2));
    return d;
}

vector<double> get_x(vector<City> cities){
    vector<double> xs(cities.size());
    transform(cities.begin(), cities.end(), xs.begin(), [](City const& city) { return city.x; });
    return xs;
}

vector<double> get_y(vector<City> cities){
    vector<double> ys(cities.size());
    transform(cities.begin(), cities.end(), ys.begin(), [](City const& city) { return city.y; });
    return ys;
}

void visualize(vector<int> indices, vector<City> cities){
    //wizualizacja drogi między miastami na bieżąco
    TCanvas *c = new TCanvas("c", "Path", 1000, 800);
    vector<double> xs = get_x(cities);
    vector<double> ys = get_y(cities);
    TGraph *g1 = new TGraph(indices.size(), &xs[0], &ys[0]);
    g1->SetTitle("Path");
    g1->SetMarkerColor(9);
    g1->SetMarkerStyle(29);
    g1->SetMarkerSize(1);

    TGraph *g = new TGraph(1, &cities[0].x, &cities[0].y);

    for(int i=0; i<indices.size(); i++){
        g->SetPoint(i, cities[indices.at(i)].x, cities[indices.at(i)].y);
    }
    g->SetPoint(indices.back(), cities[indices.at(0)].x, cities[indices.at(0)].y);
    g1->Draw("AP");
    g->Draw("same");
    c->Modified();
    c->Update();
    gSystem->ProcessEvents();
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
    vector<City> cities = load_cities("ireland-5.txt");

    vector<int> indices(cities.size());
    iota(begin(indices), end(indices), 0);

    State state = State {
        .temperature = 1,
        .total_length = total_distance(indices, cities),
        indices,
        cities
    };
    
    //losujemy kolejność miast
    srand(time(NULL));
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(indices.begin(), indices.end(),  default_random_engine(seed));
    
    //visualize(state.indices, state.cities);
    TCanvas *c = new TCanvas("c", "Path", 1000, 800);
    vector<double> xs = get_x(cities);
    vector<double> ys = get_y(cities);
    TGraph *g1 = new TGraph(indices.size(), &xs[0], &ys[0]);
    g1->SetTitle("Path");
    g1->SetMarkerColor(9);
    g1->SetMarkerStyle(29);
    g1->SetMarkerSize(1);

    TGraph *g = new TGraph(1, &cities[0].x, &cities[0].y);

    for (int i=0; i<indices.size(); i++) {
        g->SetPoint(i, cities[indices.at(i)].x, cities[indices.at(i)].y);
    }
    g->SetPoint(indices.size(), cities[indices.at(0)].x, cities[indices.at(0)].y);
    g1->Draw("AP");
    g->Draw("same");
    c->Modified();
    c->Update();
    gSystem->ProcessEvents();
    
    while (1) {
        shuffle(indices.begin(), indices.end(),  default_random_engine(seed));
        for (int i=0; i<indices.size(); i++) {
            g->SetPoint(i, cities[indices.at(i)].x, cities[indices.at(i)].y);
        }
        g->SetPoint(indices.size(), cities[indices.at(0)].x, cities[indices.at(0)].y);
        g1->Draw("AP");
        g->Draw("same");
        c->Modified();
        c->Update();
        gSystem->ProcessEvents();
        sleep(1);
    }
}
