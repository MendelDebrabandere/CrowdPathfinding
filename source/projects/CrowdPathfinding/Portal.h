#pragma once
class Portal final
{
public:
	Portal(const Elite::Vector2& pos1, const Elite::Vector2& pos2, int fromSectorIdx, int otherSectorIdx);

	void Draw() const;
	std::pair< Elite::Vector2, Elite::Vector2> GetData();
	int GetTo() const;
	int GetFrom() const;

private:
	Elite::Vector2 m_Pos1{};
	Elite::Vector2 m_Pos2{};

	int m_OtherSectorIdx{};
	int m_FromSectorIdx{};

};

