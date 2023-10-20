#include "Gobang.h"
#include <xutility>
#include <random>
#include <iostream>

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
		for (int i = 0; i < GobangSize; i++)
		{
			for (int j = 0; j < GobangSize; j++)
			{
				m_nBoard[i][j] = EGfGobangColor::None;
				m_nBlackScore[i][j] = EGfGobangScore::Minimum;
				m_nWhiteScore[i][j] = EGfGobangScore::Minimum;
			}
		}
	}

	int GfGobangBoard::GetPieceCount()
	{
		return m_nPieceCount;
	}

	//�������ɵ��������������Ƿ����赲�������Ӧ�ķ���
	EGfGobangScore GfGobangBoard::GetScore(int num, bool block1, bool block2)
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

	EGfGobangScore GfGobangBoard::GetLineMaxScore(int inX, int inY, EGfGobangColor color)
	{
		if (color == EGfGobangColor::None)
		{
			return EGfGobangScore::Zero;
		}

		int len = GobangSize;
		int colCount = 1;
		bool block1 = false;
		bool block2 = false;

		//�ж��������ɵ��������Ƿ����赲
		for (int i = inY + 1; true; i++)
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


		for (int i = inY - 1; true; i--)
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

		if ((int)colScore >= (int)EGfGobangScore::Five)
		{
			return colScore;
		}

		int rowCount = 1;
		block1 = false;
		block2 = false;

		//�жϺ������ɵ��������Ƿ����赲
		for (int i = inX + 1; true; i++)
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

		for (int i = inX - 1; true; i--)
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

		if ((int)rowScore >= (int)EGfGobangScore::Five)
		{
			return rowScore;
		}

		int obliqueCount1 = 1;
		block1 = false;
		block2 = false;

		//�жϷ�б�����ɵ��������Ƿ����赲
		for (int i = 1; true; i++)
		{
			int tx = inX + i, ty = inY + i;
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

		for (int i = 1; true; i++)
		{
			int tx = inX - i, ty = inY - i;
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

		if ((int)obliqueScore1 >= (int)EGfGobangScore::Five)
		{
			return obliqueScore1;
		}

		int obliqueCount2 = 1;
		block1 = false;
		block2 = false;

		//�ж���б�����ɵ��������Ƿ����赲
		for (int i = 1; true; i++)
		{
			int tx = inX + i, ty = inY - i;
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

		for (int i = 1; true; i++)
		{
			int tx = inX - i, ty = inY + i;
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

		if ((int)obliqueScore2 >= (int)EGfGobangScore::Five)
		{
			return obliqueScore2;
		}

		return (EGfGobangScore)std::max(std::max((int)colScore, (int)rowScore), std::max((int)obliqueScore1, (int)obliqueScore2));
	}

	void GfGobangBoard::GetAIPutPos(int& outX, int& outY, EGfGobangColor color)
	{
		if (m_nPieceCount < 2)
		{
			int x = GobangSize / 2;
			int y = x;
			if (m_nBoard[x][y] == EGfGobangColor::None)
			{
				outX = x;
				outY = y;
			}
			else
			{
				int temp = GobangSize / 4;
				x += ((std::rand() % 2 == 0 ? -1 : 1) * temp);
				y += ((std::rand() % 2 == 0 ? -1 : 1) * temp);

				outX = x;
				outY = y;
			}

			return;
		}

		EGfGobangScore(*pScore)[GobangSize] = (color == EGfGobangColor::Black) ? m_nBlackScore : m_nWhiteScore;
		//��õ÷����ĵ������
		EGfGobangScore aiScore = EGfGobangScore::Minimum;
		for (int i = m_nLeftBoundary; i <= m_nRightBoundary; i++)
		{
			for (int j = m_nTopBoundary; j <= m_nBottomBoundary; j++)
			{
				if (m_nBoard[i][j] == EGfGobangColor::None)
				{
					EGfGobangScore tempScore = pScore[i][j];
					if (tempScore == EGfGobangScore::Minimum)
					{
						tempScore = GetLineMaxScore(i, j, color);
					}

					if ((int)tempScore > (int)aiScore)
					{
						aiScore = tempScore;
						outX = i;
						outY = j;

						if ((int)aiScore >= (int)EGfGobangScore::Five)
						{
							return;
						}
					}
				}
			}
		}
		std::cout << "AI score " << (int)aiScore << std::endl;
		EGfGobangColor opponentColor = (color == EGfGobangColor::Black) ? EGfGobangColor::White : EGfGobangColor::Black;
		pScore = (opponentColor == EGfGobangColor::Black) ? m_nBlackScore : m_nWhiteScore;
		//�����ҵ÷����ĵ������
		EGfGobangScore playerScore = EGfGobangScore::Minimum;
		for (int i = m_nLeftBoundary; i <= m_nRightBoundary; i++)
		{
			for (int j = m_nTopBoundary; j <= m_nBottomBoundary; j++)
			{
				if (m_nBoard[i][j] == EGfGobangColor::None)
				{
					EGfGobangScore tempScore = pScore[i][j];
					if (tempScore == EGfGobangScore::Minimum)
					{
						tempScore = GetLineMaxScore(i, j, opponentColor);
					}
					std::cout << "player smulation score " << (int)tempScore << " " << (int)playerScore
						<< " " << i << " " << j << std::endl;
					if ((int)tempScore > (int)playerScore)
					{
						playerScore = tempScore;
						if ((int)playerScore > (int)aiScore)
						{
							//�����ҵ÷ָ���ai��߷֣�ѡ��ռ�ô˵�
							outX = i;
							outY = j;
							if ((int)playerScore >= (int)EGfGobangScore::Five)
							{
								//�����ҵ÷ִ��ڵ����������ӵĵ÷֣�ѡ��ռ�ô˵㣬��return
								return;
							}
						}
					}
				}
			}
		}
	}

	EGfGobangColor GfGobangBoard::GetColorByPos(int inX, int inY)
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

	void GfGobangBoard::PutPiece(int inX, int inY, EGfGobangColor color)
	{
		if (inX < GobangSize && inY < GobangSize)
		{
			m_nPieceCount++;
			m_nBoard[inX][inY] = color;
			UpdateScore(inX, inY);
			UpdateBoundary(inX, inY);
		}
	}

	void GfGobangBoard::StartGame(EGfGobangColor inviterColor, EGfGobangColor targetColor, int updateScoreRange)
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

	EGfGobangColor GfGobangBoard::IsEnd()
	{
		for (size_t i = 0; i < GobangSize; i++)
		{
			for (size_t j = 0; j < GobangSize; j++) 
			{
				if (m_nBlackScore[i][j] > EGfGobangScore::Five)
				{
					return EGfGobangColor::Black;
				}
			}
		}

		for (size_t i = 0; i < GobangSize; i++)
		{
			for (size_t j = 0; j < GobangSize; j++)
			{
				if (m_nWhiteScore[i][j] > EGfGobangScore::Five)
				{
					return EGfGobangColor::White;
				}
			}
		}

		return EGfGobangColor::None;
	}

	void GfGobangBoard::PrintBoard()
	{
		std::cout << "Print board" << std::endl<<"    ";

		for (size_t i = 0; i < GobangSize; i++)
		{
			if (i < 10)
			{
				std::cout << i << "  ";
			}
			else
			{
				std::cout << i << " ";
			}
		}
		std::cout << std::endl;

		for(size_t i = 0; i < GobangSize; i++)
		{
			if (i < 10)
			{
				std::cout << i << "  ";
			}
			else
			{
				std::cout << i << " ";
			}
			for (size_t j = 0; j < GobangSize; j++)
			{
				if (m_nBoard[j][i] == EGfGobangColor::None)
				{
					std::cout << "  X";
				}
				else if (m_nBoard[j][i] == EGfGobangColor::Black)
				{
					std::cout << "  B";
				}
				else if (m_nBoard[j][i] == EGfGobangColor::White)
				{
					std::cout << "  W";
				}
			}
			std::cout << std::endl;
		}
	}

	void GfGobangBoard::PrintScore(EGfGobangColor color)
	{
		std::cout << "Print Score "<< (int)color << std::endl << "    ";
		for (size_t i = 0; i < GobangSize; i++)
		{
			if (i < 10)
			{
				std::cout << i << "  ";
			}
			else
			{
				std::cout << i << " ";
			}
		}
		std::cout << std::endl;
		EGfGobangScore(*pScore)[GobangSize] = (color == EGfGobangColor::Black) ? m_nBlackScore : m_nWhiteScore;
		for (size_t i = 0; i < GobangSize; i++)
		{
			if (i < 10)
			{
				std::cout << i << "  ";
			}
			else
			{
				std::cout << i << " ";
			}
			for (size_t j = 0; j < GobangSize; j++)
			{
				std::cout << "  " << (int)pScore[j][i];
			}
			std::cout << std::endl;
		}
	}

	void GfGobangBoard::UpdateBoundary(int x, int y)
	{
		std::swap(x, y);
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

	void GfGobangBoard::UpdateScore(int x, int y)
	{
		int minRangeX = x - m_nUpdateScoreRange >= 0 ? x - m_nUpdateScoreRange : 0;
		int maxRangeX = x + m_nUpdateScoreRange < GobangSize ? y + m_nUpdateScoreRange : GobangSize;
		int minRangeY = y - m_nUpdateScoreRange >= 0 ? y - m_nUpdateScoreRange : 0;
		int maxRangeY = y + m_nUpdateScoreRange < GobangSize ? y + m_nUpdateScoreRange : GobangSize;

		for (int i = minRangeX; i < maxRangeX; i++)
		{
			for (int j = minRangeY; j < maxRangeY; j++)
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