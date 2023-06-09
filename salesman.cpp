#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

#include "TCanvas.h"
#include "TGraph.h"
#include "TSystem.h"

using namespace std;

struct City {
    double x;
    double y;
};

double total_distance(const vector<City> &cities, const vector<int> &idx) {
    long double d = 0;

    for (int i = 0; i < idx.size() - 1; i++) {
        d += sqrt(pow(cities[idx[i + 1]].x - cities[idx[i]].x, 2) +
                  pow(cities[idx[i + 1]].y - cities[idx[i]].y, 2));
    }
    d += sqrt(pow(cities[idx[0]].x - cities[idx.back()].x, 2) +
              pow(cities[idx[0]].y - cities[idx.back()].y, 2));
    return d;
}

const vector<City> load_cities(const string &filename) {
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

struct State {
    long step;
    double temperature;
    double total_length;
    vector<int> indices;
    const vector<City> cities;

    State(string filename) : cities(load_cities(filename)) {
        vector<int> indices(cities.size());
        iota(begin(indices), end(indices), 0);

        this->step = 0;
        this->temperature = 10000;
        this->total_length = total_distance(cities, indices);
        this->indices = indices;
    }
};

vector<double> get_x(const vector<City> &cities) {
    vector<double> xs(cities.size());
    transform(cities.begin(), cities.end(), xs.begin(),
              [](City const &city) { return city.x; });
    return xs;
}

vector<double> get_y(const vector<City> &cities) {
    vector<double> ys(cities.size());
    transform(cities.begin(), cities.end(), ys.begin(),
              [](City const &city) { return city.y; });
    return ys;
}

void draw_graph(TCanvas *canvas, TGraph *points, TGraph *lines,
                TGraph *lengths, TGraph *temps, const State &state) {
    canvas->cd(1);
    points->Draw("AP");
    lines->Draw("same");

    canvas->cd(2);
    lengths->Draw();

    canvas->cd(3);
    // gPad->SetLogy();  // exponential and logarithmic
    temps->Draw();

    canvas->Print("result.png");
}

const long STEPS = 1e8;

void salesman(void) {
    auto state = State("ireland-30.txt");

    srand(time(NULL));
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();

    shuffle(state.indices.begin(), state.indices.end(),
            default_random_engine(seed));

    TCanvas *canvas = new TCanvas("canvas", "Path", 1200, 800);
    canvas->Divide(3, 1);

    const vector<double> xs = get_x(state.cities);
    const vector<double> ys = get_y(state.cities);

    TGraph *points = new TGraph(state.indices.size(), &xs[0], &ys[0]);

    points->SetTitle("Path; x; y");
    points->SetMarkerColor(9);
    points->SetMarkerStyle(29);
    points->SetMarkerSize(1);

    TGraph *lines = new TGraph(1, &state.cities[0].x, &state.cities[0].y);

    TGraph *lengths = new TGraph(1, 0, &state.total_length);
    lengths->SetTitle("Length; step; length");

    TGraph *temps = new TGraph(1, 0, &state.temperature);
    temps->SetTitle("Temperature; step; temperature");

    cout << "Initial length: " << state.total_length << '\n';
    cout << "Initial temperature: " << state.temperature << '\n';
    cout << '\n';
    int rand_idx1, rand_idx2;

    double start_temperature = state.temperature;  // for logarithmic

    // while (state.temperature > 0) {  // linear
    while (state.step < STEPS) {  // exponential and logarithmic
        do {
            rand_idx1 = rand() % (state.cities.size());
            rand_idx2 = rand() % (state.cities.size());
        } while (rand_idx1 == rand_idx2);

        vector<int> new_indices(state.indices);

        swap(new_indices[rand_idx1], new_indices[rand_idx2]);

        double new_length = total_distance(state.cities, new_indices);

        double prob = (new_length < state.total_length)
                          ? 1.0
                          : exp(-(new_length - state.total_length) /
                                (state.temperature));

        if (prob > (double)rand() / (RAND_MAX + 1.0)) {
            state.total_length = new_length;
            state.indices = new_indices;
        }

        // 3 ways of decreasing temperature
        if (state.step % 10 == 0) {
            // state.temperature *= 0.99;  // exponential
            // state.temperature -= 1;  // linear
            state.temperature =
                start_temperature / (1 + log(state.step));  // logarithmic
        }

        for (int i = 0; i < state.indices.size(); i++) {
            lines->SetPoint(i, state.cities[state.indices[i]].x,
                            state.cities[state.indices[i]].y);
        }

        lines->SetPoint(state.indices.size(), state.cities[state.indices[0]].x,
                        state.cities[state.indices[0]].y);
        lengths->SetPoint(state.step, state.step, state.total_length);
        temps->SetPoint(state.step, state.step, state.temperature);

        state.step++;

        if (state.step % (STEPS / 10) == 0)
            cout << "step: " << state.step << '\n';
    }

    draw_graph(canvas, points, lines, lengths, temps, state);

    cout << '\n';
    cout << "Final length = " << state.total_length << '\n';
    cout << "Final temperature = " << state.temperature << '\n';
}

#ifndef __CINT__
int main() {
    salesman();
    return 0;
}
#endif
