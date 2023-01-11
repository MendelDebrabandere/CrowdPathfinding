#ifndef SECTOR
#define SECTOR


class Portal;

class Sector final
{
public:
	explicit Sector(const Elite::Vector2& center,
					const std::vector<uint8>& costField,
					std::vector<Sector*>* sectors);

	~Sector();

	// rule of 5 copypasta
	Sector(const Sector& other) = delete;
	Sector(Sector&& other) = delete;
	Sector& operator=(const Sector& other) = delete;
	Sector& operator=(Sector&& other) = delete;


	void Draw(bool drawEdges, bool drawCells, bool showPortals, bool showHeatMap, bool showVectors) const;
	void MakePortals(const std::vector<Sector*>& sectorPtrs);
	bool IsWall(int idx) const;
	Elite::Vector2 GetCenter() const;
	const std::vector<Portal*>* GetPortals() const;

	void SetHeatFieldPoint(const Elite::Vector2& point, int value);
	void SetHeatFieldPoint(int idx, int value);
	int GetHeatFieldValue(int idx) const;

	void GenerateFlowField();
	bool HasGeneratedFlowField() const;

	Elite::Vector2 GetFlowVector(const Elite::Vector2& position) const;

	void Make1Portal(int myIdx, int otherSectorIdx, const Elite::Vector2& startPortalPos, const Elite::Vector2& endPortalPos);


	void ClearData();

private:
	static constexpr uint8 s_Cells{10};
	static constexpr uint8 s_CellSize{1};

	Elite::Vector2 m_Center{};

	std::vector<uint8> m_CostField{};
	std::vector<int> m_HeatField{};
	std::vector<Elite::Vector2> m_FlowField{};

	bool m_HasFlowFieldGenerated{false};

	std::vector<Portal*> m_PortalsPtrs;

	std::vector<RigidBody*> m_RBptrs{};

	std::vector<Sector*>* m_pSectors{};

	void TryToMakePortal(bool& IsMakingPortal, int myIdx, int otherSectorIdx, const Elite::Vector2& startPortalPos, const Elite::Vector2& endPortalPos, const std::vector<Sector*>& sectorPtrs);
	void GenerateHeatField();
	std::vector<int> GetCellNeighbors(int idx);
	void FlowFieldFlowOverToNeighborSector();
	void GenerateVectorBasedOnHeatField(int idx);

	Elite::Vector2 GetCellPos(int idx) const;
}; 

#endif
