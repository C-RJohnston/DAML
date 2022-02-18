# Local
#GEANT4_myPath=/home/ben/Software/geant/geant4.10.04.p02
#source $GEANT4_myPath/share/Geant4-10.4.2/geant4make/geant4make.sh

# CPlab
source /Disk/opt/ubuntu/geant4.10.06.p02/bin/geant4.sh

#HEPMC
export HEPMC_DIR=$PWD/HEPmc/install
if [[ ! -e "${HEPMC_DIR}" ]]; then
    cd HEPmc
    source HEPMCinstall.sh
    cd ..
fi
