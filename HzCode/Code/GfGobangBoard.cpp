#include "HomeDecoration/GfGobangBoard.h"
#include "ClientObject/NpcData/GfNpcCuriosityGobangData.h"

namespace GameFramework
{
	GfGobangBoard::GfGobangBoard()
	{
		m_nPieceCount = 0;
		m_nTopBoundary = 0;
		m_nBottomBoundary = 0;
		m_nLeftBoundary = 0;
		m_nRightBoundary = 0;
		m_eInviterColor = EGfGobangColor::None;
		m_eTInviteeColor = EGfGobangColor::None;
	}

	GfGobangBoard::~GfGobangBoard()
	{
	}

	void GfGobangBoard::ClearBoard()
	{
		for (int32_t i = 0; i < GobangSize; i++)
		{
			for (int32_t j = 0; j < GobangSize; j++)
			{
				m_nBoard[i][j] = EGfGobangColor::None;
				m_nBlackScore[i][j] = EGfGobangScore::Minimum;
				m_nWhiteScore[i][j] = EGfGobangScore::Minimum;
			}
		}
	}

	int32_t GfGobangBoard::GetPieceCount()
	{
		return m_nPieceCount;
	}

	//输入连成的数量，及两边是否有阻挡，获得相应的分数
	EGfGobangScore GfGobangBoard::GetScore(int32_t num, bool block1, bool block2)
	{
		if (num >= 5)
		{
			return EGfGobangScore::Five;
		}
		else if (block1 && block2)
		{
			return  EGfGobangScore::Zero;
		}
		else if (num == 4)
		{
			if (!block1 && !block2)
			{
				return EGfGobangScore::Four;
			}
			else if (!(block1 && block2))
			{
				return EGfGobangScore::Blocked_Four;
			}
		}
		else if (num == 3)
		{
			if (!block1 && !block2)
			{
				return EGfGobangScore::Three;
			}
			else if (!(block1 && block2))
			{
				return EGfGobangScore::Blocked_Three;
			}
		}
		else if (num == 2)
		{
			if (!block1 && !block2)
			{
				return EGfGobangScore::Two;
			}
			else if (!(block1 && block2))
			{
				return EGfGobangScore::Blocked_Two;
			}
		}
		else if (num == 1)
		{
			if (!block1 && !block2)
			{
				return EGfGobangScore::One;
			}
			else if (!(block1 && block2))
			{
				return EGfGobangScore::Blocked_One;
			}
		}

		return EGfGobangScore::Zero;
	}

