#pragma once

//Main 
/*
#include <iostream>
#include "Gobang.h"
using namespace GameFramework;

int main()
{
    GfGobangBoard board;
    board.StartGame(EGfGobangColor::Black, EGfGobangColor::White, 19);

	while (true)
	{
		int x, y;
		std::cout << "Player" << std::endl;
		std::cin >> x >> y;
		board.PutPiece(x, y, board.GetInviterColor());
		board.PrintBoard();
		board.PrintScore(board.GetInviterColor());
		if (board.IsEnd() != EGfGobangColor::None)
		{
			break;
		}

		board.GetAIPutPos(x, y, board.GetInviteeColor());
		std::cout << "AI" << std::endl << x << std::endl << y << std::endl;
		board.PutPiece(x, y, board.GetInviteeColor());
		board.PrintBoard();
		board.PrintScore(board.GetInviteeColor());
		if (board.IsEnd() != EGfGobangColor::None)
		{
			break;
		}
	}
}
*/
namespace GameFramework
{

#ifndef GobangSize
#define GobangSize 19
#endif

	enum class EGfGobangScore : int
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

	enum class EGfGobangColor : int
	{
		None,
		Black,
		White
	};

	class GfGobangBoard
	{
	public:
		GfGobangBoard();
		~GfGobangBoard();

		void ClearBoard();
		int GetPieceCount();
		EGfGobangScore GetScore(int num, bool block1, bool block2);
		EGfGobangScore GetLineMaxScore(int inX, int inY, EGfGobangColor color);
		void GetAIPutPos(int& outX, int& outY, EGfGobangColor color);
		EGfGobangColor GetColorByPos(int inX, int inY);
		EGfGobangColor GetInviterColor();
		EGfGobangColor GetInviteeColor();
		void PutPiece(int inX, int inY, EGfGobangColor color);
		void StartGame(EGfGobangColor inviterColor, EGfGobangColor targetColor, int updateScoreRange);

		EGfGobangColor IsEnd();

		void PrintBoard();

		void PrintScore(EGfGobangColor color);

	private:
		void UpdateBoundary(int x, int y);
		void UpdateScore(int x, int y);

	private:
		EGfGobangColor m_nBoard[GobangSize][GobangSize];
		EGfGobangScore m_nBlackScore[GobangSize][GobangSize];
		EGfGobangScore m_nWhiteScore[GobangSize][GobangSize];
		EGfGobangColor m_eInviterColor;
		EGfGobangColor m_eTInviteeColor;
		int m_nPieceCount;
		int m_nTopBoundary;
		int m_nBottomBoundary;
		int m_nLeftBoundary;
		int m_nRightBoundary;
		int m_nUpdateScoreRange;
	};
}