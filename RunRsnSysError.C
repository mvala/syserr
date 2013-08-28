#ifndef __CINT__
#include <TROOT.h>
#include <TSystem.h>
#include <TObjArray.h>
#include <TObjString.h>
#include "TSysError.h"
#endif
void RunRsnSysError()
{

   gROOT->ProcessLine(".L TSysErrorUtils.cxx+g");
   gROOT->ProcessLine(".L TSysError.cxx+g");
   if (!gROOT->GetClass("TSysError")) return;

   gROOT->LoadMacro("run_rsn_sys_err_bad.C");

   TString base, refMC, fileName, dirType;
   base = "$HOME/Documents/Work/Presentations/2013/RESONANCE_2013-08-09/analysis/2013-08-07/SYS_ERR/";
   base = "$HOME/Documents/Work/Presentations/2013/RESONANCE_2013-08-16/analysis/2013-08-16/SYS_ERR/";
   refMC = base + "00_REF/KTPCnsig30_STD2010_PRIMARY_00";
   //    base = "$HOME/Documents/Work/Presentations/2013/RESONANCE_2013-08-16/analysis/2013-08-16/SYS_ERR_NOPID/";
//    refMC = base + "00_REF/qualityonly_STD2010_PRIMARY_00";
   fileName = "RFE";


   base = "$HOME/Documents/Work/Presentations/2013/RESONANCE_2013-08-30/notes/2013-08-30/SysErr/";
   refMC = base + "00_REF/TPC30/0/MIX/KTPCnsig30_STD2010_PRIMARY_00";
   fileName = "pt_RFE";

   TString dirTypes;
   dirTypes += "01_QUALITY ";
//    dirTypes += "01_QUALITY_ALL ";
   dirTypes += "02_PID ";
//    dirTypes += "02_PID_ALL ";
   dirTypes += "03_NORM ";
   dirTypes += "04_FIT ";
//    dirTypes += "04_FIT_all ";
   dirTypes += "05_MIX_LS ";

   TSysError::EType type = TSysError::kMaxValueFromBin;
   type = TSysError::kStdDevValueFromBinPercent;
   type = TSysError::kStdErrValueFromBinPercent;

   Bool_t useOwnRef = kFALSE;
   useOwnRef = kTRUE;

   gSystem->Exec(TString::Format("rm -f %s/results/out.root",base.Data()).Data());
   TObjArray *arr = dirTypes.Tokenize(" ");
   TObjString *os;
   TIter next(arr);
   while((os = (TObjString *)next())) {
      dirType = os->GetString();
      run_rsn_sys_err_bad(dirType, fileName, base, refMC, type, useOwnRef);
   }


}
