/**
* @file FDDomain.cpp
* @brief This file contains the definition/implementation of the member methods.
*
* 
*
* @author
* @version 
* @date 2013-7-4   19:40
* @note
* @todo
*/

#include "FDDomain.h"
#include "Material.h"
#include "DomainDetails.h"
#include "SctmUtils.h"
#include "SctmPhys.h"
#include "Normalization.h"

using SctmPhys::PhysProperty;
using namespace SctmUtils;

FDElement* FDDomain::GetElement(unsigned int id)
{
	//need to judge if the id is appropriate
	return elements.at(id);
	//return elements[id];
}

FDVertex* FDDomain::GetVertex(unsigned int id)
{
	return vertices.at(id);
	//return vertices[id];
}

FDRegion* FDDomain::GetRegion(unsigned int id)
{
	return regions.at(id);
}

FDRegion* FDDomain::GetRegion(std::string regionName)
{
	FDRegion *currRegion = NULL;
	bool found = false;
	for (size_t iReg = 0; iReg != this->regions.size(); ++iReg)
	{
		currRegion = GetRegion(iReg);
		if (currRegion->RegName == regionName)
		{
			found = true;
			break;
		}
	}
	SCTM_ASSERT(found, 10048);
	return currRegion;
}

FDContact* FDDomain::GetContact(unsigned int id)
{
	return contacts.at(id);
}

FDContact* FDDomain::GetContact(std::string contactName)
{
	FDContact *currCont = NULL;
	bool found = false;
	for (size_t iCont = 0; iCont != contacts.size(); ++iCont)
	{
		currCont = GetContact(iCont);
		if (currCont->ContactName == contactName)
		{
			found = true;
			break;
		}
	}
	SCTM_ASSERT(found, 10030);
	return currCont;
}

std::vector<FDVertex *>& FDDomain::GetVertices()
{
	return this->vertices;
}

std::vector<FDVertex *>& FDDomain::GetDDVerts()
{
	return this->ddVerts;
}

bool FDDomain::isValidElem(FDElement *elem)
{
	return elem != NULL;
}

bool FDDomain::isNotTrappingElem(FDElement *elem)
{
	//TODO: deal with the trap
	if (elem == NULL)
		return true;
	bool ret = false;

	string regionname = elem->Region->RegName;
	ret = (regionname.find("Trap") > regionname.size());

	return ret;
}

void FDDomain::setBoundary()
{
	FDVertex *currVertex;
	for (std::size_t iVer = 0; iVer != vertices.size(); ++iVer)
	{
		currVertex = GetVertex(iVer);//in FDDomain, the index of vertex in the vertices vector in the vertexID.
		setBndVert_Potential(currVertex);//set the boundary (not boundary condition). This method will check if the vertex is a boundary vertex
		setBndVert_Density(currVertex);
	}
}

