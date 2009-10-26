//____________________________________________________________________________
/* !

\program gvld_hadronization_medium_effect_test

\brief   Compares the GENIE 

         Syntax:
           gvld_hadronization_medium_effect_test [-g genie_inputs]

         Options:

           [] Denotes an optional argument.

           -g An XML file with GENIE inputs (event samples)
              Multiple models can be included in that file, each identified 
              by a "name" (all model predictions will be overlayed).

              <?xml version="1.0" encoding="ISO-8859-1"?>
              <vld_inputs>
                 <model name="a_model_name">
                   <evt_file format="gst"> /path/model_1/evtfile0.root </evt_file>
                   <evt_file format="gst"> /path/model_1/evtfile1.root </evt_file>
                   <evt_file format="gst"> /path/model_1/evtfile2.root </evt_file>
                   ...
                 </model>

                 <model name="another_model_name">
                   <evt_file format="gst"> /path/model_2/evtfile0.root </evt_file>
                   <evt_file format="gst"> /path/model_2/evtfile1.root </evt_file>
                   <evt_file format="gst"> /path/model_2/evtfile2.root </evt_file>
                   ...
                 </model>
                 ...
              </vld_inputs>

         Notes:
           * The input event files are `gst' summary ntuples generated by 
             GENIE gntpc utility.
             See the GENIE Physics & User Manual for more details.
             The files will be chained together. 
             They should contain sufficient statistics of the following samples:
                -  ?
             The samples should be generated with an ? energy spectrum over 
             ? GeV - ? GeV.
           * A script for preparing inputs for this benchmark test can be 
             found in:
             $GENIE/src/scripts/production/rutherford_tier2/?.pl
             The script has been developed for the RAL/Tier2 farm. 
             Modify it accordingly for your own system.
             
\author  Athans Hatzikoutelis <a.hatzikoutelis \at lancaster.ac.uk>
         Lancaster University

         Costas Andreopoulos <costas.andreopoulos \at stfc.ac.uk>
         STFC, Rutherford Appleton Laboratory

\created June 06, 2008 

\cpright Copyright (c) 2003-2009, GENIE Neutrino MC Generator Collaboration
         For the full text of the license visit http://copyright.genie-mc.org
         or see $GENIE/LICENSE
*/
//____________________________________________________________________________

#include <cstdlib>
#include <cassert>
#include <sstream>
#include <string>

#include <TSystem.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TGraph.h>
#include <TPostScript.h>
#include <TH1D.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TPavesText.h>
#include <TText.h>
#include <TLatex.h>
#include <TStyle.h>
#include <TLegend.h>
#include <TChain.h>

#include "Conventions/GBuild.h"
#include "Messenger/Messenger.h"
#include "PDG/PDGUtils.h"
#include "PDG/PDGCodes.h"
#include "Utils/CmdLineArgParserUtils.h"
#include "Utils/CmdLineArgParserException.h"
#include "Utils/StringUtils.h"
#include "Utils/VldTestInputs.h"

using std::ostringstream;
using std::string;

using namespace genie;
using namespace genie::utils::vld;

/* 
..............................................................................
DATA
..............................................................................
ID   DESCRIPTION
 0   bla bla
..............................................................................
*/
const int    kNDataSets = 1;
const char * kDataSetLabel[kNDataSets] = {
/* 0 */ "bla bla",
//
// add more
//
};
const char * kDataSetXAxisLabel[kNDataSets] = {
/* 0 */ "??",
//
// add more
//
};
const char * kDataSetYAxisLabel[kNDataSets] = {
/* 0 */ "??",
//
// add more
//
};

// function prototypes
void     Init               (void);
void     Run                (void);
void     End                (void);
void     AddCoverPage       (void);
TGraph * Data               (int iset);
TGraph * Model              (int iset, int imodel);
void     Draw               (int iset);
void     Format             (TGraph* gr, int lcol, int lsty, int lwid, int mcol, int msty, double msiz);
void     GetCommandLineArgs (int argc, char ** argv);
void     PrintSyntax        (void);

// command-line arguments
VldTestInputs  gOptGenieInputs;

// globals
TPostScript * gPS = 0;
TCanvas *     gC  = 0;

// plotting consts

const int kNCx = 1; // number of columns in TCanvas::Divide()
const int kNCy = 1; // number of rows    in TCanvas::Divide()

