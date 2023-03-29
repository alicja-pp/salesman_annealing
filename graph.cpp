#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <stdio.h>
#include <vector>

#include "TSystem.h"
#include "TCanvas.h"
#include "TGraph.h"

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
    for (int i = 0; i < idx.size(); i++) {
        d += sqrt(pow(cities[idx[i + 1]].x - cities[idx[i]].x, 2) +
                  pow(cities[idx[i + 1]].y - cities[idx[i]].y, 2));
    }
    d += sqrt(pow(cities[idx[0]].x - cities[idx.back()].x, 2) +
              pow(cities[idx[0]].y - cities[idx.back()].y, 2));
    return d;
}

vector<double> get_x(vector<City> cities) {
    vector<double> xs(cities.size());
    transform(cities.begin(), cities.end(), xs.begin(),
              [](City const &city) { return city.x; });
    return xs;
}

vector<double> get_y(vector<City> cities) {
    vector<double> ys(cities.size());
    transform(cities.begin(), cities.end(), ys.begin(),
              [](City const &city) { return city.y; });
    return ys;
}

vector<City> load_cities(string filename) {
    ifstream Cities;
    Cities.open(filename);

    vector<City> cities;

    double index, x, y;
    string coo;

    while (getline(Cities, coo)) {
        stringstream line(coo);

        if (line >> index >> x >> y) cities.push_back(City{x, y});
    }

    Cities.close();

    return cities;
}

tuple<int, int> generate_random_indices(int max) {
    int rand_idx1, rand_idx2;

    do {
        rand_idx1 = rand() % (max + 1);
        rand_idx2 = rand() % (max + 1);
    } while (rand_idx1 == rand_idx2);

    return tuple(rand_idx1, rand_idx2);
}

State init_state(string filename) {
    vector<City> cities = load_cities(filename);

    vector<int> indices(cities.size());
    iota(begin(indices), end(indices), 0);

    return State{.temperature = 1,
                 .total_length = total_distance(indices, cities),
                 .indices = indices,
                 .cities = cities};
}

// TODO: update only changed points
void update_graph(TCanvas *canvas, TGraph *points, TGraph *lines,
                  State &state) {
    for (int i = 0; i < state.indices.size(); i++) {
        lines->SetPoint(i, state.cities[state.indices[i]].x,
                        state.cities[state.indices[i]].y);
    }

    lines->SetPoint(state.indices.size(), state.cities[state.indices[0]].x,
                    state.cities[state.indices[0]].y);

    points->Draw("AP");
    lines->Draw("same");
    canvas->Modified();
    canvas->Update();
    gSystem->ProcessEvents();
}

void graph(void) {
    // czytanie współrzędnych miast z pliku, zapis do wektorów
    State state = init_state("ireland-5.txt");

    srand(time(NULL));
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(state.indices.begin(), state.indices.end(),
            default_random_engine(seed));

    // visualize(state.indices, state.cities);
    TCanvas *canvas = new TCanvas("c", "Path", 1000, 800);

    vector<double> xs = get_x(state.cities);
    vector<double> ys = get_y(state.cities);

    TGraph *points = new TGraph(state.indices.size(), &xs[0], &ys[0]);

    points->SetTitle("Path");
    points->SetMarkerColor(9);
    points->SetMarkerStyle(29);
    points->SetMarkerSize(1);

    TGraph *lines = new TGraph(1, &state.cities[0].x, &state.cities[0].y);

    update_graph(canvas, points, lines, state);

    while (1) {
        auto [idx1, idx2] = generate_random_indices(state.cities.size() - 1);

        for (int i = 0; i < state.indices.size(); i++) {
            cout << state.indices[i] << " ";
        }
        cout << endl;

        swap(state.indices[idx1], state.indices[idx2]);

        update_graph(canvas, points, lines, state);
        sleep(1);
    }
}

#ifndef __CINT__
int main() {
    graph();
    return 0;
}
#endif
