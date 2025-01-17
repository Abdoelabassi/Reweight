//____________________________________________________________________________
/*!

\class    genie::rew::GReWeightINuke

\brief    Reweighting GENIE INTRANUKE/hA hadron transport model.

	  The reweighting code considers two sets of physics changes:
          * Change in the hadron mean free path \lambda, i.e. change in
            the total rescattering probability P_{rescat}.
          * Changes in probabilty for rescattering mode X, given a fixed
            total rescattering probability P(X | \lambda).
            X = {elastic, inelastic, charge exchange, pion production,
                 absorption + multi-nucleon knockout}.

          Physics changes are considered separately for pions and nucleons.
          Unitarity is explicitly conserved.

\author   Jim Dobson <J.Dobson07 \at imperial.ac.uk>
          Imperial College London

          Costas Andreopoulos <costas.andreopoulos \at stfc.ac.uk>
          University of Liverpool & STFC Rutherford Appleton Lab

\created  Sep 10, 2009

\cpright  Copyright (c) 2003-2018, The GENIE Collaboration
          For the full text of the license visit http://copyright.genie-mc.org
*/
//____________________________________________________________________________

#ifndef _G_REWEIGHT_INUKE_H_
#define _G_REWEIGHT_INUKE_H_

//#define _G_REWEIGHT_INUKE_DEBUG_NTP_

// GENIE/Reweight includes
#include "RwCalculators/GReWeightModel.h"
#include "RwCalculators/GReWeightINukeParams.h"

using namespace genie::rew;
using namespace genie;

class TFile;
class TNtuple;
class TLorentzVector;

namespace genie {

 class HAIntranuke2018;

namespace rew   {

 class GReWeightINuke : public GReWeightModel
 {
 public:
   GReWeightINuke();
  ~GReWeightINuke();

   // implement the GReWeightI interface
   bool   AppliesTo      (ScatteringType_t type, bool is_cc) const;
   bool   IsHandled      (GSyst_t syst) const;
   void   SetSystematic  (GSyst_t syst, double val);
   void   Reset          (void);
   void   Reconfigure    (void);
   double CalcWeight     (const EventRecord & event);

 private:

   void CalcDeltaAZ( const EventRecord& event, const GHepParticle& p,
     int& deltaA, int& deltaZ );

   void UpdateRemnantAZ( int deltaA, int deltaZ );

   GReWeightINukeParams fINukeRwParams;

   HAIntranuke2018* fFSIModel;

#ifdef _G_REWEIGHT_INUKE_DEBUG_NTP_
   TFile *              fTestFile;
   TNtuple *            fTestNtp;
#endif

 };

} // rew
} // genie

#endif
