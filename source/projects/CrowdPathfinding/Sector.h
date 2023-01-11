#ifndef SECTOR
#define SECTOR


class Portal;

class Sector final
{
public:
	explicit Sector(const Elite::Vector2& center,
					const std::vector<uint8>& costField);

	~Sector();

	// rule of 5 copypasta
	Sector(const Sector& other) = delete;
	Sector(Sector&& other) = delete;
	Sector& operator=(const Sector& other) = delete;
	Sector& operator=(Sector&& other) = delete;


	void Draw(bool drawEdges, bool drawCells, bool showPortals, bool showHeatMap) const;
	void MakePortals(const std::vector<Sector*>& sectorPtrs);
	bool IsWall(int idx) const;
	Elite::Vector2 GetCenter() const;
	const std::vector<Portal*>* GetPortals() const;

	void SetFlowFieldPoint(const Elite::Vector2& point, int value);

	void GenerateFlowField();

	void Make1Portal(int myIdx, int otherSectorIdx, const Elite::Vector2& startPortalPos, const Elite::Vector2& endPortalPos);
private:
	static constexpr uint8 s_Cells{10};
	static constexpr uint8 s_CellSize{1};

	Elite::Vector2 m_Center{};

	//1 Byte per item, 255 = wall
	std::vector<uint8> m_CostField{};
	//5 Bytes per item, float = total integrated cost, bool = Active waveFront
	std::vector<float> m_HeatField{};
	//1 Byte per item, uint8_t = key for direction lookupTable
	std::vector<uint8> m_FlowField{};

	bool m_HasFlowFieldGenerated{false};

	std::vector<Portal*> m_PortalsPtrs;

	std::vector<RigidBody*> m_RBptrs{};


	void TryToMakePortal(bool& IsMakingPortal, int myIdx, int otherSectorIdx, const Elite::Vector2& startPortalPos, const Elite::Vector2& endPortalPos, const std::vector<Sector*>& sectorPtrs);
	void GenerateIntegrationField();
	std::vector<int> GetCellNeighbors(int idx);
};

#endif