	EGfGobangScore GfGobangBoard::GetLineMaxScore(int32_t inX, int32_t inY, EGfGobangColor color)
	{
		if (color == EGfGobangColor::None)
		{
			return EGfGobangScore::Zero;
		}

		int32_t len = GobangSize;
		int32_t colCount = 1;
		bool block1 = false;
		bool block2 = false;

		//判断纵向连成的数量及是否有阻挡
		for (int32_t i = inY + 1; true; i++)
		{
			if (i >= len)
			{
				block1 = true;
				break;
			}

			EGfGobangColor t = m_nBoard[inX][i];

			if (t != color)
			{
				if (t != EGfGobangColor::None)
				{
					block1 = true;
				}
				break;
			}

			colCount++;
		}


		for (int32_t i = inY - 1; true; i--)
		{
			if (i < 0)
			{
				block2 = true;
				break;
			}

			EGfGobangColor t = m_nBoard[inX][i];

			if (t != color)
			{
				if (t != EGfGobangColor::None)
				{
					block2 = true;
				}
				break;
			}

			colCount++;
		}

		EGfGobangScore colScore = GetScore(colCount, block1, block2);

		if ((int32_t)colScore >= (int32_t)EGfGobangScore::Five)
		{
			return colScore;
		}

		int32_t rowCount = 1;
		block1 = false;
		block2 = false;

		//判断横向连成的数量及是否有阻挡
		for (int32_t i = inX + 1; true; i++)
		{
			if (i >= len)
			{
				block1 = true;
				break;
			}

			EGfGobangColor  t = m_nBoard[i][inY];
			if (t != color)
			{
				if (t != EGfGobangColor::None)
				{
					block1 = true;
				}
				break;
			}

			rowCount++;
		}

		for (int32_t i = inX - 1; true; i--)
		{
			if (i < 0)
			{
				block2 = true;
				break;
			}

			EGfGobangColor t = m_nBoard[i][inY];
			if (t != color)
			{
				if (t != EGfGobangColor::None)
				{
					block2 = true;
				}
				break;
			}

			rowCount++;
		}

		EGfGobangScore rowScore = GetScore(rowCount, block1, block2);

		if ((int32_t)rowScore >= (int32_t)EGfGobangScore::Five)
		{
			return rowScore;
		}

		int32_t obliqueCount1 = 1;
		block1 = false;
		block2 = false;

		//判断反斜向连成的数量及是否有阻挡
		for (int32_t i = 1; true; i++)
		{
			int32_t tx = inX + i, ty = inY + i;
			if (tx >= len || ty >= len)
			{
				block1 = true;
				break;
			}

			EGfGobangColor t = m_nBoard[tx][ty];

			if (t != color)
			{
				if (t != EGfGobangColor::None)
				{
					block1 = true;
				}
				break;
			}

			obliqueCount1++;
		}

		for (int32_t i = 1; true; i++)
		{
			int32_t tx = inX - i, ty = inY - i;
			if (tx < 0 || ty < 0)
			{
				block2 = true;
				break;
			}

			EGfGobangColor t = m_nBoard[tx][ty];

			if (t != color)
			{
				if (t != EGfGobangColor::None)
				{
					block2 = true;
				}
				break;
			}

			obliqueCount1++;
		}

		EGfGobangScore obliqueScore1 = GetScore(obliqueCount1, block1, block2);

		if ((int32_t)obliqueScore1 >= (int32_t)EGfGobangScore::Five)
		{
			return obliqueScore1;
		}

		int32_t obliqueCount2 = 1;
		block1 = false;
		block2 = false;

		//判断正斜向连成的数量及是否有阻挡
		for (int32_t i = 1; true; i++)
		{
			int32_t tx = inX + i, ty = inY - i;
			if (tx < 0 || ty < 0 || tx >= len || ty >= len)
			{
				block1 = true;
				break;
			}

			EGfGobangColor t = m_nBoard[tx][ty];

			if (t != color)
			{
				if (t != EGfGobangColor::None)
				{
					block1 = true;
				}
				break;
			}

			obliqueCount2++;
		}

		for (int32_t i = 1; true; i++)
		{
			int32_t tx = inX - i, ty = inY + i;
			if (tx < 0 || ty < 0 || tx >= len || ty >= len)
			{
				block2 = true;
				break;
			}

			EGfGobangColor t = m_nBoard[tx][ty];

			if (t != color)
			{
				if (t != EGfGobangColor::None)
				{
					block2 = true;
				}
				break;
			}

			obliqueCount2++;
		}

		EGfGobangScore obliqueScore2 = GetScore(obliqueCount2, block1, block2);

		if ((int32_t)obliqueScore2 >= (int32_t)EGfGobangScore::Five)
		{
			return obliqueScore2;
		}

		return (EGfGobangScore)std::max(std::max((int32_t)colScore, (int32_t)rowScore), std::max((int32_t)obliqueScore1, (int32_t)obliqueScore2));
	}

	void GfGobangBoard::GetAIPutPos(int32_t& outX, int32_t& outY, EGfGobangColor color)
	{
		if (m_nPieceCount < 2)
		{
			int32_t x = GobangSize / 2;
			int32_t y = x;
			if (m_nBoard[x][y] == EGfGobangColor::None)
			{
				outX = x;
				outY = y;
			}
			else
			{
				int32_t temp = GobangSize / 4;
				x += ((GfHelper::GfRandom::Next(0, 2) % 2 == 0 ? -1 : 1)* temp);
				y += ((GfHelper::GfRandom::Next(0, 2) % 2 == 0 ? -1 : 1)* temp);

				outX = x;
				outY = y;
			}

			return;
		}

		EGfGobangScore(*pScore)[GobangSize] = (color == EGfGobangColor::Black) ? m_nBlackScore : m_nWhiteScore;
		//获得得分最大的点与分数
		EGfGobangScore aiScore = EGfGobangScore::Minimum;
		for (int32_t i = m_nLeftBoundary; i < m_nRightBoundary; i++)
		{
			for (int32_t j = m_nTopBoundary; j < m_nBottomBoundary; j++)
			{
				if (m_nBoard[i][j] == EGfGobangColor::None)
				{
					EGfGobangScore tempScore = pScore[i][j];
					if (tempScore == EGfGobangScore::Minimum)
					{
						tempScore = GetLineMaxScore(i, j, color);
					}

					if ((int32_t)tempScore > (int32_t)aiScore)
					{
						aiScore = tempScore;
						outX = i;
						outY = j;

						if ((int32_t)aiScore >= (int32_t)EGfGobangScore::Five)
						{
							return;
						}
					}
				}
			}
		}

		EGfGobangColor opponentColor = (color == EGfGobangColor::Black) ? EGfGobangColor::White:EGfGobangColor::Black;
		pScore = (opponentColor == EGfGobangColor::Black) ? m_nBlackScore : m_nWhiteScore;
		//获得玩家得分最大的点与分数
		EGfGobangScore playerScore = EGfGobangScore::Minimum;
		for (int32_t i = m_nLeftBoundary; i < m_nRightBoundary; i++)
		{
			for (int32_t j = m_nTopBoundary; j < m_nBottomBoundary; j++)
			{
				if (m_nBoard[i][j] == EGfGobangColor::None)
				{
					EGfGobangScore tempScore = pScore[i][j];
					if (tempScore == EGfGobangScore::Minimum)
					{
						tempScore = GetLineMaxScore(i, j, opponentColor);
					}

					if ((int32_t)tempScore > (int32_t)playerScore)
					{
						playerScore = tempScore;
						if ((int32_t)playerScore > (int32_t)aiScore)
						{
							//如果玩家得分高于ai最高分，选则占用此点
							outX = i;
							outY = j;
							if ((int32_t)playerScore >= (int32_t)EGfGobangScore::Five)
							{
								//如果玩家得分大于等于连成五子的得分，选则占用此点，并return
								return;
							}
						}
					}
				}
			}
		}
	}

