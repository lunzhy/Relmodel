/**
* @file GateStack.cpp
* @brief
*
*
*
* @author
* @version 
* @date 2015-3-31   21:58
* @note
* @todo
*/

#include "GateStack.h"
#include "SctmUtils.h"
#include "DomainDetails.h"
#include "Normalization.h"
#include "SctmPhys.h"
#include "Material.h"

using namespace SctmUtils;

GateStack::GateStack()
{
	this->temperature = SctmGlobalControl::Get().Temperature;
}

void GateStack::setParametersFromParamParser()
{
	ParamBase *parBase = NULL;

	//gate voltage
	parBase = SctmParameterParser::Get().GetPar(SctmParameterParser::st_gate_voltage);
	gateVoltage = dynamic_cast<Param<double> *>(parBase)->Value();

	//gate work function
	parBase = SctmParameterParser::Get().GetPar(SctmParameterParser::st_gate_workfunction);
	gateWorkfunction = dynamic_cast<Param<double> *>(parBase)->Value();

	//width
	parBase = SctmParameterParser::Get().GetPar(SctmParameterParser::st_width_value);
	widthValue = dynamic_cast<Param<double> *>(parBase)->Value();

	//interface layer thickness
	parBase = SctmParameterParser::Get().GetPar(SctmParameterParser::st_il_thick);
	thickIl = dynamic_cast<Param<double> *>(parBase)->Value();

	//oxide layer thick
	parBase = SctmParameterParser::Get().GetPar(SctmParameterParser::st_oxide_thick);
	thickOxide = dynamic_cast<Param<double> *>(parBase)->Value();

	//width grid
	parBase = SctmParameterParser::Get().GetPar(SctmParameterParser::st_width_grid);
	gridThickIl = dynamic_cast<Param<int> *>(parBase)->Value();

	//interface layer grid
	parBase = SctmParameterParser::Get().GetPar(SctmParameterParser::st_il_grid);
	gridThickIl = dynamic_cast<Param<int> *>(parBase)->Value();

	//oxide layer grid
	parBase = SctmParameterParser::Get().GetPar(SctmParameterParser::st_oxide_grid);
	gridThickOxide = dynamic_cast<Param<int> *>(parBase)->Value();

	//interface layer material name
	parBase = SctmParameterParser::Get().GetPar(SctmParameterParser::st_il_material);
	matNameStrIl = dynamic_cast<Param<string> *>(parBase)->Value();

	//oxide layer material name
	parBase = SctmParameterParser::Get().GetPar(SctmParameterParser::st_oxide_material);
	matNameStrOxide = dynamic_cast<Param<string> *>(parBase)->Value();
}