void FDDomain::setBndVert_Potential(FDVertex *vert)
{
	bool isValid_NW = isValidElem(vert->NorthwestElem);
	bool isValid_NE = isValidElem(vert->NortheastElem);
	bool isValid_SE = isValidElem(vert->SoutheastElem);
	bool isValid_SW = isValidElem(vert->SouthwestElem);

	static FDBoundary::BCName bcToSet = FDBoundary::Potential;
	static FDBoundary::BCType defaultBCType = FDBoundary::BC_Neumann;
	//for vertex that is not correlated to a contact. The value of 0 indicated an artificial boundary there.
	//Northwest corner
	if ( !isValid_NW && !isValid_NE &&
		!isValid_SW && isValid_SE )
	{
		//when the two adjacent neighbors are both valid (other region) or invalid, the boundary direction is considered to be along the diagonal
		//vert->BndCond.SetBndCond(FDBoundary::Potential, FDBoundary::BC_Neumann, 0, VectorValue(-vert->EastLength, vert->SouthLength));
		vert->BndCond.SetBnd(bcToSet, defaultBCType, VectorValue(-vert->EastLength, vert->SouthLength));
		return;
	}

	//Northeast corner
	if ( !isValid_NW && !isValid_NE && 
		isValid_SW && !isValid_SE )
	{
		//when the two adjacent neighbors are both valid (other region) or invalid, the the boundary direction is considered to be along the diagonal
		//vert->BndCond.SetBndCond(FDBoundary::Potential, FDBoundary::BC_Neumann, 0, VectorValue(vert->WestLength, vert->SouthLength));
		vert->BndCond.SetBnd(bcToSet, defaultBCType, VectorValue(vert->WestLength, vert->SouthLength));
		return;
	}

	//Southeast corner
	if ( isValid_NW && !isValid_NE && 
		!isValid_SW && !isValid_SE )
	{
		//when the two adjacent neighbors are both valid (other region) or invalid, the the boundary direction is considered to be along the diagonal
		//vert->BndCond.SetBndCond(FDBoundary::Potential, FDBoundary::BC_Neumann, 0, VectorValue(vert->WestLength, -vert->NorthLength));
		vert->BndCond.SetBnd(bcToSet, defaultBCType, VectorValue(vert->WestLength, -vert->NorthLength));
		return;
	}

	//Southwest corner
	if ( !isValid_NW && isValid_NE && 
		!isValid_SW && !isValid_SE)
	{
		//when the two adjacent neighbors are both valid (other region) or invalid, the the boundary direction is considered to be along the diagonal
		//vert->BndCond.SetBndCond(FDBoundary::Potential, FDBoundary::BC_Neumann, 0, VectorValue(-vert->EastLength, -vert->NorthLength));
		vert->BndCond.SetBnd(bcToSet, defaultBCType, VectorValue(-vert->EastLength, -vert->NorthLength));
		return;
	}

	//North side
	if ( !isValid_NW && !isValid_NE && 
		isValid_SW && isValid_SE)
	{
		//vert->BndCond.SetBndCond(FDBoundary::Potential, FDBoundary::BC_Neumann, 0, VectorValue(0, 1));
		vert->BndCond.SetBnd(bcToSet, defaultBCType, VectorValue(0, 1));
		return;
	}

	//East side
	if ( isValid_NW && !isValid_NE && 
		isValid_SW && !isValid_SE)
	{
		//vert->BndCond.SetBndCond(FDBoundary::Potential, FDBoundary::BC_Neumann, 0, VectorValue(1, 0));
		vert->BndCond.SetBnd(bcToSet, defaultBCType, VectorValue(1, 0));
		return;
	}

	//South side
	if ( isValid_NW && isValid_NE && 
		!isValid_SW && !isValid_SE)
	{
		//vert->BndCond.SetBndCond(FDBoundary::Potential, FDBoundary::BC_Neumann, 0, VectorValue(0, -1));
		vert->BndCond.SetBnd(bcToSet, defaultBCType, VectorValue(0, -1));
		return;
	}

	//West side
	if ( !isValid_NW && isValid_NE && 
		!isValid_SW && isValid_SE)
	{
		//vert->BndCond.SetBndCond(FDBoundary::Potential, FDBoundary::BC_Neumann, 0, VectorValue(-1, 0));
		vert->BndCond.SetBnd(bcToSet, defaultBCType, VectorValue(-1, 0));
		return;
	}

	//lack of NorthWest corner
	if (!isValid_NW && isValid_NE &&
		isValid_SW && isValid_SE)
	{
		vert->BndCond.SetBnd(bcToSet, defaultBCType, VectorValue(-vert->WestLength, vert->NorthLength));
		return;
	}

	//lack of NorthEast corner
	if (isValid_NW && !isValid_NE &&
		isValid_SW && isValid_SE)
	{
		vert->BndCond.SetBnd(bcToSet, defaultBCType, VectorValue(vert->EastLength, vert->NorthLength));
		return;
	}

	//lack of SouthEast corner
	if (isValid_NW && isValid_NE &&
		isValid_SW && !isValid_SE)
	{
		vert->BndCond.SetBnd(bcToSet, defaultBCType, VectorValue(vert->EastLength, -vert->SouthLength));
		return;
	}

	//lack of SouthWest corner
	if (isValid_NW && isValid_NE &&
		!isValid_SW && isValid_SE)
	{
		vert->BndCond.SetBnd(bcToSet, defaultBCType, VectorValue(-vert->WestLength, vert->SouthLength));
		return;
	}
}

