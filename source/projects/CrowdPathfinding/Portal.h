#pragma once
class Portal final
{
public:
	Portal(const Elite::Vector2& pos1, const Elite::Vector2& pos2, int otherSectorIdx);

	void Draw() const;
	std::pair< Elite::Vector2, Elite::Vector2> GetData();

private:
	Elite::Vector2 m_Pos1{};
	Elite::Vector2 m_Pos2{};

	int m_OtherSectorIdx{};

};

