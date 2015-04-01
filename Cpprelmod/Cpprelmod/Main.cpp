#include "SimpleONO.h"
#include "FDDomain.h"
#include "TunnelSolver.h"
#include "SctmUtils.h"
#include "PoissonSolver.h"
#include "DDSolver.h"
#include "SctmPhys.h"
#include "SolverPack.h"
#include "Normalization.h"
#include "SubstrateSolver.h"
#include "TripleCells.h"
#include "GateStack.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>

using namespace SctmUtils;

void initialize(const char *defaulParFile, const char *prjdir, const char *chargeInput)
{
	//SctmMessaging::Get().PrintWelcomingInformation();

	string prj(prjdir);
	string defaultParam(defaulParFile);
	string chargeInputFile(chargeInput);

	SctmMessaging::Get().PrintHeader("Initializing the simulator.");
	SctmGlobalControl::SetGlobalControl(defaultParam, prj, chargeInputFile);
	MaterialDB::SetMaterial_FromParFile();
	SctmPhys::SetPhysConstant();
}

void RunGateStack()
{
	FDDomain *aDomain = NULL;

	SctmTimer::Get().Set();
	SctmMessaging::Get().PrintHeader("Building gate stack.");
	aDomain = new GateStack();
	aDomain->BuildDomain();
	GateStackSolverPack gateSolver = GateStackSolverPack(aDomain);
	gateSolver.Solve();
}

int main(int argc, char* argv[])
{
	initialize(argv[1], argv[2], argv[3]);
	RunGateStack();
	return 0;
}