void FDDomain::setBndVert_Density(FDVertex *vert)
{
	bool notTrapping_NW = isNotTrappingElem(vert->NorthwestElem);
	bool notTrapping_NE = isNotTrappingElem(vert->NortheastElem);
	bool notTrapping_SE = isNotTrappingElem(vert->SoutheastElem);
	bool notTrapping_SW = isNotTrappingElem(vert->SouthwestElem);

	bool valid_NW = isValidElem(vert->NorthwestElem);
	bool valid_NE = isValidElem(vert->NortheastElem);
	bool valid_SE = isValidElem(vert->SoutheastElem);
	bool valid_SW = isValidElem(vert->SouthwestElem);

	//static FDBoundary::BCName bcToSet = FDBoundary::eDensity;
	static FDBoundary::BCType defaultTCType = FDBoundary::BC_Cauchy;

	//currently the boundary condition direction is the same with boundary direction
    //Northwest corner
	if ( notTrapping_NW && notTrapping_NE &&
		notTrapping_SW && !notTrapping_SE )
	{
		//when the two adjacent neighbors are both valid (other region) or invalid, the boundary direction is considered to be along the diagonal
		vert->BndCond.SetBnd(FDBoundary::eDensity, defaultTCType, VectorValue(-vert->EastLength, vert->SouthLength));
		vert->BndCond.SetBnd(FDBoundary::hDensity, defaultTCType, VectorValue(-vert->EastLength, vert->SouthLength));
		return;
	}

	//Northeast corner
	if ( notTrapping_NW && notTrapping_NE && 
		!notTrapping_SW && notTrapping_SE )
	{
		//when the two adjacent neighbors are both valid (other region) or invalid, the boundary direction is considered to be along the diagonal
		vert->BndCond.SetBnd(FDBoundary::eDensity, defaultTCType, VectorValue(vert->WestLength, vert->SouthLength));
		vert->BndCond.SetBnd(FDBoundary::hDensity, defaultTCType, VectorValue(vert->WestLength, vert->SouthLength));
		return;
	}

	//Southeast corner
	if ( !notTrapping_NW && notTrapping_NE && 
		notTrapping_SW && notTrapping_SE )
	{
		//when the two adjacent neighbors are both valid (other region) or invalid, the boundary direction is considered to be along the diagonal
		vert->BndCond.SetBnd(FDBoundary::eDensity, defaultTCType, VectorValue(vert->WestLength, -vert->NorthLength));
		vert->BndCond.SetBnd(FDBoundary::hDensity, defaultTCType, VectorValue(vert->WestLength, -vert->NorthLength));
		return;
	}

	//Southwest corner
	if ( notTrapping_NW && !notTrapping_NE && 
		notTrapping_SW && notTrapping_SE)
	{
		//when the two adjacent neighbors are both valid (other region) or invalid, the boundary direction is considered to be along the diagonal
		vert->BndCond.SetBnd(FDBoundary::eDensity, defaultTCType, VectorValue(-vert->EastLength, -vert->NorthLength));
		vert->BndCond.SetBnd(FDBoundary::hDensity, defaultTCType, VectorValue(-vert->EastLength, -vert->NorthLength));
		return;
	}

	//North side
	if ( notTrapping_NW && notTrapping_NE && 
		!notTrapping_SW && !notTrapping_SE)
	{
		vert->BndCond.SetBnd(FDBoundary::eDensity, defaultTCType, VectorValue(0, 1));
		vert->BndCond.SetBnd(FDBoundary::hDensity, defaultTCType, VectorValue(0, 1));
		return;
	}

	//East side
	if ( !notTrapping_NW && notTrapping_NE && 
		!notTrapping_SW && notTrapping_SE)
	{
		vert->BndCond.SetBnd(FDBoundary::eDensity, defaultTCType, VectorValue(1, 0));
		vert->BndCond.SetBnd(FDBoundary::hDensity, defaultTCType, VectorValue(1, 0));
		return;
	}

	//South side
	if ( !notTrapping_NW && !notTrapping_NE && 
		notTrapping_SW && notTrapping_SE)
	{
		vert->BndCond.SetBnd(FDBoundary::eDensity, defaultTCType, VectorValue(0, -1));
		vert->BndCond.SetBnd(FDBoundary::hDensity, defaultTCType, VectorValue(0, -1));
		return;
	}

	//West side
	if ( notTrapping_NW && !notTrapping_NE && 
		notTrapping_SW && !notTrapping_SE)
	{
		vert->BndCond.SetBnd(FDBoundary::eDensity, defaultTCType, VectorValue(-1, 0));
		vert->BndCond.SetBnd(FDBoundary::hDensity, defaultTCType, VectorValue(-1, 0));
		return;
	}

}