	EGfGobangColor GfGobangBoard::GetColorByPos(int32_t inX, int32_t inY)
	{
		if (inX < GobangSize && inY < GobangSize)
		{
			return m_nBoard[inX][inY];
		}

		return EGfGobangColor::None;
	}

	EGfGobangColor GfGobangBoard::GetInviterColor()
	{
		return m_eInviterColor;
	}

	EGfGobangColor GfGobangBoard::GetInviteeColor()
	{
		return m_eTInviteeColor;
	}

	void GfGobangBoard::PutPiece(int32_t inX, int32_t inY, EGfGobangColor color)
	{
		if (inX < GobangSize && inY < GobangSize)
		{
			m_nPieceCount++;
			m_nBoard[inX][inY] = color;
			UpdateScore(inX, inY);
			UpdateBoundary(inX, inY);
		}
	}

	void GfGobangBoard::StartGame(EGfGobangColor inviterColor, EGfGobangColor targetColor, int32_t updateScoreRange)
	{
		m_nPieceCount = 0;
		m_nTopBoundary = GobangSize / 2;
		m_nBottomBoundary = m_nTopBoundary;
		m_nLeftBoundary = m_nTopBoundary;
		m_nRightBoundary = m_nTopBoundary;
		m_eInviterColor = inviterColor;
		m_eTInviteeColor = targetColor;
		m_nUpdateScoreRange = updateScoreRange;
		ClearBoard();
	}

	void GfGobangBoard::UpdateBoundary(int32_t x, int32_t y)
	{
		if (x <= m_nTopBoundary)
		{
			m_nTopBoundary = x;
			if (m_nTopBoundary >= 1)
			{
				m_nTopBoundary--;
			}
		}

		if (x >= m_nBottomBoundary)
		{
			m_nBottomBoundary = x;
			if (m_nBottomBoundary < GobangSize - 1)
			{
				m_nBottomBoundary++;
			}
		}

		if (y <= m_nLeftBoundary)
		{
			m_nLeftBoundary = y;
			if (m_nLeftBoundary >= 1)
			{
				m_nLeftBoundary--;
			}
		}

		if (y >= m_nRightBoundary)
		{
			m_nRightBoundary = y;
			if (m_nRightBoundary < GobangSize - 1)
			{
				m_nRightBoundary++;
			}
		}
	}

	void GfGobangBoard::UpdateScore(int32_t x, int32_t y)
	{
		int32_t minRangeX = x - m_nUpdateScoreRange >= 0 ? x - m_nUpdateScoreRange : 0;
		int32_t maxRangeX = x + m_nUpdateScoreRange < GobangSize ? y + m_nUpdateScoreRange : GobangSize;
		int32_t minRangeY = y - m_nUpdateScoreRange >= 0 ? y - m_nUpdateScoreRange : 0;
		int32_t maxRangeY = y + m_nUpdateScoreRange < GobangSize ? y + m_nUpdateScoreRange : GobangSize;

		for (int32_t i = minRangeX; i < maxRangeX; i++)
		{
			for (int32_t j = minRangeY; j < maxRangeY; j++)
			{
				if (m_nBoard[i][j] == EGfGobangColor::None)
				{
					m_nBlackScore[i][j] = GetLineMaxScore(i, j, EGfGobangColor::Black);
					m_nWhiteScore[i][j] = GetLineMaxScore(i, j, EGfGobangColor::White);
				}
			}
		}
	}
}