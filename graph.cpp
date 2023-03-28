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

double count_distance(vector<int> n, vector<double> X, vector<double> Y){
    long double d = 0;
    for(int i=1; i<n.size(); i++){
        d=d+sqrt(pow(X.at(n.at(i))-X.at(n.at(i-1)),2)+pow(Y.at(n.at(i))-Y.at(n.at(i-1)),2));
    }
    d=d+sqrt(pow(X.at(n.size()-1)-X.at(0),2)+pow(Y.at(n.size()-1)-Y.at(0),2));
    return d;
}

void graph(){
    //czytanie współrzędnych miast z pliku, zapis do wektorów
    ifstream TownFile;
    TownFile.open("ireland.txt");
    vector<double> x_coo;
    vector<double> y_coo;
    vector<int> numbers;

    double num, x, y;
    int i = 0;
    string coo;
    while(getline(TownFile, coo)){
        stringstream c(coo);
        if (c >> num >> x >> y){
            numbers.push_back(num-1);
            x_coo.push_back(x);
            y_coo.push_back(y);
        }
        i++;
    }
    TownFile.close();
    
    //losujemy kolejność miast
    srand(time(NULL));
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(begin(numbers),end(numbers),default_random_engine(seed));
    
    //odległość, TODO: sprawdzić, czy dobrze to jest liczone
    double distance = count_distance(numbers, x_coo, y_coo);
    
    cout.precision(17);
    cout<<distance<<endl;
    
    
    //wizualizacja drogi między miastami na bieżąco
    TCanvas *c=new TCanvas("c", "Ireland", 1000,800);
    TGraph *g1 = new TGraph(numbers.size(), &x_coo[0], &y_coo[0]);
    g1->SetTitle("Ireland");
    g1->SetMarkerColor(9);
    g1->SetMarkerStyle(29);
    g1->SetMarkerSize(1);

    TGraph *g = new TGraph(1, &x_coo[0], &y_coo[0]);
  
    for(int i=0; i<numbers.size(); i++){
        g->SetPoint(i,x_coo[numbers.at(i)],y_coo[numbers.at(i)]);
        g1->Draw("AP");
        g->Draw("same");
        c->Modified();
        c->Update();
        gSystem->ProcessEvents();
    }
    
}
