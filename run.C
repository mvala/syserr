#ifndef __CINT__
#include <TROOT.h>
#include "TSysError.h"
#endif

void run() {

   gROOT->ProcessLine(".L TSysError.cxx+g");
   if (!gROOT->GetClass("TSysError")) return;

   // const char *dir = "/eos/saske.sk/users/m/mvala/ALICE/Rsn_WORK/pp_2.76/SysErr";
   const char *dir = "/home/mvala/ALICE/RSN_WORK/SysErr";

   TSysError *finalPt = new TSysError("finalPt","Final Pt");

   TSysError *pt_tracking_LS = new TSysError("pt_tracking_LS","Pt (tracking) Like Sign");
   pt_tracking_LS->AddGraphDirectory(TString::Format("%s/QUALITY/LS",dir).Data(),"","%lg %lg %lg %lg");
   // pt_tracking_LS->SetUseWeight(kFALSE);

   TSysError *pt_tracking_MIX = new TSysError("pt_tracking_MIX","Pt (tracking) Mixing");
   pt_tracking_MIX->AddGraphDirectory(TString::Format("%s/QUALITY/MIX",dir).Data(),"","%lg %lg %lg %lg");

   finalPt->Add(pt_tracking_LS);
   finalPt->Add(pt_tracking_MIX);

   // calculate everyting
   finalPt->Calculate();

   TGraphErrors *gr = finalPt->GetGraph();
   if (!gr) return;
   Printf("Drawing graph ...");
   gr->Draw("AP");


}