// model line styles
const int kNMaxNumModels = 5;
const int kLStyle    [kNMaxNumModels] = { 
   1,       2,        3,        5,            6 
}; 
string    kLStyleTxt [kNMaxNumModels] = { 
  "solid", "dashed", "dotted", "dot-dashed", "dot-dot-dashed" 
};

//_________________________________________________________________________________
int main(int argc, char ** argv)
{
  GetCommandLineArgs (argc,argv);

  Init();
  Run ();
  End ();

  LOG("gvldtest", pINFO)  << "Done!";
  return 0;
}
//_________________________________________________________________________________
void Run(void)
{
  // loop over data sets
  for(int iset = 0; iset < kNDataSets; iset++) 
  {
    Draw(iset);
  }
}
//_________________________________________________________________________________
void Init(void)
{
  LOG("vldtest", pNOTICE) << "Initializing...";;

  gC = new TCanvas("c","",20,20,500,650);
  gC->SetBorderMode(0);
  gC->SetFillColor(0);
  gC->SetGridx();
  gC->SetGridy();

  // output file
  gPS = new TPostScript("genie_hadronization_medium_effects.ps", 111);

  AddCoverPage();

  gC->SetLogx();
  gC->SetLogy();
}
//_________________________________________________________________________________
void AddCoverPage(void)
{
  // header
  gPS->NewPage();
  gC->Range(0,0,100,100);
  TPavesText hdr(10,40,90,70,3,"tr");
  hdr.AddText(" ");
  hdr.AddText("GENIE Hadronization, Medium Effects: Comparisons with data");
  hdr.AddText(" ");
  hdr.AddText(" ");
  for(int imodel=0; imodel< gOptGenieInputs.NModels(); imodel++) {
    ostringstream stream;
    stream << "model tag: " << gOptGenieInputs.ModelTag(imodel)
           << " (" << kLStyleTxt[imodel] << " line)";
    hdr.AddText(stream.str().c_str());
  }
  hdr.AddText(" ");
  hdr.Draw();
  gC->Update();
}
//_________________________________________________________________________________
void End(void)
{
  LOG("vldtest", pNOTICE) << "Cleaning up...";

  gPS->Close();

  delete gC;
  delete gPS;
}
//_________________________________________________________________________________
// Corresponding GENIE prediction for the `iset' data set 
//.................................................................................
TGraph * Model(int iset, int imodel)
{
  LOG("vldtest", pNOTICE) 
    << "Getting GENIE prediction (model ID = " 
    << imodel << ", data set ID = " << iset << ")";

  TChain * event_chain = gOptGenieInputs.EvtChain(imodel);
  if(!event_chain) {
     LOG("vldtest", pNOTICE) 
        << "No corresponding event chain.";
     return 0;
  }

  switch(iset) {

    case (0) :
    {
      //
      // do calculation
      //
      break;
    }

    case (1) :
    {
      //
      // do calculation
      //
      break;
    }

    //
    // add more
    //

    default:
    {
       return 0;
    }
  }

  return 0;
}
//_________________________________________________________________________________
TGraph * Data(int iset)
{
  LOG("vldtest", pNOTICE) 
    << "Loading experimental data set ID = " << iset;

  return 0;
}
//_________________________________________________________________________________
void Draw(int iset)
{
  // get data for the current comparison
  TGraph * data = Data(iset);

  // get the corresponding GENIE model prediction
  vector<TGraph*> models;
  for(int imodel=0; imodel< gOptGenieInputs.NModels(); imodel++) {
     models.push_back(Model(iset,imodel));
  }

  if(models.size()==0 && !data) return;

  int plots_per_page = kNCx * kNCy;
  int iplot = 1 + iset % plots_per_page;
    
  if(iplot == 1) {   
     gPS->NewPage();
     gC -> Clear();
     gC -> Divide(kNCx,kNCy);
  }
  
  gC -> GetPad(iplot) -> Range(0,0,100,100);
  gC -> GetPad(iplot) -> SetFillColor(0);
  gC -> GetPad(iplot) -> SetBorderMode(0);
  gC -> GetPad(iplot) -> cd();

  double xmin = 0.0, scale_xmin = 0.5;
  double xmax = 0.0, scale_xmax = 1.2;
  double ymin = 0.0, scale_ymin = 0.4;
  double ymax = 0.0, scale_ymax = 1.2;
       
  TH1F * hframe = 0;
  bool have_frame = false;

  // have data points to plot?
  if(data) {
    // create frame from the data point range
    xmin  = ( data->GetX() )[TMath::LocMin(data->GetN(),data->GetX())];
    xmax  = ( data->GetX() )[TMath::LocMax(data->GetN(),data->GetX())];
    ymin  = ( data->GetY() )[TMath::LocMin(data->GetN(),data->GetY())];
    ymax  = ( data->GetY() )[TMath::LocMax(data->GetN(),data->GetY())];
    hframe = (TH1F*) gC->GetPad(iplot)->DrawFrame(
        scale_xmin*xmin, scale_ymin*ymin, scale_xmax*xmax, scale_ymax*ymax);
    hframe->Draw();
    have_frame = true;

    // draw current data set
    data->Draw("P");
  }//dbtable?

  // have model prediction to plot?
  if(models.size()>0) {
     if(!have_frame) {
        // the data points have not been plotted
        // create a frame from this graph range
        xmin  = ( models[0]->GetX() )[TMath::LocMin(models[0]->GetN(),models[0]->GetX())];
        xmax  = ( models[0]->GetX() )[TMath::LocMax(models[0]->GetN(),models[0]->GetX())];
        ymin  = ( models[0]->GetY() )[TMath::LocMin(models[0]->GetN(),models[0]->GetY())];
        ymax  = ( models[0]->GetY() )[TMath::LocMax(models[0]->GetN(),models[0]->GetY())];
        hframe = (TH1F*) gC->GetPad(1)->DrawFrame(
           scale_xmin*xmin, scale_ymin*ymin, scale_xmax*xmax, scale_ymax*ymax);
        hframe->Draw();
     }
     for(int imodel=0; imodel<gOptGenieInputs.NModels(); imodel++) {
       TGraph * plot = models[imodel];
       if(plot) {
         int lsty = kLStyle[imodel];     
         Format(plot,1,lsty,2,1,1,1);
         plot->Draw("L");
       }
     }
  }//model?

  // axes label
  hframe->GetXaxis()->SetTitle(kDataSetXAxisLabel[iset]);
  hframe->GetYaxis()->SetTitle(kDataSetYAxisLabel[iset]);

  // title
  double xtitle = scale_xmin*xmin + 0.2*(scale_xmax*xmax-scale_xmin*xmin);
  double ytitle = 1.01*scale_ymax*ymax; 
  TLatex * title = new TLatex(xtitle, ytitle, kDataSetLabel[iset]);
  title->SetTextSize(0.022);
  title->Draw();

  // update    
  gC->GetPad(iplot)->Update();
  gC->Update();
}
//_________________________________________________________________________________
// Formatting
//.................................................................................
void Format(
    TGraph* gr, int lcol, int lsty, int lwid, int mcol, int msty, double msiz)
{
  if(!gr) return;

  if (lcol >= 0) gr -> SetLineColor   (lcol);
  if (lsty >= 0) gr -> SetLineStyle   (lsty);
  if (lwid >= 0) gr -> SetLineWidth   (lwid);

  if (mcol >= 0) gr -> SetMarkerColor (mcol);
  if (msty >= 0) gr -> SetMarkerStyle (msty);
  if (msiz >= 0) gr -> SetMarkerSize  (msiz);
}
//_________________________________________________________________________________
// Parsing command-line arguments, check/form filenames, etc
//.................................................................................
void GetCommandLineArgs(int argc, char ** argv)
{
  LOG("gvldtest", pNOTICE) << "*** Parsing command line arguments";

  // get GENIE inputs
  try {
     string inputs = utils::clap::CmdLineArgAsString(argc,argv,'g');
     bool ok = gOptGenieInputs.LoadFromFile(inputs);
     if(!ok) {
        LOG("gvldtest", pFATAL) << "Could not read: " << inputs;
        exit(1);
     }
  } catch(exceptions::CmdLineArgParserException e) {
     if(!e.ArgumentFound()) {

     }
  }
}
//_________________________________________________________________________________
void PrintSyntax(void)
{
  LOG("gvldtest", pNOTICE)
    << "\n\n" << "Syntax:" << "\n"
    << "   gvld_hadronization_medium_effect_test [-g genie_inputs.xml]\n";
}
//_________________________________________________________________________________

