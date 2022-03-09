#include "GeneratorAction.h"

#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "g4csv.hh"
#include <G4GlobalMagFieldMessenger.hh>


GeneratorAction::GeneratorAction() : G4VUserPrimaryGeneratorAction()
{
  G4int nofParticles = 1;
  // Amendment keeping track of number of particles fired to know when to
  // increase energy and magnetic field values
  runCounter = 0;
  
  m_particleGun = new G4ParticleGun( nofParticles );
  m_fieldManager = new G4GlobalMagFieldMessenger(G4ThreeVector(0,0,0));
  // Default particle
  G4ParticleDefinition * particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle( "e-" );
  m_particleGun->SetParticleDefinition( particleDefinition );
  m_particleGun->SetParticlePosition( G4ThreeVector( 0.0, 0.0, -250.0*cm ) ); // right in the middle
  m_particleGun->SetParticleMomentumDirection( G4ThreeVector( 0.0, 0.0, 1.0 ) ); // along z axis
  m_particleGun->SetParticleEnergy( 200.0*MeV );
  
  
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
  double fieldStrength = m_fieldManager->GetFieldValue().getX();
  analysisManager->FillNtupleDColumn( 0, 0, particleEnergy );
  analysisManager->FillNtupleDColumn(0, 1, fieldStrength);
  
  // After a number of particles fired, increase energy and mag field
  runCounter++;
  if(runCounter % 10 == 0)
  {
    // every 10, increase the magnetic field
    m_fieldManager->SetFieldValue(G4ThreeVector(fieldStrength+=0.01*tesla,0,0));
    
  }
  if(runCounter % 100 == 0)
  {
    //every 100, increase the electric field and reset the magnetic field
    m_particleGun->SetParticleEnergy(particleEnergy+=10.0*MeV);
    m_fieldManager->SetFieldValue(G4ThreeVector(0,0,1E-9));
    std::cout<<"Ran for "<<runCounter<<" events."<<std::endl;
  }
    
  
}