void GateStack::setDomainDetails()
{
	double nm_in_cm = SctmPhys::nm_in_cm;

	int indexVertex = 0;
	int indexElement = 0;
	int indexRegion = 0;
	int indexContact = 0;

	Normalization norm = Normalization(this->temperature);

	double coordX_in_nm = 0; // real value (without normalization), in [nm]
	double coordY_in_nm = 0;
	double normCoordX = 0; // normalized value, in [cm]
	double normCoordY = 0;

	//////////////////////////////////////////////////////////////////
	//set vertices
	//////////////////////////////////////////////////////////////////
	int xIndexMax = gridWidth;
	int yIndexMax = gridThickIl + gridThickOxide;
	//the iteration sequence determined the method to get id width specific (idx, idy)
	for (int iy = 0; iy <= yIndexMax; ++iy)
	{
		for (int ix = 0; ix <= xIndexMax; ++ix)
		{
			//the iteration sequence guarantees the use of FDDomainHelper
			coordX_in_nm = getVertCoordX(ix, iy);
			coordY_in_nm = getVertCoordY(ix, iy);
			if (coordX_in_nm < 0) //so coordY_in_nm will be less than o
			{
				continue;
			}
			normCoordX = norm.PushLength(coordX_in_nm * nm_in_cm);
			normCoordY = norm.PushLength(coordY_in_nm * nm_in_cm);
			vertices.push_back(new FDVertex(indexVertex, normCoordX, normCoordY));
			indexVertex++;
		}
	}

	///////////////////////////////////////////////////////////////////
	//set contacts
	///////////////////////////////////////////////////////////////////
	FDContact *currContact = NULL;
	FDVertex *currVertex = NULL;
	int vertID = 0;
	int indexX = 0;
	int indexY = 0;

	double voltage = gateVoltage;
	double workfun = gateWorkfunction;
	contacts.push_back(new FDContact(indexContact++, "Gate", norm.PushPotential(voltage), norm.PushPotential(workfun)));
	contacts.push_back(new FDContact(indexContact++, "Channel", 0, 0));//explicitly set voltage and workfunction to be 0
	//gate
	currContact = this->GetContact("Gate");
	indexY = gridThickIl + gridThickOxide;
	for (int ix = 0; ix <= gridWidth; ++ix)
	{
		indexX = ix;
		vertID = getVertIdAt(indexX, indexY);
		currVertex = this->GetVertex(vertID);
		currVertex->SetContact(currContact);
		currContact->AddVertex(currVertex);
	}
	//channel
	currContact = this->GetContact("Channel");
	indexY = 0;
	for (int ix = 0; ix <= gridWidth; ++ix)
	{
		indexX = ix;
		vertID = getVertIdAt(indexX, indexY);
		currVertex = this->GetVertex(vertID);
		currVertex->SetContact(currContact);
		currContact->AddVertex(currVertex);
	}

	///////////////////////////////////////////////////////////////////
	//set regions and elements
	///////////////////////////////////////////////////////////////////
	using MaterialDB::GetMaterial;
	using MaterialDB::Mat;
	Mat::Name currMatName;//ErrorMaterial
	currMatName = Mat::Parse(matNameStrIl);
	regions.push_back(new FDRegion(indexRegion++, "IL", GetMaterial(currMatName)));
	currMatName = Mat::Parse(matNameStrOxide);
	regions.push_back(new FDRegion(indexRegion++, "Oxide", GetMaterial(currMatName)));

	/////set element of each region
	FDRegion *currRegion = NULL;
	int ixBegin = 0;
	int ixEnd = 0; // exclude the end
	int iyBegin = 0;
	int iyEnd = 0;

	//interface layer
	currRegion = this->GetRegion("IL");
	ixBegin = 0; ixEnd = gridWidth;
	iyBegin = 0; iyEnd = gridThickIl;
	setSingleElement(indexElement, currRegion, ixBegin, ixEnd, iyBegin, iyEnd);
	
	//Oxide
	currRegion = this->GetRegion("Block");
	ixBegin = 0; ixEnd = gridWidth;
	iyBegin = gridThickIl; iyEnd = iyBegin + gridThickOxide;
	setSingleElement(indexElement, currRegion, ixBegin, ixEnd, iyBegin, iyEnd);

}

double GateStack::getVertCoordX(int idX, int idY)
{
	double currdX = 0; // the coordinate in x-direction, in [nm], without normalization 

	static double lengthPerGrid = widthValue / gridWidth;

	if (!isValidVertex(idX, idY))
	{
		return -1;
	}

	currdX = idX * lengthPerGrid;
	return currdX;
}

double GateStack::getVertCoordY(int idX, int idY)
{
	double currdY = 0; // the coordinate in y-direction, in [nm], without normalization
	static double lengthPerGridIl = thickIl / gridThickIl;
	static double lengthPerGridOxide = thickOxide / gridThickOxide;

	if (!isValidVertex(idX, idY))
	{
		return -1;
	}

	if (idY <= gridThickIl)
	{
		currdY = idY * lengthPerGridIl;
		return currdY;
	}
	idY -= gridThickIl;
	currdY = thickIl;

	currdY += idY * lengthPerGridOxide;
	return currdY;
}

