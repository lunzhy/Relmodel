/**
* @file GateStack.h
* @brief
*
*
*
* @author
* @version 
* @date 2015-3-31   21:52
* @note
* @todo
*/

#ifndef _GATESTACK_H_
#define _GATESTACK_H_
#include "FDDomain.h"
#include <string>

using std::string;

class GateStack : public FDDomain
{
public:
	GateStack();
	void ReadChargeOccupation();
	void RefreshGatePotential();

protected:
	double gateVoltage;
	double gateWorkfunction;
	
	double widthValue;
	int gridWidth;

	double thickIl;
	int gridThickIl;

	double thickOxide;
	int gridThickOxide;

	string matNameStrIl;
	string matNameStrOxide;

protected:
	void buildStructure();
	void setParametersFromParamParser();
	void setDomainDetails();
	void setAdjacency();
	void setTrapDistribution();

	//the methods below are used to calculate the coordinates of specific vertex.
	double getVertCoordX(int idX, int idY);
	double getVertCoordY(int idX, int idY);
	int getVertIdAt(int idX, int idY);
	bool isValidVertex(int idX, int idY);
	void setSingleElement(int &idElem, FDRegion *region, int xbegin, int xend, int ybegin, int yend);
};

#endif