#ifndef __CINT__
#include <TROOT.h>
#include "TSysError.h"
#endif

void run() {

   gROOT->ProcessLine(".L TSysError.cxx+g");
   if (!gROOT->GetClass("TSysError")) return;

   TSysError *finalPt = new TSysError("finalPt","Final Pt");

   TSysError *pt_tracking = new TSysError("pt_tracking","Pt (tracking)");
   pt_tracking->AddGraphDirectory("/home/mvala/Temp/ALICE","pt*.txt","%lg %lg %lg %lg");

   TSysError *pt_fit = new TSysError("pt_fit","Pt (fiting)");
   pt_fit->AddGraphDirectory("/home/mvala/Temp/ALICE","pt*.txt","%lg %lg %lg %lg");

   TSysError *pt_norm = new TSysError("pt_norm","Pt (normalize)");
   pt_norm->AddGraphDirectory("/home/mvala/Temp/ALICE","pt*.txt","%lg %lg %lg %lg");

   finalPt->Add(pt_tracking);
   finalPt->Add(pt_fit);
   finalPt->Add(pt_norm);

   // Calculate everyting
   finalPt->Calculate();

}
