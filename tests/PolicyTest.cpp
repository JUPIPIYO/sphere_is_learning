#include "ai/Policy.cpp"
#include <array>
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>

using namespace sil;
using namespace std;

int main()
{
	//常二上に動くgenes
	Genome g;
	g.fill(1);

	Metrics m = evaluate(g, 42);

	cout << "fitness : " << m.fitness << "\n bestZ : " << m.bestZ << "\n surviveTime : " << m.surviveSeconds;
}