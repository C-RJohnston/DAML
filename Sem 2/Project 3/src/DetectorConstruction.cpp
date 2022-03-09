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
  G4Material* liquidArgon = nistManager->FindOrBuildMaterial( "G4_lAr" );
  // Amendment silicon
  G4Material* silicon = nistManager->FindOrBuildMaterial("G4_Si");

  // Sizes of the principal geometrical components (solids)
  G4double absorberThickness = 5.0*cm;
  G4double absorberRadius = 50.0*cm;
  G4double detectorThickness = 10.0*cm;
  G4double detectorRadius = 50.0*cm;
  G4double worldLength = 250.0*cm;
  // Amendment enclosing tube detector properties
  G4double detector2InnerRadius = 75.0*cm;
  G4double detector2OuterRadius = 85.0*cm;
  G4double detector2Thickness = (absorberThickness +
                                detectorThickness);

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
                 G4ThreeVector(0.0, 0.0, 0.0), // in the centre
                 worldLV,         // its logical volume
                 "World",         // its name
                 0,               // its mother volume
                 false,           // no boolean operations
                 0,               // copy number
                 true );          // checking overlaps

  // ABSORBER: Layer properties
  G4ThreeVector absorberPosition( 0, 0, absorberThickness );


  // ABSORBER: Solid (tube)
  G4Tubs* absorberS = new G4Tubs(
                 "Absorber",      // its name
                 0.0,               // inner radius 0, so it's a solid cylinder (not a hollow tube)
                 absorberRadius,    // outer radius
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
                 absorberPosition,  // where is it
                 absorberLV,        // its logical volume
                 "Absorber",      // its name
                 worldLV,           // its mother volume
                 false,             // no boolean operations
                 0,                 // copy number
                 true );            // checking overlaps

  // ABSORBER: Quit if there's an overlap
  if ( absorberPV->CheckOverlaps() ) std::cerr << "WARNING: your simulated objects overlap" << std::endl;

  // DETECTOR: Layer properties
  G4ThreeVector detectorPosition( 0, 0,
                                  2 * absorberThickness + detectorThickness );

  // DETECTOR: Solid (tube)
  G4Tubs* detectorS = new G4Tubs(
                 "Inner Detector",      // its name
                 0.0,               // inner radius 0, so it's a solid cylinder (not a hollow tube)
                 detectorRadius,    // outer radius
                 detectorThickness, // how much material in the beam path (half length)
                 0.0*deg,           // starting angle
                 360.0*deg );       // ending angle (i.e. it's a full circle)

  // DETECTOR: Logical volume (how to treat it)
  G4LogicalVolume* detectorLV = new G4LogicalVolume(
                 detectorS,         // its solid
                 liquidArgon,       // its material
                 "Inner Detector",      // its name
                 0, 0, 0 );         // Modifiers we don't use
  G4VisAttributes* detectorVisAtt = new G4VisAttributes(G4Colour(152.0/255.0, 80.0/255.0, 204.0/255.0, 0.6));
  detectorLV -> SetVisAttributes(detectorVisAtt);

  // DETECTOR: Physical volume (where is it)
  G4VPhysicalVolume* detectorPV = new G4PVPlacement(
                 0,                 // no rotation
                 detectorPosition,  // where is it
                 detectorLV,        // its logical volume
                 "Inner Detector",      // its name
                 worldLV,           // its mother volume
                 false,             // no boolean operations
                 0,                 // copy number
                 true );            // checking overlaps

  // DETECTOR: Warn if there's an overlap
  if ( detectorPV->CheckOverlaps() ) std::cerr << "WARNING: your simulated objects overlap" << std::endl;

  // Amendment create the enclosing tube around the calorimeter
  G4ThreeVector detector2Position(0, 0, detector2Thickness);
  G4Tubs* detector2S = new G4Tubs(
                                  "Outer Detector",
                                  detector2InnerRadius,
                                  detector2OuterRadius,
                                  detector2Thickness,
                                  0.0*deg,
                                  360.0*deg);
  G4LogicalVolume* detector2LV = new G4LogicalVolume(
                                                    detector2S,
                                                    silicon,
                                                    "Outer Detector",
                                                    0, 0, 0);
  G4VisAttributes* detector2VisAtt = new G4VisAttributes(G4Colour(223.0/255.0, 240.0/255.0, 240.0/255.0));
  detector2LV -> SetVisAttributes(detector2VisAtt);
  G4VPhysicalVolume* detector2PV = new G4PVPlacement(
                                                      0,
                                                      detector2Position,
                                                      detector2LV,
                                                      "Outer Detector",
                                                      worldLV,
                                                      false,
                                                      0,
                                                      true);
  
  // DETECTOR: Warn if there's an overlap
  if ( detector2PV->CheckOverlaps() ) {
    std::cerr << "WARNING: your simulated objects overlap" << std::endl;
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

  // Make "sensitive detectors" for the liquid argon layers
  auto argonDetector = new EnergyCounter( "Inner Detector", 2);
  G4SDManager::GetSDMpointer()->AddNewDetector( argonDetector );
  this->SetSensitiveDetector( "Inner Detector", argonDetector );
  // Amendment make sensitive detectors for the outer silicon layer
  auto siliconDetector = new EnergyCounter("Outer Detector", 3);
  G4SDManager::GetSDMpointer()->AddNewDetector(siliconDetector);
  this->SetSensitiveDetector("Outer Detector", siliconDetector);
}
