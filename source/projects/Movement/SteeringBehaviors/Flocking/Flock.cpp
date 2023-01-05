#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"
#include "../SpacePartitioning/SpacePartitioning.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/,
	float worldSize /*= 100.f*/,
	SteeringAgent* pAgentToEvade /*= nullptr*/,
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld{ trimWorld }
	, m_pAgentToEvade{ pAgentToEvade }
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{ 0 }
{
	m_Agents.resize(m_FlockSize);



	m_pSeekBehavior = new Seek();
	m_pSeparationBehavior = new Separation(this);
	m_pCohesionBehavior = new Cohesion(this);
	m_pVelMatchBehavior = new VelocityMatch(this);
	m_pWanderBehavior = new Wander();

	m_pBlendedSteering = new BlendedSteering({ {m_pSeekBehavior, 0.0f},
												{m_pSeparationBehavior, 0.0f},
												{m_pCohesionBehavior, 0.0f},
												{m_pVelMatchBehavior, 0.0f},
												{m_pWanderBehavior, 0.0f} });

	m_pEvadeBehavior = new Evade();
	m_pEvadeBehavior->SetEvadeRadius(m_NeighborhoodRadius);

	m_pPrioritySteering = new PrioritySteering({ m_pEvadeBehavior, m_pBlendedSteering });

	for (int i{}; i < flockSize; i++)
	{
		m_Agents[i] = new SteeringAgent();
		m_Agents[i]->SetPosition(Elite::Vector2(float(rand() % 100), float(rand() % 100)));
		m_Agents[i]->SetSteeringBehavior(m_pPrioritySteering);
		m_Agents[i]->SetMaxLinearSpeed(15.f);
		m_Agents[i]->SetAutoOrient(true);
		m_Agents[i]->SetMass(0.3f);
		m_Agents[i]->SetBodyColor(Color{ 1, 1, 1 });
	}
	
	m_pAgentToEvade = new SteeringAgent();
	m_pAgentToEvade->SetSteeringBehavior(m_pWanderBehavior);
	m_pAgentToEvade->SetMaxLinearSpeed(30.f);
	m_pAgentToEvade->SetAutoOrient(true);
	m_pAgentToEvade->SetMass(0.1f);
	m_pAgentToEvade->SetBodyColor(Color{ 1,0,0 });

	m_pCellSpace = new CellSpace(worldSize, worldSize, 15, 15, m_FlockSize);

	for (auto pAgent : m_Agents)
	{
		m_pCellSpace->AddAgent(pAgent);
	}

	m_OldPositions.resize(m_Agents.size());
	for (UINT idx{}; idx < m_Agents.size(); ++idx)
	{
		m_OldPositions[idx] = m_Agents[idx]->GetPosition();
	}

}

Flock::~Flock()
{

	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pSeparationBehavior);
	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pVelMatchBehavior);
	SAFE_DELETE(m_pWanderBehavior);
	SAFE_DELETE(m_pEvadeBehavior)

	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);

	for(auto pAgent: m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	m_Agents.clear();

	SAFE_DELETE(m_pAgentToEvade);

	SAFE_DELETE(m_pCellSpace);
}

void Flock::Update(float deltaT)
{
	// loop over all the agents
		// register its neighbors	(-> memory pool is filled with neighbors of the currently evaluated agent)
		// update it				(-> the behaviors can use the neighbors stored in the pool, next iteration they will be the next agent's neighbors)
		// trim it to the world

	m_pEvadeBehavior->SetTarget(m_pAgentToEvade->GetPosition());
	m_pAgentToEvade->Update(deltaT);

	for (BlendedSteering::WeightedBehavior weightedBehavior : m_pBlendedSteering->GetWeightedBehaviorsRef())
	{
		if (weightedBehavior.weight == 0.f)
			weightedBehavior.pBehavior->SetDebugRender(false);
		else
			weightedBehavior.pBehavior->SetDebugRender(m_DebugRendering);
	}


	if (m_UsingSpacePartitioning)
	{
		for (UINT idx{}; idx < m_Agents.size(); ++idx)
		{
			m_pCellSpace->UpdateAgentCell(m_Agents[idx], m_OldPositions[idx]);
			m_pCellSpace->RegisterNeighbors(m_Agents[idx], m_NeighborhoodRadius, m_DebugSpacePartitioning);

			m_Agents[idx]->Update(deltaT);
			m_OldPositions[idx] = m_Agents[idx]->GetPosition();
		}
	}
	else
	{
		for (UINT idx{}; idx < m_Agents.size(); ++idx)
		{
			RegisterNeighbors(m_Agents[idx]);
			m_Agents[idx]->Update(deltaT);
		}
	}

	if (m_TrimWorld)
	{
		for (auto pAgent : m_Agents)
		{
			pAgent->TrimToWorld(m_WorldSize);
		}
		m_pAgentToEvade->TrimToWorld(m_WorldSize);
	}


}

