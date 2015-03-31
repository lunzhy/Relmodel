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

void initialize(const char *prjdir ="", const char *defaulParFile = "")
{
	//SctmMessaging::Get().PrintWelcomingInformation();

	string prj(prjdir);
	string defaultParam(defaulParFile);

	if (prj.empty())
	{
		SctmMessaging::Get().PrintHeader("Cleaning debug folder.");
		prj = SctmEnv::Get().DebugPrjPath;
		SctmPyCaller::PyClean(prj);
	}
	else
	{
		if (SctmEnv::IsWindows())
		{
			//clean the project in Windows
			SctmPyCaller::PyClean(prj);
		}
		if (SctmEnv::IsLinux())
		{
			SctmMessaging::Get().PrintHeader("Preparing project folder.");
			SctmPyCaller::PyPrepare(prj);//currently the preparation of project folders only exists in Linux
		}
	}

	if (defaultParam.empty())
	{
		defaultParam = SctmEnv::Get().DefaultParamPath;
	}

	SctmGlobalControl::SetGlobalControl(defaultParam, prj);

	SctmMessaging::Get().PrintHeader("Initializing the simulator.");
	SctmTimer::Get().Start();
	
	//the initialization of the simulation goes here
	//MaterialDB::SetMaterials_Directly();
	MaterialDB::SetMaterial_FromParFile();
	SctmPhys::SetPhysConstant();
}

void RunSolverPack()
{
	FDDomain *aDomain = NULL;
	if (SctmGlobalControl::Get().Structure == "TripleFull")
	{
		SctmMessaging::Get().PrintHeader("Building full triple-cell domain.");
		aDomain = new TripleCellsFull();
	}
	if (SctmGlobalControl::Get().Structure == "Single")
	{
		SctmMessaging::Get().PrintHeader("Building single-cell domain.");
		aDomain = new SimpleONO();
	}
	aDomain->BuildDomain();
	SolverPack aPack = SolverPack(aDomain);
	aPack.Run();
}


void RunGateStack()
{
	FDDomain *aDomain = NULL;
	SctmMessaging::Get().PrintHeader("Building gate stack.");
	aDomain = new GateStack();
	GateStackSolverPack gateSolver = GateStackSolverPack(aDomain);
	gateSolver.Solve();
}

int main(int argc, char* argv[])
{
	switch (argc)
	{
	case 1:
		initialize();
		break;
	case 2:
		initialize(argv[1]);
		break;
	case 3:
		initialize(argv[1], argv[2]);
		break;
	default:
		SctmMessaging::Get().PrintHeader("Argument Error");
		exit(0);
		break;
	}
	RunSolverPack();
	return 0;
}