void FDDomain::BuildDomain()
{
	//Initialize the vectors in FDDomain
	vertices.clear();
	ddVerts.clear();
	elements.clear();
	regions.clear();
	contacts.clear();

	//build the data and mesh structure of simulated region, this is a pure virtual method
	buildStructure();
	setVertexRadius();
	//fill the vertices belonging to drift-diffusion process
	fillDDVerts();
	//set the physics value related to vertex, when the vertex is related to trapping layer
	setVertexPhysProperty();
	setVertexTrapProperty();
	//set the trap property
	setTrapDistribution();
	setTrapOccupation();
	//set the boundary condition, the specific value is not considered in this class.
	setBoundary();
	updateBndCond();
	
	//in case the specific domain has some special post-procedure
	//This is used because previously the gate and channel potential is directly set using global control
	//Currently, it is done using related parameters.
	postProcessOfDomain();
}

void FDDomain::setVertexPhysProperty()
{
	FDVertex * currVertex = NULL;

	using namespace MaterialDB;
	using namespace SctmPhys;
	using std::vector;

	vector<MatProperty::Name> matPrptys;
	vector<PhysProperty::Name> verPrptys; //vertex-based physical property
	matPrptys.push_back(MatProperty::Mat_ElectronAffinity); verPrptys.push_back(PhysProperty::ElectronAffinity);
	matPrptys.push_back(MatProperty::Mat_Bandgap); verPrptys.push_back(PhysProperty::Bandgap);
	matPrptys.push_back(MatProperty::Mat_DielectricConstant); verPrptys.push_back(PhysProperty::DielectricConstant);

	//for electrons
	matPrptys.push_back(MatProperty::Mat_ElectronMass); verPrptys.push_back(PhysProperty::eMass);
	matPrptys.push_back(MatProperty::Mat_ElecDOSMass); verPrptys.push_back(PhysProperty::eDOSMass);
	matPrptys.push_back(MatProperty::Mat_ElectronMobility); verPrptys.push_back(PhysProperty::eMobility);

	//for holes
	matPrptys.push_back(MatProperty::Mat_HoleMass); verPrptys.push_back(PhysProperty::hMass);
	matPrptys.push_back(MatProperty::Mat_HoleDOSMass); verPrptys.push_back(PhysProperty::hDOSMass);
	matPrptys.push_back(MatProperty::Mat_HoleMobility); verPrptys.push_back(PhysProperty::hMobility);

	//iteration over the vertices
	for (std::size_t iVer = 0; iVer != this->vertices.size(); ++iVer)
	{
		currVertex = GetVertex(iVer);
		//iteration over the physical properties to be set from material property
		for (std::size_t iPrpty = 0; iPrpty != matPrptys.size(); ++iPrpty)
		{
			//filling vertex physics using material property
			//The method for filling vertex-based physical value using material-based value is ready
			//electron mobility is only valid in the trapping region
			if (matPrptys.at(iPrpty) == MatProperty::Mat_ElectronMobility || 
				matPrptys.at(iPrpty) == MatProperty::Mat_ElecDOSMass ||
				matPrptys.at(iPrpty) == MatProperty::Mat_HoleMobility ||
				matPrptys.at(iPrpty) == MatProperty::Mat_HoleDOSMass)
			{
				currVertex->Phys->FillVertexPhysUsingMatPrpty(verPrptys.at(iPrpty), matPrptys.at(iPrpty), true);
			}
			else
			{
				currVertex->Phys->FillVertexPhysUsingMatPrpty(verPrptys.at(iPrpty), matPrptys.at(iPrpty));
			}
			currVertex->Phys->CalculateDensityControlArea();

			//the below is added for storing the properties of vertex belonging to different materials.
			//the weighted average value is also stored.
			if (matPrptys.at(iPrpty) == MatProperty::Mat_ElectronAffinity
				|| matPrptys.at(iPrpty) == MatProperty::Mat_Bandgap
				|| matPrptys.at(iPrpty) == MatProperty::Mat_DielectricConstant)
			{
				currVertex->Phys->SetMultiPrpty(verPrptys.at(iPrpty), matPrptys.at(iPrpty));
			}
		}
	}
}

void FDDomain::fillDDVerts()
{
	FDVertex *currVert = NULL;

	bool notTrapping_NW = false;
	bool notTrapping_NE = false;
	bool notTrapping_SE = false;
	bool notTrapping_SW = false;

	for (size_t iVert = 0; iVert != this->vertices.size(); ++iVert)
	{
		currVert = this->vertices.at(iVert);

		notTrapping_NW = isNotTrappingElem(currVert->NorthwestElem);
		notTrapping_NE = isNotTrappingElem(currVert->NortheastElem);
		notTrapping_SE = isNotTrappingElem(currVert->SoutheastElem);
		notTrapping_SW = isNotTrappingElem(currVert->SouthwestElem);

		if (!(notTrapping_NE && notTrapping_NW && notTrapping_SE && notTrapping_SW))
		{
			this->ddVerts.push_back(currVert);
		}
	}
}