void Flock::Render(float deltaT)
{
	m_pAgentToEvade->Render(deltaT);


	if (m_DebugRendering)
	{
		DEBUGRENDERER2D->DrawCircle(m_Agents[0]->GetPosition(), m_NeighborhoodRadius, { 1,0,0 }, 0.9f);
		m_Agents[0]->SetRenderBehavior(true);

		for (auto pAgentOther : m_Agents)
		{
			if (pAgentOther != m_Agents[0])
			{
				float distanceSquared = ((pAgentOther->GetPosition().x - m_Agents[0]->GetPosition().x) * (pAgentOther->GetPosition().x - m_Agents[0]->GetPosition().x))
					+ ((pAgentOther->GetPosition().y - m_Agents[0]->GetPosition().y) * (pAgentOther->GetPosition().y - m_Agents[0]->GetPosition().y));

				if (distanceSquared <= m_NeighborhoodRadius * m_NeighborhoodRadius)
				{
					pAgentOther->SetBodyColor({ 0,1,0 });
				}
			}
		}

	}
	else
		m_Agents[0]->SetRenderBehavior(false);

	for (auto pAgent : m_Agents)
	{
		pAgent->Render(deltaT);
		pAgent->SetBodyColor({ 1,1,1 });
	}

	if (m_DebugSpacePartitioning && m_UsingSpacePartitioning)
	{
		m_pCellSpace->RenderCells();

		Elite::Polygon polygon{ std::vector<Elite::Vector2>{Elite::Vector2{m_Agents[0]->GetPosition().x - m_NeighborhoodRadius,m_Agents[0]->GetPosition().y - m_NeighborhoodRadius},
													Elite::Vector2{m_Agents[0]->GetPosition().x + m_NeighborhoodRadius,m_Agents[0]->GetPosition().y - m_NeighborhoodRadius},
													Elite::Vector2{m_Agents[0]->GetPosition().x + m_NeighborhoodRadius,m_Agents[0]->GetPosition().y + m_NeighborhoodRadius},
													Elite::Vector2{m_Agents[0]->GetPosition().x - m_NeighborhoodRadius,m_Agents[0]->GetPosition().y + m_NeighborhoodRadius}, } };
		DEBUGRENDERER2D->DrawPolygon(&polygon, Color{ 1,0,0 });

	}

}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	//ImGui::Text("Flocking");
	//ImGui::Spacing();



	ImGui::SliderFloat("Seeking", &m_pBlendedSteering->GetWeightedBehaviorsRef()[0].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Separation", &m_pBlendedSteering->GetWeightedBehaviorsRef()[1].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->GetWeightedBehaviorsRef()[2].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("VelMatch", &m_pBlendedSteering->GetWeightedBehaviorsRef()[3].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->GetWeightedBehaviorsRef()[4].weight, 0.f, 1.f, "%.2");

	ImGui::Checkbox("Debug Rendering", &m_DebugRendering);
	ImGui::Checkbox("Trim World", &m_TrimWorld);
	ImGui::SliderFloat("Trim Size", &m_WorldSize, 50.f, 200.f, "%.2");

	ImGui::Checkbox("Using Space Partitioning", &m_UsingSpacePartitioning);
	ImGui::Checkbox("Draw debug for Space Partitioning", &m_DebugSpacePartitioning);

	//End
	ImGui::PopAllowKeyboardFocus(); 
	ImGui::End();


}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	m_Neighbors.clear();
	m_Neighbors.resize(0);
	m_NrOfNeighbors = 0;

	Elite::Vector2 agentPos{ pAgent->GetPosition() };

	for (auto pAgentPotNbr : m_Agents)
	{
		if (pAgentPotNbr != pAgent)
		{
			Elite::Vector2 agentPotNbrPos{ pAgentPotNbr->GetPosition() };

			if (m_NeighborhoodRadius * m_NeighborhoodRadius >=
				(agentPotNbrPos.x - agentPos.x) * (agentPotNbrPos.x - agentPos.x) +
				(agentPotNbrPos.y - agentPos.y) * (agentPotNbrPos.y - agentPos.y))
			{
				m_Neighbors.push_back(pAgentPotNbr);
				++m_NrOfNeighbors;
			}
		}
	}
}

int Flock::GetNrOfNeighbors() const
{
	if (m_UsingSpacePartitioning)
	{
		return m_pCellSpace->GetNrOfNeighbors();
	}
	else
	{
		return m_NrOfNeighbors;
	}
}

const std::vector<SteeringAgent*>& Flock::GetNeighbors() const
{
	if (m_UsingSpacePartitioning)
	{
		return m_pCellSpace->GetNeighbors();
	}
	else
	{
		return m_Neighbors;
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	Vector2 averageNeigborPos{};

	if (m_UsingSpacePartitioning)
	{
		for (auto pAgent : m_pCellSpace->GetNeighbors())
		{
			averageNeigborPos += pAgent->GetPosition();
		}
		averageNeigborPos /= float(m_pCellSpace->GetNrOfNeighbors());
	}
	else
	{
		for (auto pAgent : m_Neighbors)
		{
			averageNeigborPos += pAgent->GetPosition();
		}
		averageNeigborPos /= float(m_NrOfNeighbors);
	}
	return averageNeigborPos;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{

	Vector2 averageNeigborVel{};

	if (m_UsingSpacePartitioning)
	{
		for (auto pAgent : m_pCellSpace->GetNeighbors())
		{
			averageNeigborVel += pAgent->GetLinearVelocity();
		}
		averageNeigborVel /= float(m_pCellSpace->GetNrOfNeighbors());
	}
	else
	{
		for (auto pAgent : m_Neighbors)
		{
			averageNeigborVel += pAgent->GetLinearVelocity();
		}
		averageNeigborVel /= float(m_NrOfNeighbors);
	}
	return averageNeigborVel;
}

void Flock::SetTarget_Seek(TargetData target)
{

	m_pSeekBehavior->SetTarget(target);
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}
