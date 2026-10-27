#pragma once
#include "ai/ga/GAAgent.hpp"
#include "ai/common/Episode.hpp"
#include <cstdint>
#include <string>


namespace sil
{
	//観測と行動の決まりを1つにまとめた文字列。決まりを変えたら古いファイルが読めなくなる
	std::string modelText();

	//保存する内容。ステージは保存しないので、どのステージでも読み込める
	struct SavedGenome
	{
		Genome genes{};
		int generation{};
		std::uint32_t seed{};
		Metrics metrics{};
	};

	//pathへ上書き保存する。失敗したらstd::runtime_errorを投げる
	void saveGenome(const std::string& path, const SavedGenome& saved);

	//pathから読み込む。形式やmodelText()が一致しなければstd::runtime_errorを投げる
	SavedGenome loadGenome(const std::string& path);
}