void FDDomain::updateBndCond()
{
	FDVertex *currVertex;
	for (std::size_t iVer = 0; iVer != vertices.size(); ++iVer)
	{
		currVertex = GetVertex(iVer);//in FDDomain, the index of vertex in the vertices vector in the vertexID.
		//CAUTION! assume all vertices belong to contact are at boundary
		if (currVertex->IsAtBoundary(FDBoundary::Potential))
		{
			updateBCVert_Potential(currVertex);
		}
		if (currVertex->IsAtBoundary(FDBoundary::eDensity))
		{
			updateBCVert_Density(currVertex);
		}
	}
}

void FDDomain::updateBCVert_Potential(FDVertex *vert)
{
	Normalization norm = Normalization(SctmGlobalControl::Get().Temperature);
	static double workFunction_Si = SctmPhys::ReferencePotential;//already in normalized value

	FDContact *contact = NULL;
	double gateVoltage = 0;
	double gateWorkfunction = 0;
	double gatePotential = 0;

	//to decide if the vertex is at a contact.
	if (vert->IsAtContact())
	{
		contact = vert->Contact;
		if (contact->ContactName == "Channel")
		{
			//change the boundary condition type to BC_Dirichlet, not care about the value
			//because the channel potential is set after solving substrate.
			vert->BndCond.RefreshBndCond(FDBoundary::Potential, FDBoundary::BC_Dirichlet);
			return;
		}
		else//the gate name is in accordance with the name specified in setting domain details
		{
			//change the boundary condition type to BC_Dirichlet
			//set the gate potential using gate voltage and work function.
			gateVoltage = contact->Voltage;
			gateWorkfunction = contact->Workfunction;
			gatePotential = gateVoltage - (gateWorkfunction - workFunction_Si);
			vert->BndCond.RefreshBndCond(FDBoundary::Potential, FDBoundary::BC_Dirichlet, gatePotential);
			return;
		}
	}
}

void FDDomain::updateBCVert_Density(FDVertex *vert)
{
	//When dealing with the normal direction of the boundary condition in terms eDensity problem, actually, the vector value
	//of the boundary condition is not the real normal vector of the boundary direction. For, example, to a corner vertex
	//of the trapping region, the normal vector is determined considering if its adjacent element is valid.

	bool notTrapping_NW = isNotTrappingElem(vert->NorthwestElem);
	bool notTrapping_NE = isNotTrappingElem(vert->NortheastElem);
	bool notTrapping_SE = isNotTrappingElem(vert->SoutheastElem);
	bool notTrapping_SW = isNotTrappingElem(vert->SouthwestElem);

	bool valid_NW = isValidElem(vert->NorthwestElem);
	bool valid_NE = isValidElem(vert->NortheastElem);
	bool valid_SE = isValidElem(vert->SoutheastElem);
	bool valid_SW = isValidElem(vert->SouthwestElem);

	//Northwest corner
	if ( notTrapping_NW && notTrapping_NE &&
		notTrapping_SW && !notTrapping_SE )
	{
		if (              valid_NE &&
			!valid_SW )
		{
			vert->BndCond.RefreshBndCond(FDBoundary::eDensity, VectorValue(0, 1));
			vert->BndCond.RefreshBndCond(FDBoundary::hDensity, VectorValue(0, 1));
			return;
		}
		if (              !valid_NE &&
			valid_SW )
		{
			vert->BndCond.RefreshBndCond(FDBoundary::eDensity, VectorValue(-1, 0));
			vert->BndCond.RefreshBndCond(FDBoundary::hDensity, VectorValue(-1, 0));
			return;
		}
	}

	//Northeast corner
	if ( notTrapping_NW && notTrapping_NE && 
		!notTrapping_SW && notTrapping_SE )
	{
		if ( valid_NW &&
			!valid_SE)
		{
			vert->BndCond.RefreshBndCond(FDBoundary::eDensity, VectorValue(0, 1));
			vert->BndCond.RefreshBndCond(FDBoundary::hDensity, VectorValue(0, 1));
			return;
		}
		if ( !valid_NW &&
			valid_SE)
		{
			vert->BndCond.RefreshBndCond(FDBoundary::eDensity, VectorValue(1, 0));
			vert->BndCond.RefreshBndCond(FDBoundary::hDensity, VectorValue(1, 0));
			return;
		}
	}

	//Southeast corner
	if ( !notTrapping_NW && notTrapping_NE && 
		notTrapping_SW && notTrapping_SE )
	{
		if (			!valid_NE &&
			valid_SW)
		{
			vert->BndCond.RefreshBndCond(FDBoundary::eDensity, VectorValue(0, -1));
			vert->BndCond.RefreshBndCond(FDBoundary::hDensity, VectorValue(0, -1));
			return;
		}
		if (			valid_NE &&
			!valid_SW)
		{
			vert->BndCond.RefreshBndCond(FDBoundary::eDensity, VectorValue(1, 0));
			vert->BndCond.RefreshBndCond(FDBoundary::hDensity, VectorValue(1, 0));
			return;
		}
	}

	//Southwest corner
	if ( notTrapping_NW && !notTrapping_NE && 
		notTrapping_SW && notTrapping_SE)
	{
		if ( valid_NW &&
			!valid_SE )
		{
			vert->BndCond.RefreshBndCond(FDBoundary::eDensity, VectorValue(-1, 0));
			vert->BndCond.RefreshBndCond(FDBoundary::hDensity, VectorValue(-1, 0));
			return;
		}
		if ( !valid_NW &&
			valid_SE )
		{
			vert->BndCond.RefreshBndCond(FDBoundary::eDensity, VectorValue(0, -1));
			vert->BndCond.RefreshBndCond(FDBoundary::hDensity, VectorValue(0, -1));
			return;
		}
	}
}

