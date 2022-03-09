#include "GeneratorAction.h"

#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "g4csv.hh"

GeneratorAction::GeneratorAction() : G4VUserPrimaryGeneratorAction()
{
  G4int nofParticles = 1;
  // Amendment keeping track of number of particles fired to know when to
  // increase energy and magnetic field values
  runCounter = 0;
  
  m_particleGun = new G4ParticleGun( nofParticles );

  // Default particle
  G4ParticleDefinition * particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle( "e-" );
  m_particleGun->SetParticleDefinition( particleDefinition );
  m_particleGun->SetParticlePosition( G4ThreeVector( 0.0, 0.0, -250.0*cm ) ); // right in the middle
  m_particleGun->SetParticleMomentumDirection( G4ThreeVector( 0.0, 0.0, 1.0 ) ); // along z axis
  m_particleGun->SetParticleEnergy( 200.0*MeV );
  // Amendment set up magnetic field
  magField = new G4UniformMagField(G4ThreeVector(0.1*tesla, 0.0, 0.0));
  
  G4FieldManager* fieldMgr
    = G4TransportationManager::GetTransportationManager()
    ->GetFieldManager();
  
  fieldMgr->SetDetectorField(magField);
}

GeneratorAction::~GeneratorAction()
{
  delete m_particleGun;
}

// This function is called at the begining of event
void GeneratorAction::GeneratePrimaries( G4Event* anEvent )
{
  // Fire a particle
  m_particleGun->GeneratePrimaryVertex( anEvent );
  
  // Store truth information - first column
  auto analysisManager = G4AnalysisManager::Instance();
  G4double particleEnergy = m_particleGun->GetParticleEnergy();
  G4double fieldStrength = magField->GetConstantFieldValue().x();
  analysisManager->FillNtupleDColumn( 0, 0, particleEnergy );
  analysisManager->FillNtupleDColumn(0, 1, fieldStrength);
  
  // After a number of particles fired, increase energy and mag field
  runCounter++;
  if(runCounter % 10 == 0)
  {
    m_particleGun->SetParticleEnergy(particleEnergy+=10.0*MeV);
    magField->SetFieldValue(G4ThreeVector(fieldStrength+=0.05*tesla, 0.0, 0.0));
  }
  
}
