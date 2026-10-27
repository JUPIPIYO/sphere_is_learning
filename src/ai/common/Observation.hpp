#pragma once
#include "core/Simulation.hpp"
#include <array>

namespace sil
{

	enum class Cell : uint8_t {
		Empty = 0x00,
		Wall = 0x01,
		Obstacle = 0x02,
		Goal = 0x03
	};

	inline constexpr size_t CellKinds = 4; //�Z���̎��(�Z���ɑ��݂���I�u�W�F�N�g�̎��)�̐�
	inline constexpr size_t neighborCount = 8; //�אڃ}�X�̐� 3x3 - 1
	inline constexpr float cellWidth = 1.0f; //�Z���̑傫��
	inline constexpr int decisionSteps = 4; //�����ύX����t���[�����o

	inline constexpr size_t stateSize = 1 << 16; //���肦��󋵂̑���

	typedef std::array<Cell, neighborCount> Observation; //���͂̏󋵂��i�[����z��


	inline constexpr size_t actionSize = 9;

	//rect�ɂ��Ă��͈̔͂�Cell�󋵂��m�F
	Cell classifyCell(const Stage& stage, const State& state, Rect& rect);

	//Player���͂̏������ۂɌ���observation���쐬
	Observation observe(const Environment& env);

	//Observation�̓��e��0 ~ 65535�̒l�ɕϊ�
	size_t toIndex(const Observation& observation);

}