void FDDomain::setVertexTrapProperty()
{
	using MaterialDB::MatProperty;
	using SctmPhys::TrapProperty;

	FDVertex *currVert = NULL;
	for (size_t iVert = 0; iVert != ddVerts.size(); ++iVert)
	{
		currVert = ddVerts.at(iVert);
		currVert->Trap = new TrapProperty(currVert);

		currVert->Trap->FillTrapPrptyUsingMatPrpty(TrapProperty::HighFrqEpsilon, MatProperty::Mat_HighFrqDielConst);

		//for electrons
		currVert->Trap->FillTrapPrptyUsingMatPrpty(TrapProperty::eFrequency_T2B, MatProperty::Mat_ElecFrequencyT2B);
		currVert->Trap->FillTrapPrptyUsingMatPrpty(TrapProperty::eFrequency_PF, MatProperty::Mat_ElecFrequencyPF);
		currVert->Trap->FillTrapPrptyUsingMatPrpty(TrapProperty::eCrossSection, MatProperty::Mat_ElecXSection);
		currVert->Trap->FillTrapPrptyUsingMatPrpty(TrapProperty::eTrapCrossSection, MatProperty::Mat_ElecTrappedXSection);
		currVert->Trap->FillTrapPrptyUsingMatPrpty(TrapProperty::eEnergyFromCondBand, MatProperty::Mat_ElecTrapEnergyFromCB);

		//for holes
		currVert->Trap->FillTrapPrptyUsingMatPrpty(TrapProperty::hFrequency_T2B, MatProperty::Mat_HoleFrequencyT2B);
		currVert->Trap->FillTrapPrptyUsingMatPrpty(TrapProperty::hFrequency_PF, MatProperty::Mat_HoleFrequencyPF);
		currVert->Trap->FillTrapPrptyUsingMatPrpty(TrapProperty::hCrossSection, MatProperty::Mat_HoleXSection);
		currVert->Trap->FillTrapPrptyUsingMatPrpty(TrapProperty::hTrapCrossSection, MatProperty::Mat_HoleTrappedXSection);
		currVert->Trap->FillTrapPrptyUsingMatPrpty(TrapProperty::hEnergyFromValeBand, MatProperty::Mat_HoleTrapEnergyFromVB);
	}
}

MaterialDB::Mat::Name FDDomain::GetTrapMatName()
{
	//CAUTION!!! This method is relative to the region name assigned during building the domain.
	MaterialDB::Mat::Name matname;
	string simStructure = SctmGlobalControl::Get().Structure;
	if (simStructure == "Single")
	{
		matname = GetRegion("Trap")->Mat->MatName();
	}
	else if (simStructure == "TripleFull")
	{
		matname = GetRegion("Trap.Gate2")->Mat->MatName();
	}
	return matname;
}

