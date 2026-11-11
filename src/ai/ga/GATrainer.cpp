#include "ai/ga/GATrainer.hpp"
#include <algorithm>

using namespace std;
namespace sil
{
	vector<Individual> initPopulation(mt19937& random, int count)
	{
		uniform_int_distribution<int> dist(0, actionSize - 1);

		vector<Individual> pop;
		for (int i = 0; i < sizePerEp; i++)
		{
			Genome g;
			for (int j = 0; j < stateSize; j++)
			{
				g[j] = static_cast<uint32_t>(dist(random));
			}
			Metrics m{};
			pop.push_back({ g, m });
		}

		return pop;
	}

	void evalPopulation(std::vector<Individual>& population, std::uint32_t seed, int stageIdx)
	{
		for (int i = 0; i < population.size(); i++)
		{
			Individual& indi = population[i];

			GAAgent ga{ indi.gene };

			indi.metrics = evaluate(ga, seed, stageIdx);

		}

		stable_sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
			return a.metrics.fitness > b.metrics.fitness;
			});
	}

	vector<Individual> nextPopulation(const std::vector<Individual>& population, std::mt19937& random, int count, float mutationRate)
	{
		//eliteをまずコピー
		vector<Individual> newPop;
		for (int i = 0; i < elitePerEp; i++)
		{
			newPop.push_back(population[i]);
		}

		uniform_int_distribution<int> act(0, actionSize - 1);
		//ランダムに一体選ぶための乱数
		uniform_int_distribution<int> dist(0, sizePerEp - 1);
		//親のどっちを選ぶかの乱数をここで作っとく
		bernoulli_distribution bel(0.5);
		//突然変異用の乱数も作る
		bernoulli_distribution mut(mutationRate);

		while (newPop.size() < sizePerEp)
		{
			Genome g;

			//親の選択：適当に3体選んでその中からbest
			//populationは降順になっているのでminのindexでOK
			const Individual& father = population[min(min(dist(random), dist(random)), dist(random))];
			const Individual& mother = population[min(min(dist(random), dist(random)), dist(random))];

			for (int i = 0; i < stateSize; i++)
			{
				if (bel(random)) g[i] = father.gene[i];
				else g[i] = mother.gene[i];

				//突然変異
				if (mut(random))
				{
					g[i] = static_cast<uint32_t>(act(random));
				}
			}

			Metrics m{};

			newPop.push_back({ g, m });
		}

		return newPop;
	}
}

