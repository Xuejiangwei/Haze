#pragma once
#include "../../Common/GfCommon.h"

namespace GameFramework
{

#ifndef GobangSize
#define GobangSize 19
#endif

	enum class EGfGobangScore : int32_t
	{
		Minimum,
		Zero,
		Blocked_One,
		One,
		Blocked_Two,
		Two,
		Blocked_Three,
		Three,
		Blocked_Four,
		Four,
		Five,
	};

	enum class EGfGobangColor;

	class GfGobangBoard
	{
	public:
		GfGobangBoard();
		~GfGobangBoard();

		void ClearBoard();
		int32_t GetPieceCount();
		EGfGobangScore GetScore(int32_t num, bool block1, bool block2);
		EGfGobangScore GetLineMaxScore(int32_t inX, int32_t inY, EGfGobangColor color);
		void GetAIPutPos(int32_t& outX, int32_t& outY, EGfGobangColor color);
		EGfGobangColor GetColorByPos(int32_t inX, int32_t inY);
		EGfGobangColor GetInviterColor();
		EGfGobangColor GetInviteeColor();
		void PutPiece(int32_t inX, int32_t inY, EGfGobangColor color);
		void StartGame(EGfGobangColor inviterColor, EGfGobangColor targetColor, int32_t updateScoreRange);

	private:
		void UpdateBoundary(int32_t x, int32_t y);
		void UpdateScore(int32_t x, int32_t y);

	private:
		EGfGobangColor m_nBoard[GobangSize][GobangSize];
		EGfGobangScore m_nBlackScore[GobangSize][GobangSize];
		EGfGobangScore m_nWhiteScore[GobangSize][GobangSize];
		EGfGobangColor m_eInviterColor;
		EGfGobangColor m_eTInviteeColor;
		int32_t m_nPieceCount;
		int32_t m_nTopBoundary;
		int32_t m_nBottomBoundary;
		int32_t m_nLeftBoundary;
		int32_t m_nRightBoundary;
		int32_t m_nUpdateScoreRange;
	};
}