void FDDomain::setTrapOccupation()
{
	//set trap occupation is revised for the new consideration of trap density, which accounts for electron and hole
	//trap simultaneously, i.e., amphoteric traps.
	FDVertex *currVert = NULL;
	double trapDens = 0;
	double eTrappedDens = 0;
	double hTrappedDens = 0;
	double trapOccupy = SctmGlobalControl::Get().TrapOccupation;
	string simStructure = SctmGlobalControl::Get().Structure;
	string trappedRegion = SctmGlobalControl::Get().TrappedCell;

	if (simStructure == "Single")
	{
		for (size_t iVert = 0; iVert != ddVerts.size(); ++iVert)
		{
			currVert = ddVerts.at(iVert);
			trapDens = currVert->Trap->GetTrapPrpty(TrapProperty::TrapDensity);

			if (trapOccupy < -1 || trapOccupy > 1)
			{
				SCTM_ASSERT(SCTM_ERROR, 10044);
			}
			else if (trapOccupy < 0) // occupied by electrons
			{
				eTrappedDens = trapDens * SctmMath::abs(trapOccupy);
				currVert->Trap->SetTrapPrpty(TrapProperty::eTrapped, eTrappedDens);
			}
			else if (trapOccupy > 0) // occupied by holes
			{
				hTrappedDens = trapDens * trapOccupy;
				currVert->Trap->SetTrapPrpty(TrapProperty::hTrapped, hTrappedDens);
			}
		}
	}
	else if (simStructure == "TripleFull")
	{
		std::vector<string> regions;

		if (trappedRegion == "Center")
		{
			regions.push_back("Trap.Gate2");
		}
		else if (trappedRegion == "All")
		{
			regions.push_back("Trap.Gate1");
			regions.push_back("Trap.Gate2");
			regions.push_back("Trap.Gate3");
		}
		else if (trappedRegion == "Side")
		{
			regions.push_back("Trap.Gate1");
			regions.push_back("Trap.Gate3");
		}

		for (size_t iVert = 0; iVert != ddVerts.size(); ++iVert)
		{
			currVert = ddVerts.at(iVert);
			trapDens = currVert->Trap->GetTrapPrpty(TrapProperty::eTrapDensity);

			if (trapOccupy < -1 || trapOccupy > 1)
			{
				SCTM_ASSERT(SCTM_ERROR, 10044);
			}
			else if (trapOccupy < 0) // occupied by electrons
			{
				eTrappedDens = trapDens * SctmMath::abs(trapOccupy);
				for (std::vector<string>::iterator it = regions.begin(); it != regions.end(); ++it)
				{
					if (isBelongToRegion(currVert, *it))
					{
						//one vertex can only belong to one region under the gate
						currVert->Trap->SetTrapPrpty(TrapProperty::eTrapped, eTrappedDens);
						break;
					}
				}
			}
			else if (trapOccupy > 0) // occupied by holes
			{
				hTrappedDens = trapDens * trapOccupy;
				for (std::vector<string>::iterator it = regions.begin(); it != regions.end(); ++it)
				{
					if (isBelongToRegion(currVert, *it))
					{
						//one vertex can only belong to one region under the gate
						currVert->Trap->SetTrapPrpty(TrapProperty::hTrapped, hTrappedDens);
						break;
					}
				}
			}

			
		}
	}

}

void FDDomain::RefreshGateVoltage()
{
	Normalization norm = Normalization(SctmGlobalControl::Get().Temperature);
	static double workFunction_Si = SctmPhys::ReferencePotential;//already in normalized value

	FDContact *contact = NULL;
	FDVertex *vert = NULL;
	vector<FDVertex *> contVerts;

	string gateName = "";
	double gateWorkfunction = 0;
	double gatePotential = 0;
	double voltage = 0;
	
	static vector<string> gateNames;
	static bool isLoad = false;
	string simStructure = SctmGlobalControl::Get().Structure;
	
	if (!isLoad && simStructure == "Single")
	{
		gateNames.push_back("Gate");
		isLoad = true;
	}
	else if (!isLoad && (simStructure == "TripleFull"))
	{
		gateNames.push_back("Gate1");
		gateNames.push_back("Gate2");
		gateNames.push_back("Gate3");
		isLoad = true;
	}

	for (size_t in = 0; in != gateNames.size(); ++in)
	{
		gateName = gateNames.at(in);
		contact = this->GetContact(gateName);
		//refresh gate voltage
		if (gateName == "Gate" || gateName == "Gate1")
		{
			voltage = SctmTimeStep::Get().VoltageCellA();
		}
		else if (gateName == "Gate2")
		{
			voltage = SctmTimeStep::Get().VoltageCellB();
		}
		else if (gateName == "Gate3")
		{
			voltage = SctmTimeStep::Get().VoltageCellC();
		}
		//the voltage stored in SctmTimeStep is in normalized value
		contact->Voltage = voltage;

		//refresh potential of gate vertex
		contVerts = contact->GetContactVerts();
		for (size_t iv = 0; iv != contVerts.size(); ++iv)
		{
			gateWorkfunction = contact->Workfunction;
			gatePotential = voltage - (gateWorkfunction - workFunction_Si);
			vert = contVerts.at(iv);
			vert->BndCond.RefreshBndCond(FDBoundary::Potential, gatePotential);
		}
	}
}

