#include "RunAction.h"

#include "g4csv.hh"
#include "Consts.h"

RunAction::RunAction() : G4UserRunAction()
{
  
  // Create analysis manager
  auto analysisManager = G4AnalysisManager::Instance();

  // Add an ntuple for energy deposits (ntuple id 0)
  analysisManager->CreateNtuple( "Energy", "Deposited energy");
  analysisManager->CreateNtupleDColumn( "Generated" );
  analysisManager->CreateNtupleDColumn("Magnetic field");
  // Add a column for each layer of the detector
  int numLayers = (int) radius / detectorRingWidth;
  for(int layer = 1; layer<=numLayers; layer++)
  {
    analysisManager->CreateNtupleDColumn("Detector"+std::to_string(layer));
  }
  analysisManager->FinishNtuple();
}

RunAction::~RunAction()
{
  // Delete analysis manager
  delete G4AnalysisManager::Instance();
}

void RunAction::BeginOfRunAction( const G4Run* )
{
  // Get analysis manager
  auto analysisManager = G4AnalysisManager::Instance();

  // Open an output file
  analysisManager->OpenFile( "output.csv" );
}

void RunAction::EndOfRunAction( const G4Run* )
{
  // Get analysis manager
  auto analysisManager = G4AnalysisManager::Instance();

  // Save output data
  analysisManager->Write();
  analysisManager->CloseFile();
}
