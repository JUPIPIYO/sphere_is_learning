#include "ai/ga/GATrainer.hpp"
#include "ai/ga/GAAgent.hpp"
#include "ai/ga/GenomeFile.hpp"
#include <algorithm>
#include <charconv>
#include <iostream>
#include <string_view>
#include <system_error>

using namespace std;
using namespace sil;

namespace 
{
    //起動時のオプション
    struct Options 
    {
        std::string mode;
        std::string stage{ "demo" }, output{ "agent" }, input;
        std::uint32_t seed{ 42 };
        int generations{ 20 }, evaluations{ 1 };
    };

    void help() {
        std::cout << "sphere_experiment train|evaluate [options]\n"
            << "  --stage demo          fixed stage (default demo)\n"
            << "  --seed 0..4294967295  environment seed (default 42)\n"
            << "  --generations   last generation (default 20; train only)\n"
            << "  --output PREFIX       save prefix (default agent; train only)\n"
            << "  --input FILE          saved agent (required for evaluate)\n"
            << "  --evaluations 1..100000  trials (default 1; evaluate only)\n"
            ;
    }

    template<class T> T number(string_view text)
    {
        T value{};
        const auto [last, error] = from_chars(text.data(), text.data() + text.size(), value);
        if (error != std::errc{} || last != text.data() + text.size()) throw invalid_argument("invalid integer");
        return value;
    }

    //CLIをパースして実行まで
    Options parse(int argc, char** argv) 
    {
        Options o;
        o.mode = argv[1];
        if (o.mode != "train" && o.mode != "evaluate") throw std::invalid_argument("unknown mode");
        for (int i = 2; i < argc; ++i) {
            const string key = argv[i];
            if (i + 1 >= argc) throw std::invalid_argument("missing option value: " + key);
            const string value = argv[++i];
            if (key == "--stage") o.stage = value;
            else if (key == "--seed") o.seed = number<std::uint32_t>(value);
            else if (key == "--generations" && o.mode == "train") o.generations = number<int>(value);
            else if (key == "--output" && o.mode == "train") o.output = value;
            else if (key == "--input" && o.mode == "evaluate") o.input = value;
            else if (key == "--evaluations" && o.mode == "evaluate") o.evaluations = number<int>(value);
            else throw std::invalid_argument("no option: " + key);
        }
        if (o.stage != "demo" || o.output.empty() || (o.mode == "evaluate" && o.input.empty())) throw std::invalid_argument("option out of range");
        return o;
    }
    void train(const Options& o) 
    {
        mt19937 random(o.seed);
        auto population = initPopulation(random);
        for (int generation = 0; generation <= o.generations; ++generation) 
        {
            //populationを評価
            evalPopulation(population, o.seed);
            //平均を取得
            float mean = 0;
            for (const auto& i : population) mean += i.metrics.fitness;
            mean /= static_cast<float>(population.size());

            //最高点を取得
            const auto& best = population.front();
            //最高到達点を取得
            const auto farthest = std::max_element(population.begin(), population.end(), [](const auto& a, const auto& b) {
                return a.metrics.bestZ < b.metrics.bestZ;
                });
            const auto m = best.metrics;
            cout << generation << ',' << m.fitness << ',' << mean << ',' << farthest->metrics.bestZ
                << ',' << m.surviveSeconds << ',' << StateToStr(m.end) << ',' << static_cast<int>(m.end) << '\n';

            //最初、中間、最後の3回でsave
            if (generation == 0 || generation == o.generations / 2 || generation == o.generations) 
            {
                const std::string prefix = o.output + "_g" + to_string(generation);
                saveGenome(prefix + "_fitness.sil", { best.gene, generation, o.seed,  best.metrics });
                saveGenome(prefix + "_distance.sil", { farthest->gene, generation, o.seed, farthest->metrics });
            }
            if (generation != o.generations) population = nextPopulation(population, random);
        }
    }
    void evaluate(const Options& o) 
    {
        const auto saved = loadGenome(o.input);
        std::cout << "trial,fitness,distance,survival,lateral,end\n";
        GAAgent ga{ saved.genes };
        for (int trial = 0; trial < o.evaluations; ++trial) 
        {
            const Metrics m = evaluate(ga, o.seed);
            std::cout << trial << ',' << m.fitness << ',' << m.bestZ << ',' << m.surviveSeconds << ',' << StateToStr(m.end)
                 << '\n';
        }
    }
}

int main(int argc, char** argv) {
    try {
        const auto options = parse(argc, argv);
        if (options.mode == "train") train(options);
        else evaluate(options);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}