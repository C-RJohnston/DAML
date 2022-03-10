#include "DetectorConstruction.h"
#include "EnergyCounter.h"

#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4AutoDelete.hh"
#include "G4GeometryManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4SDManager.hh"
#include <G4VisAttributes.hh>
#include <G4Colour.hh>
#include "Consts.h"

G4ThreadLocal
G4GlobalMagFieldMessenger* DetectorConstruction::m_magneticFieldMessenger = 0;

DetectorConstruction::DetectorConstruction() : G4VUserDetectorConstruction()
{
}

DetectorConstruction::~DetectorConstruction()
{
}

// Here we define the actual experiment that we want to perform
G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // Materials
  // http://geant4-userdoc.web.cern.ch/geant4-userdoc/UsersGuides/ForApplicationDeveloper/html/Appendix/materialNames.html
  G4NistManager* nistManager = G4NistManager::Instance();
  G4Material* vacuum = nistManager->FindOrBuildMaterial( "G4_Galactic" );
  G4Material* lead = nistManager->FindOrBuildMaterial( "G4_Pb" );
  // Amendment silicon
  G4Material* silicon = nistManager->FindOrBuildMaterial("G4_Si");

  // Sizes of the principal geometrical components (solids)
  G4double absorberThickness = 0.1*cm;
  G4double detectorThickness = 5.0*cm;
  G4double worldLength = 250.0*cm;

  // Definitions of Solids, Logical Volumes, Physical Volumes

  // WORLD: Solid (cube)
  G4GeometryManager::GetInstance()->SetWorldMaximumExtent( worldLength );
  G4Box* worldS = new G4Box(
                 "World",         // its name
                 worldLength,
                 worldLength,
                 worldLength );   // its size (in half-lengths)

  // WORLD: Logical volume (how to treat it)
  G4LogicalVolume* worldLV = new G4LogicalVolume(
                 worldS,          // its solid
                 vacuum,          // its material
                 "World" );       // its name

  // WORLD: Physical volume (where is it)
  // Must place the World Physical volume unrotated at (0,0,0).
  G4VPhysicalVolume* worldPV = new G4PVPlacement(
                 0,               // no rotation
                 G4ThreeVector(0.0, 0.0, 0.0),
                 worldLV,         // its logical volume
                 "World",         // its name
                 0,               // its mother volume
                 false,           // no boolean operations
                 0,               // copy number
                 true );          // checking overlaps

  // ABSORBER: Layer properties
  // ABSORBER: Solid (tube)
  G4Tubs* absorberS = new G4Tubs(
    "Absorber",      // its name
    0.0,               // inner radius 0, so it's a solid cylinder (not a hollow tube)
    radius,    // outer radius
    absorberThickness, // how much material in the beam path (half length)
    0.0*deg,           // starting angle
    360.0*deg );       // ending angle (i.e. it's a full circle)

  // ABSORBER: Logical volume (how to treat it)
  G4LogicalVolume* absorberLV = new G4LogicalVolume(
    absorberS,         // its solid
    lead,              // its material
    "Absorber",      // its name
    0, 0, 0 );         // Modifiers we don't use
  G4VisAttributes* absorberVisAtt = new G4VisAttributes(G4Colour(131.0/255.0, 136.0/255.0, 145.0/255.0));
  absorberLV -> SetVisAttributes(absorberVisAtt);

  // ABSORBER: Physical volume (where is it)
  G4VPhysicalVolume* absorberPV = new G4PVPlacement(
    0,                 // no rotation
    G4ThreeVector(0,0,-(detectorThickness+absorberThickness)),  // where is it
    absorberLV,        // its logical volume
    "Absorber",      // its name
    worldLV,           // its mother volume
    false,             // no boolean operations
    0,                 // copy number
    true );            // checking overlaps

  // ABSORBER: Quit if there's an overlap
  if ( absorberPV->CheckOverlaps() ) std::cerr << "WARNING: your simulated objects overlap" << std::endl;

  layerNum = 0;
  // Detector
  for(G4double outerRadius = detectorRingWidth;
      outerRadius <= radius; outerRadius += detectorRingWidth)
  {
    layerNum++;
    std::string detectorName = "Detector" + std::to_string(layerNum);
    G4Tubs* detectorS = new G4Tubs(
      detectorName,
      detectorRingWidth * (layerNum -1),
      outerRadius,
      detectorThickness,
      0.0*deg,
      360.0*deg);

    G4LogicalVolume* detectorLV = new G4LogicalVolume(
      detectorS,
      silicon,
      detectorName,
      0,0,0);
    G4VisAttributes* detectorVisAtt = new G4VisAttributes(G4Colour(outerRadius/radius,
                                                                  outerRadius/radius,
                                                                  outerRadius/radius));
    detectorLV-> SetVisAttributes(detectorVisAtt);

    G4VPhysicalVolume* detectorPV = new G4PVPlacement(
      0,
      G4ThreeVector(0,0,0),
      detectorLV,
      detectorName,
      worldLV,
      false,
      0,
      true);

    // DETECTOR: Warn if there's an overlap
    if ( detectorPV->CheckOverlaps() ) std::cerr << "WARNING: your simulated objects overlap" << std::endl;
  }

  // Always return the physical world
  return worldPV;
}

// Set up the magnetic field
void DetectorConstruction::ConstructSDandField()
{
  // Create global magnetic field messenger.
  // Uniform magnetic field is then created automatically if
  // the field value is not zero.
  G4ThreeVector fieldValue = G4ThreeVector();
  m_magneticFieldMessenger = new G4GlobalMagFieldMessenger( fieldValue );

  // Register the field messenger for deleting
  G4AutoDelete::Register( m_magneticFieldMessenger );

  // Amendment make sensitive detectors for the outer silicon layer
  for(int layer = 1; layer<=layerNum; layer++)
  {
    std::string detectorName = "Detector" + std::to_string(layer);
    auto detector = new EnergyCounter(detectorName, layer);
    G4SDManager::GetSDMpointer()->AddNewDetector(detector);
    this->SetSensitiveDetector(detectorName, detector);
  }
}