void FDDomain::ClearCarrier()
{
	FDVertex *vert = NULL;
	for (size_t iv = 0; iv != this->ddVerts.size(); ++iv	)
	{
		vert = this->ddVerts.at(iv);
		vert->Phys->SetPhysPrpty(PhysProperty::eDensity, 0);
		vert->Phys->SetPhysPrpty(PhysProperty::hDensity, 0);
	}
}

bool FDDomain::isBelongToRegion(FDVertex *vert, string regName)
{
	bool isBelong = false;

	FDElement *northWestElem = NULL;
	FDElement *northEastElem = NULL;
	FDElement *southEastElem = NULL;
	FDElement *southWestElem = NULL;

	northWestElem = vert->NorthwestElem;
	if (northWestElem != NULL && northWestElem->Region->RegName == regName)
	{
		isBelong = true;
	}
	northEastElem = vert->NortheastElem;
	if (northEastElem != NULL && northEastElem->Region->RegName == regName)
	{
		isBelong = true;
	}
	southEastElem = vert->SoutheastElem;
	if (southEastElem != NULL && southEastElem->Region->RegName == regName)
	{
		isBelong = true;
	}
	southWestElem = vert->SouthwestElem;
	if (southWestElem != NULL && southWestElem->Region->RegName == regName)
	{
		isBelong = true;
	}

	return isBelong;
}

std::vector<FDVertex *> FDDomain::GetVertsOfRegion(std::string name)
{
	std::vector<FDVertex *> regVerts;
	FDVertex* vert = NULL;

	for (size_t vertID = 0; vertID != this->vertices.size(); ++vertID)
	{
		vert = vertices.at(vertID);
		if (FDDomain::isBelongToRegion(vert, name))
		{
			regVerts.push_back(vert);
		}
	}
	return regVerts;
}

void FDDomain::setVertexRadius()
{
	Normalization norm = Normalization(this->temperature);
	double nm_in_cm = SctmPhys::nm_in_cm;
	double channel_radius = SctmGlobalControl::Get().ChannelRadius; // in [nm]
	channel_radius = norm.PushLength(channel_radius * nm_in_cm);

	FDVertex* vert = NULL;
	for (size_t iv = 0; iv != this->vertices.size(); ++iv)
	{
		vert = this->vertices.at(iv);
		vert->R = vert->Y + channel_radius; // r = r0 + y
	}
}

void FDDomain::ReadTrappedOccupation()
{
	FDVertex* vert = NULL;
	double trapDens = 0;
	double eTrappedDens = 0;
	double hTrappedDens = 0;
	double eOcc = 0;
	double hOcc = 0;
	vector<double> vecElecOcc;
	vector<double> vecHoleOcc;

	SctmData::Get().ReadTrappedOcc(vecElecOcc, vecHoleOcc);

	for (size_t iVert = 0; iVert != this->ddVerts.size(); ++iVert)
	{
		vert = ddVerts.at(iVert);
		trapDens = vert->Trap->GetTrapPrpty(TrapProperty::TrapDensity);
		eOcc = vecElecOcc.at(iVert);
		hOcc = vecHoleOcc.at(iVert);
		eTrappedDens = trapDens * eOcc;
		hTrappedDens = trapDens * hOcc;
		vert->Trap->SetTrapPrpty(TrapProperty::eTrapped, eTrappedDens);
		vert->Trap->SetTrapPrpty(TrapProperty::hTrapped, hTrappedDens);
	}
}


