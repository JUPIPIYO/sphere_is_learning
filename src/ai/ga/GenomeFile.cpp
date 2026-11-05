#include "ai/ga/GenomeFile.hpp"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <filesystem>

using namespace std;

namespace sil
{
	namespace
	{
		const string fileHeader = "SIL_GENOME_V1";

		[[noreturn]] void fail(const string& path, const string& reason)
		{
			throw runtime_error("GenomeFile: " + path + ": " + reason);
		}
	}

	string modelText()
	{
		//定数から組み立てるのでcellWidthなどを変えると自動で文字列も変わる
		ostringstream out;
		out << setprecision(numeric_limits<float>::max_digits10);
		out << "neighbors" << neighborCount << "-cell" << cellWidth << "-xOuter(+1,0,-1)-zInner(+1,0,-1)-skipCenter"
			<< ";kinds-Empty-Wall-Obstacle-Goal"
			<< ";priority-Obstacle-Goal-Wall"
			<< ";index-base" << CellKinds << "-cell0Lowest"
			<< ";actions-Stop-Up-Down-Left-Right-UpRight-UpLeft-DownRight-DownLeft"
			<< ";decision" << decisionSteps << "-dt1/60"
			<< ";reward-v1"; //報酬の計算を変えたらv2にする
		return out.str();
	}

	void saveGenome(const string& dir, const string& path, const SavedGenome& saved)
	{
		//遺伝子を文字列に
		string genesLine(saved.genes.size(), '0');
		for (size_t i = 0; i < saved.genes.size(); i++)
		{
			genesLine[i] = static_cast<char>('0' + saved.genes[i]);
		}

		if (!std::filesystem::exists(dir))
		{
			std::filesystem::create_directories(dir);
		}

		std::filesystem::path filePath = dir + '/' + path;

		ofstream out(filePath);
		if (!out) fail(dir + '/' + path, "cannot open for writing");

		const Metrics& m = saved.metrics;
		out << setprecision(numeric_limits<float>::max_digits10);
		out << fileHeader << '\n';
		out << quoted(modelText()) << '\n';
		out << saved.generation << ' ' << saved.seed << '\n';
		out << m.fitness << ' ' << m.bestZ << ' ' << m.surviveSeconds << ' ' << static_cast<int>(m.end) << '\n';
		out << saved.genes.size() << '\n';
		out << genesLine << '\n';

		out.close();
		if (!out) fail(path, "write failed");
	}

	SavedGenome loadGenome(const string& path)
	{
		ifstream in(path);
		string failStr = format("cannot open for reading : {} : On GenomeFile.cpp", path);
		if (!in) fail(path, failStr);

		string header;
		if (!(in >> header) || header != fileHeader) fail(path, "unknown header");

		string model;
		//" "で囲んだ範囲があるか
		if (!(in >> quoted(model))) fail(path, "cannot read model text");

		SavedGenome saved;

		//uint32_tへ直接読むと"-1"が大きな正の数として通るので、一度long longで読む
		long long seed = 0;
		if (!(in >> saved.generation >> seed)) fail(path, "cannot read generation and seed");
		if (seed < 0 || seed > numeric_limits<uint32_t>::max()) fail(path, "seed is out of range");
		saved.seed = static_cast<uint32_t>(seed);

		Metrics& m = saved.metrics;
		int end = 0;
		if (!(in >> m.fitness >> m.bestZ >> m.surviveSeconds >> end)) fail(path, "cannot read metrics");
		m.end = static_cast<GameState>(end);

		size_t count = 0;
		if (!(in >> count) || count != stateSize) fail(path, "gene count does not match stateSize");

		string genesLine;
		if (!(in >> genesLine) || genesLine.size() != stateSize) fail(path, "gene line has wrong length");
		for (size_t i = 0; i < stateSize; i++)
		{
			int action = genesLine[i] - '0';
			saved.genes[i] = static_cast<uint8_t>(action);
		}

		//セーブファイルにまだ残ってないか確かめる
		in >> ws;
		if (!in.eof()) fail(path, "unexpected data after");

		return saved;
	}
}