bool GateStack::isValidVertex(int idX, int idY)
{
	bool ret = true;
	static int gridTotalX = gridWidth;
	static int gridTotalY = gridThickIl + gridThickOxide;
	if (idX < 0 || idX > gridTotalX)
	{
		ret = false;
	}
	if (idY < 0 || idY > gridTotalY)
	{
		ret = false;
	}
	return ret;
}

int GateStack::getVertIdAt(int idX, int idY)
{
	int vertID = 0;
	static int gridTotalX = gridWidth;
	static int gridTotalY = gridThickIl + gridThickOxide;

	//out of boundary
	if (idX < 0 || idX > gridTotalX)
	{
		return -1;
	}
	if (idY < 0 || idY > gridTotalY)
	{
		return -1;
	}

	vertID = (gridTotalX + 1)*idY + idX;
	return vertID;
}

void GateStack::setSingleElement(int &idElem, FDRegion *region, int xbegin, int xend, int ybegin, int yend)
{
	FDElement *currElem = NULL;
	FDVertex *swVertex = NULL;
	FDVertex *seVertex = NULL;
	FDVertex *nwVertex = NULL;
	FDVertex *neVertex = NULL;

	for (int iyElem = ybegin; iyElem != yend; ++iyElem)
	{
		for (int ixElem = xbegin; ixElem != xend; ++ixElem)
		{
			swVertex = GetVertex(getVertIdAt(ixElem, iyElem));
			seVertex = GetVertex(getVertIdAt(ixElem + 1, iyElem));
			neVertex = GetVertex(getVertIdAt(ixElem + 1, iyElem + 1));
			nwVertex = GetVertex(getVertIdAt(ixElem, iyElem + 1));
			currElem = new FDElement(idElem++, swVertex, seVertex, neVertex, nwVertex);
			this->elements.push_back(currElem);
			currElem->SetVertexAdjacent();
			currElem->SetRegion(region);
			region->AddElement(currElem);
		}
	}
}

void GateStack::setAdjacency()
{
	int gridTotalX = gridWidth;
	int gridTotalY = gridThickIl + gridThickOxide;

	FDVertex *currVert = NULL;
	FDVertex *adjacentVert = NULL;

	int vertID = 0;
	int adjacentID = 0;
	for (int iy = 0; iy <= gridTotalY; ++iy)
	{
		for (int ix = 0; ix <= gridTotalX; ++ix)
		{
			vertID = getVertIdAt(ix, iy);
			if (vertID < 0)//(ix, iy) does not corresponds to a valid vertex
			{
				continue;
			}
			currVert = this->GetVertex(vertID);
			//west vertex
			adjacentID = getVertIdAt(ix - 1, iy);
			if (adjacentID >= 0)
			{
				adjacentVert = this->GetVertex(adjacentID);
				currVert->WestVertex = adjacentVert;
				currVert->WestLength = FDVertex::Distance(currVert, currVert->WestVertex);
			}
			//north vertex
			adjacentID = getVertIdAt(ix, iy + 1);
			if (adjacentID >= 0)
			{
				adjacentVert = this->GetVertex(adjacentID);
				currVert->NorthVertex = adjacentVert;
				currVert->NorthLength = FDVertex::Distance(currVert, currVert->NorthVertex);
			}
			//east vertex
			adjacentID = getVertIdAt(ix + 1, iy);
			if (adjacentID >= 0)
			{
				adjacentVert = this->GetVertex(adjacentID);
				currVert->EastVertex = adjacentVert;
				currVert->EastLength = FDVertex::Distance(currVert, currVert->EastVertex);
			}
			//south vertex
			adjacentID = getVertIdAt(ix, iy - 1);
			if (adjacentID >= 0)
			{
				adjacentVert = this->GetVertex(adjacentID);
				currVert->SouthVertex = adjacentVert;
				currVert->SouthLength = FDVertex::Distance(currVert, currVert->SouthVertex);
			}
		}
	}
}

void GateStack::buildStructure()
{
	setParametersFromParamParser();
	setDomainDetails();
	setAdjacency();
}

void GateStack::setTrapDistribution()
{

}

