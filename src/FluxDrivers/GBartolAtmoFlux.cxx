//____________________________________________________________________________
/*
 Copyright (c) 2003-2007, GENIE Neutrino MC Generator Collaboration
 For the full text of the license visit http://copyright.genie-mc.org
 or see $GENIE/LICENSE

 Author: Christopher Backhouse <c.backhouse1@physics.ox.ac.uk>
         Oxford University, January 26, 2008

 For the class documentation see the corresponding header file.

*/
//____________________________________________________________________________

#include <fstream>

#include <TH2D.h>
#include <TMath.h>

#include "Conventions/Constants.h"
#include "FluxDrivers/GBartolAtmoFlux.h"
#include "Messenger/Messenger.h"
#include "Numerical/RandomGen.h"
#include "PDG/PDGCodeList.h"
#include "PDG/PDGCodes.h"
#include "Utils/PrintUtils.h"

using std::ifstream;
using std::ios;
using namespace genie;
using namespace genie::flux;
using namespace genie::constants;

//____________________________________________________________________________
GBartolAtmoFlux::GBartolAtmoFlux()
{
  this->Initialize();
}
//___________________________________________________________________________
GBartolAtmoFlux::~GBartolAtmoFlux()
{
  this->CleanUp();
}
//___________________________________________________________________________
bool GBartolAtmoFlux::GenerateNext(void)
{
  //-- Reset previously generated neutrino code / 4-p / 4-x
  this->ResetSelection();

  //-- Get a RandomGen instance
  RandomGen * rnd = RandomGen::Instance();

  //-- Generate a (Ev, costheta) pair from the 'combined' flux histogram
  //   and select (phi) uniformly over [0,2pi]
  Axis_t aEv   = 0;
  Axis_t aCos8 = 0;
  fFluxSum2D->GetRandom2( aEv, aCos8);
  double Ev   = (double)aEv;
  double cos8 = (double)aCos8;
  double phi  = 2.*kPi* rnd->RndFlux().Rndm();

  //-- etc trigonometric numbers
  double sin8   = TMath::Sqrt(1-cos8*cos8);
  double cosphi = TMath::Cos(phi);
  double sinphi = TMath::Sin(phi);

  //-- Select a neutrino species from the flux fractions at the selected
  //   (Ev,costtheta) pair

  fgPdgC = (*fPdgCList)[this->SelectNeutrino(Ev, cos8)];

  //-- Compute the neutrino 4-p
  //   (the - means it is directed towards the detector)
  double pz = -1.* Ev * cos8;
  double py = -1.* Ev * sin8 * cosphi;
  double px = -1.* Ev * sin8 * sinphi;

  fgP4.SetPxPyPzE(px, py, pz, Ev);

  //-- Compute neutrino 4-x

  // compute its position at the surface of a sphere with R=fRl
  double z = fRl * cos8;
  double y = fRl * sin8 * cosphi;
  double x = fRl * sin8 * sinphi;

  // If the position is left as is, then all generated neutrinos
  // would point towards the origin.
  // Displace the position randomly on the surface that is
  // perpendicular to the selected point P(xo,yo,zo) on the sphere

  TVector3 vec(x,y,z);              // vector towards selected point
  TVector3 dvec = vec.Orthogonal(); // orthogonal vector

  double psi = 2.*kPi* rnd->RndFlux().Rndm(); // rndm angle [0,2pi]
  double Rt  = fRt* rnd->RndFlux().Rndm();    // rndm norm  [0,Rtransverse]

  dvec.Rotate(psi,vec); // rotate around original vector
  dvec.SetMag(Rt);      // set new norm

  // displace the original vector & set the neutrino 4-position
  x += dvec.X();
  y += dvec.Y();
  z += dvec.Z();

  fgX4.SetXYZT(x,y,z,0.);

  LOG("Flux", pINFO) << "Generated neutrino pdg-code: " << fgPdgC;
  LOG("Flux", pINFO)
        << "Generated neutrino p4: " << utils::print::P4AsShortString(&fgP4);
  LOG("Flux", pINFO)
             << "Generated neutrino x4: " << utils::print::X4AsString(&fgX4);

  return true;
}
//___________________________________________________________________________
void GBartolAtmoFlux::Initialize(void)
{
  LOG("Flux", pNOTICE) << "Initializing GBartolAtmoFlux driver";

  // setting maximum energy in flux files
  fMaxEv = kGBrtEv[kNGBrtEv-1];

  // setting neutrino-types in flux files
  PDGCodeList::size_type nnu = 4;
  fPdgCList = new PDGCodeList(nnu);
  (*fPdgCList)[0]= kPdgNuMu;
  (*fPdgCList)[1]= kPdgAntiNuMu;
  (*fPdgCList)[2]= kPdgNuE;
  (*fPdgCList)[3]= kPdgAntiNuE;

  // Filenames for flux files [you need to get them from the web
  // - see class documentation]
  // You need to Set...() them before LoadFluxData()
  // You do not have to set all 4 if you need one flux component skipped
  // (but you do have to set at least one)
  fNSkipped  = 0;
  for (unsigned int iflux=0; iflux<kNNu; iflux++) fFluxFile[iflux] = "";

  // initializing flux TH2D histos [ flux = f(Ev,costheta) ]
  //fFlux2D = new TH2D[kNNu];
  for (unsigned int iflux=0; iflux<kNNu; iflux++) fFlux2D[iflux] = 0;
  fFluxSum2D = 0;

  this->ResetSelection();
}
//___________________________________________________________________________
void GBartolAtmoFlux::ResetSelection(void)
{
// initializing running neutrino pdg-code, 4-position, 4-momentum
  fgPdgC = 0;
  fgP4.SetPxPyPzE (0.,0.,0.,0.);
  fgX4.SetXYZT    (0.,0.,0.,0.);
}
//___________________________________________________________________________
void GBartolAtmoFlux::CleanUp(void)
{
  LOG("Flux", pNOTICE) << "Cleaning up...";

  for(unsigned int iflux = 0;
        iflux < kNNu; iflux++) { if (fFlux2D[iflux]) delete fFlux2D[iflux]; }
  if (fFluxSum2D) delete fFluxSum2D;
  if (fPdgCList ) delete fPdgCList;
}
//___________________________________________________________________________
void GBartolAtmoFlux::SetRadii(double Rlongitudinal, double Rtransverse)
{
  LOG ("Flux", pNOTICE) << "Setting R[longitudinal] = " << Rlongitudinal;
  LOG ("Flux", pNOTICE) << "Setting R[transverse]   = " << Rtransverse;

  fRl = Rlongitudinal;
  fRt = Rtransverse;
}
//___________________________________________________________________________
bool GBartolAtmoFlux::LoadFluxData(void)
{
  LOG("Flux", pNOTICE) << "Creating Flux = f(Ev,cos8z) 2-D histograms";

  fFlux2D[0] = this->CreateFluxHisto2D("numu",   "GBartol 3D flux: numu"   );
  fFlux2D[1] = this->CreateFluxHisto2D("numubar","GBartol 3D flux: numubar");
  fFlux2D[2] = this->CreateFluxHisto2D("nue",    "GBartol 3D flux: nue"    );
  fFlux2D[3] = this->CreateFluxHisto2D("nuebar", "GBartol 3D flux: nuebar" );

  LOG("Flux", pNOTICE)
          << "Loading GBartol 3-D Atmo. data";

  bool loading_status = true;
  for(unsigned int iflux = 0; iflux < kNNu; iflux++) {
    bool loaded = this->FillFluxHisto2D(fFlux2D[iflux], fFluxFile[iflux]);
    loading_status = loading_status && loaded;
  }
  if(loading_status) {
     LOG("Flux", pNOTICE) << "GBartol Atmo. Simulation Data Loaded";
     this->AddAllFluxes();
     return true;
  }
  LOG("Flux", pERROR) << "Error in loading GBartol Simulation Data";
  return false;
}
//___________________________________________________________________________
bool GBartolAtmoFlux::FillFluxHisto2D(TH2D * histo, string filename)
{
  LOG("Flux", pNOTICE) << "Loading: " << filename;

  if(!histo) {
     LOG("Flux", pERROR) << "The flux histo to fill is not instantiated!";
     return false;
  }
  if(filename.size() == 0) {
     // the user wants to skip this flux component - create an empty flux
     this->ZeroFluxHisto2D(histo);
     fNSkipped++;
     assert(fNSkipped<4); // you must load at least one flux file
     return true;
  }

  ifstream flux_stream(filename.c_str(), ios::in);
  if(!flux_stream) {
     LOG("Flux", pERROR) << "Could not open file: " << filename;
     return false;
  }

  double energy, costheta, flux;
  double junkd; // throw away error estimates

  // throw away comment line
  flux_stream.ignore(99999, '\n');

  const int kNLines = kNGBrtCos * kNGBrtEv;
  int iline = 0;
  while (++iline<=kNLines) {
    flux_stream >> energy >> costheta >> flux >> junkd >> junkd;
    // Compensate for logarithmic units - dlogE=dE/E
    flux /= energy;
    LOG("Flux", pDEBUG)
      << "Flux[Ev = " << energy << ", cos8 = " << costheta << "] = " << flux;
    histo->Fill( (Axis_t)energy, (Axis_t)costheta, (Stat_t)flux);
  }
  return true;
}
//___________________________________________________________________________
void GBartolAtmoFlux::ZeroFluxHisto2D(TH2D * histo)
{
  LOG("Flux", pNOTICE) << "Forcing flux histogram contents to 0";

  for(unsigned int ie = 0; ie < kNGBrtEv; ie++) {
    for(unsigned int ic = 0; ic < kNGBrtCos; ic++) {
       double energy   = kGBrtEv[ie]  - 1.E-4;
       double costheta = kGBrtCos[ic] - 1.E-4;
       histo->Fill(energy,costheta,0.);
    }
  }
}
//___________________________________________________________________________
void GBartolAtmoFlux::AddAllFluxes(void)
{
  LOG("Flux", pNOTICE) << "Computing combined flux";

  fFluxSum2D = this->CreateFluxHisto2D("sum", "combined flux" );

  for(unsigned int iflux=0;
           iflux<kNNu; iflux++) fFluxSum2D->Add(fFlux2D[iflux]);
}
//___________________________________________________________________________
TH2D * GBartolAtmoFlux::CreateFluxHisto2D(string name, string title)
{
  LOG("Flux", pNOTICE) << "Instantiating histogram: [" << name << "]";
  TH2D * h2 = new TH2D(name.c_str(), title.c_str(),
                           kNGBrtEv-1,kGBrtEv, kNGBrtCos-1,kGBrtCos);
  return h2;
}
//___________________________________________________________________________
int GBartolAtmoFlux::SelectNeutrino(double Ev, double costheta)
{
  double flux[kNNu];
  for(unsigned int iflux=0; iflux<kNNu; iflux++) {
     int ibin = fFlux2D[iflux]->FindBin(Ev,costheta);
     flux[iflux] = fFlux2D[iflux]->GetBinContent(ibin);
  }
  double flux_sum = 0;
  for(unsigned int iflux=0; iflux<kNNu; iflux++) {
     flux_sum   += flux[iflux];
     flux[iflux] = flux_sum;
     LOG("Flux", pINFO) << "SUM-FLUX(0->" << iflux <<") = " << flux[iflux];
  }

  RandomGen * rnd = RandomGen::Instance();
  double R = flux_sum * rnd->RndFlux().Rndm();

  LOG("Flux", pINFO) << "R = " << R;

  for(unsigned int iflux = 0; iflux < kNNu; iflux++) {
     if( R < flux[iflux] ) return iflux;
  }

  LOG("Flux", pERROR) << "Could not select a neutrino species";
  assert(false);

  return -1;
}
//___________________________________________________________________________

