#pragma once
#include "ai/ga/GAAgent.hpp"
#include "ai/common/Episode.hpp"
#include <random>
#include <vector>

namespace sil
{
  inline constexpr int sizePerEp = 64;
  inline constexpr int elitePerEp = 4;

  struct Individual
  {
    Genome gene{};
    Metrics metrics{};
  };

  std::vector<Individual> initPopulation(std::mt19937& random, int count = sizePerEp);
  void evalPopulation(std::vector<Individual>& population, std::uint32_t seed);
  std::vector<Individual> nextPopulation(const std::vector<Individual>& population, std::mt19937& random, int count = sizePerEp, float mutationRate = 0.01);
}