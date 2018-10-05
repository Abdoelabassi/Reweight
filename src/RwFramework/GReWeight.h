//____________________________________________________________________________
/*!

\class    genie::rew::GReWeight

\brief    Interface to the GENIE event reweighting engines

\author   Jim Dobson <J.Dobson07 \at imperial.ac.uk>
          Imperial College London

          Costas Andreopoulos <costas.andreopoulos \at stfc.ac.uk>
          University of Liverpool & STFC Rutherford Appleton Lab

\created  Aug 1, 2009

\cpright  Copyright (c) 2003-2018, The GENIE Collaboration
          For the full text of the license visit http://copyright.genie-mc.org
*/
//____________________________________________________________________________

#ifndef _G_REWEIGHT_H_
#define _G_REWEIGHT_H_

#include <string>
#include <map>
#include <vector>

// GENIE/Reweight includes
#include "RwFramework/GSystSet.h"
#include "RwFramework/GReWeightI.h"

namespace genie {

class EventRecord;

namespace rew   {

 class GReWeight
 {
 public:
   GReWeight();
  ~GReWeight();

   void        AdoptWghtCalc (string name, GReWeightI* wcalc);   ///< add concrete weight calculator, transfers ownership
   GReWeightI* WghtCalc      (string name);                      ///< access a weight calculator by name
   GSystSet &  Systematics   (void);                             ///< set of enabled systematic params & values
   void        Reconfigure   (void);                             ///< reconfigure weight calculators with new params
   double      CalcWeight    (const genie::EventRecord & event); ///< calculate weight for input event
   void        Print         (void);                             ///< print
   
   const std::vector<std::string> & WghtCalcNames() const;

  private:

   void CleanUp (void);

   GSystSet                  fSystSet;   ///< set of enabled nuisance parameters
   std::map<std::string, GReWeightI *> fWghtCalc;  ///< concrete weight calculators
   std::vector<std::string> fWghtCalcNames; ///< list of weight calculators
 };

} // rew   namespace
} // genie namespace

#endif

