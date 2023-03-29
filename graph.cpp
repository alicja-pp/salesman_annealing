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
    long step;
    double temperature;
    double total_length;
    vector<int> indices;
    const vector<City> cities;
};

double total_distance(vector<City> cities, vector<int> idx) {
    long double d = 0;

    for (int i = 0; i < idx.size() - 1; i++) {
        d += pow(cities[idx[i + 1]].x - cities[idx[i]].x, 2) +
             pow(cities[idx[i + 1]].y - cities[idx[i]].y, 2);
    }
    d += pow(cities[idx[0]].x - cities[idx.back()].x, 2) +
         pow(cities[idx[0]].y - cities[idx.back()].y, 2);
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

    return State{.step = 0,
                 .temperature = 1,
                 .total_length = total_distance(cities, indices),
                 .indices = indices,
                 .cities = cities};
}

// TODO: update only changed points
void update_graph(TCanvas *canvas, TGraph *points, TGraph *lines,
                  TGraph *lengths, State &state) {
    for (int i = 0; i < state.indices.size(); i++) {
        lines->SetPoint(i, state.cities[state.indices[i]].x,
                        state.cities[state.indices[i]].y);
    }

    lines->SetPoint(state.indices.size(), state.cities[state.indices[0]].x,
                    state.cities[state.indices[0]].y);

    lengths->SetPoint(state.step, state.step, state.total_length);

    canvas->cd(1);
    points->Draw("AP");
    lines->Draw("same");

    canvas->cd(2);
    lengths->Draw();

    canvas->Modified();
    canvas->Update();
    gSystem->ProcessEvents();
}

void graph(void) {
    // czytanie współrzędnych miast z pliku, zapis do wektorów
    State state = init_state("ireland-100.txt");
    int step_number = 10000000;

    srand(time(NULL));
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(state.indices.begin(), state.indices.end(),
            default_random_engine(seed));

    // visualize(state.indices, state.cities);
    TCanvas *canvas = new TCanvas("c", "Path", 1000, 800);
    canvas->Divide(2, 1);

    vector<double> xs = get_x(state.cities);
    vector<double> ys = get_y(state.cities);

    TGraph *points = new TGraph(state.indices.size(), &xs[0], &ys[0]);

    points->SetTitle("Path");
    points->SetMarkerColor(9);
    points->SetMarkerStyle(29);
    points->SetMarkerSize(1);

    TGraph *lines = new TGraph(1, &state.cities[0].x, &state.cities[0].y);

    TGraph *lengths = new TGraph(1, 0, state.total_length);
    lengths->SetTitle("Length");

    update_graph(canvas, points, lines, lengths, state);

    int rand_idx1, rand_idx2;

    while (state.step < step_number) {
        do {
            rand_idx1 = rand() % (state.cities.size());
            rand_idx2 = rand() % (state.cities.size());
        } while (rand_idx1 == rand_idx2);

        // auto [idx1, idx2] = generate_random_indices(state.cities.size() - 1);
        cout << "random_idx1: " << rand_idx1 << ", "
             << "random_idx2: " << rand_idx2 << endl;

        vector<int> new_indices(state.indices);

        swap(new_indices[rand_idx1], new_indices[rand_idx2]);

        double new_length = total_distance(state.cities, new_indices);

        double prob =
            exp(-(new_length - state.total_length) / state.temperature);
        double random = (double)rand() / (RAND_MAX + 1.0);

        cout << "random: " << random << endl;
        cout << "probability: " << prob << endl;

        if (random < prob) {
            state.total_length = new_length;
            state.indices = new_indices;
        }

        cout << "length: " << state.total_length << endl;
        cout << "temperature: " << state.temperature << endl;

        state.temperature *= 0.98;

        update_graph(canvas, points, lines, lengths, state);
        // sleep(1);
        cout << endl;
        state.step++;
    }
}

#ifndef __CINT__
int main() {
    graph();
    return 0;
}
#endif
