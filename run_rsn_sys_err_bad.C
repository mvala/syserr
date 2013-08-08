#ifndef __CINT__
#include <TROOT.h>
#include "TSysError.h"
#include "TSysErrorUtils.h"
#endif

void ProcessDirecotry(TObject *se, const char *dirname, const char *filter="", Int_t maxMeasurments=kMaxInt,
                      Int_t maxMethods = kMaxInt,
                      TString refMC="")
{

   TString pwd = gSystem->WorkingDirectory();
   const char *dir = gSystem->ExpandPathName(dirname);
   const char *refmc = gSystem->ExpandPathName(refMC);

   if (!gSystem->ChangeDirectory(dir)) {
      ::Error("TSysError::AddGraphDirectory",
              TString::Format("Could not enter direcotry '%s' !!!", dir).Data());
      return kFALSE;
   }
   
   TString out = gSystem->GetFromPipe(TString::Format("ls -1 %s", filter).Data());
   if (out.IsNull()) {
      ::Error("TSysError::AddGraphDirectory",
              TString::Format("No files found in '%s' !!!", dir).Data());
      return kFALSE;

   }

   
   TString outRefMC;
   if (!refMC.IsNull()) {
      outRefMC = gSystem->GetFromPipe(TString::Format("ls -1 %s%s", refMC.Data(), filter).Data());
      if (outRefMC.IsNull()) {
         ::Error("TSysError::AddGraphDirectory",
                 TString::Format("RefMC: No files found in '%s' !!!", dir).Data());
         return kFALSE;
         
      }
   }
   
   TObjArray *t = out.Tokenize("\n");
   TObjArray *t2 = outRefMC.Tokenize("\n");
   TObjString *so, *so2;
   TString s,s2;
   TIter next(t);
   TSysError *seTmp;
   Int_t c=0;
   TList *l;
   while ((so = (TObjString *) next())) {
      s = so->GetString();
      seTmp = new TSysError(s.Data(), "");
      seTmp->AddGraphDirectory(TString::Format("%s/%s/", dir,s.Data()).Data(), "RFE", "%lg %lg %lg %lg", maxMeasurments);
      seTmp->SetType(TSysError::kMaxValueFromBin);
      seTmp->SetTypeToList(TSysError::kAbsoluteDevFromRef);
      ((TSysError*)se)->Add(seTmp);
      TIter nextSE(seTmp->GetList());
      TH1D *h;
      Int_t c2=0;
      while ((seTmp = (TSysError *) nextSE())) {
         so2 = (TObjString *) t2->At(c2); 
         s2 = so2->GetString();
         seTmp->AddInput(new TNamed("RefMC", TString::Format("%s/RFE",refMC.Data()).Data()));
         c2++;
      }
      if (++c>=maxMethods) break;
   }
   gSystem->ChangeDirectory(pwd.Data());
}

void run_rsn_sys_err_bad()
{

   gROOT->ProcessLine(".L TSysErrorUtils.cxx+g");
   gROOT->ProcessLine(".L TSysError.cxx+g");
   if (!gROOT->GetClass("TSysError")) return;

   Bool_t rc;

   TString dir;

   TString refMC="$HOME/Documents/Work/Presentations/2013/RESONANCE_2013-08-09/analysis/2013-08-07/SYS_ERR/00_REF/KTPCnsig30_STD2010_PRIMARY_00";

   // dir = "/eos/saske.sk/scratch/ALICE/RSN_OUT/001";
   dir = "$HOME/Documents/Work/Presentations/2013/RESONANCE_2013-08-09/analysis/2013-08-07/SYS_ERR/01_QUALITY/";
   // dir = "$HOME/Documents/Work/Presentations/2013/RESONANCE_2013-08-09/analysis/2013-08-07/SYS_ERR/02_PID/";
   // dir = "$HOME/Documents/Work/Presentations/2013/RESONANCE_2013-08-09/analysis/2013-08-07/SYS_ERR/03_NORM/";
   // dir = "$HOME/Documents/Work/Presentations/2013/RESONANCE_2013-08-09/analysis/2013-08-07/SYS_ERR/04_FIT/";

   TSysError *qualityPt = new TSysError("bestMethodNumMCPt", "Best Method in num MC Pt");
   // qualityPt->SetType(TSysError::kMean);
   // qualityPt->SetType(TSysError::kMaxValueFromBin);
   qualityPt->SetType(TSysError::kMaxValueFromBinPercent);
   // qualityPt->SetType(TSysError::kMinStdDev);
   qualityPt->AddInput(new TNamed("RefMC", TString::Format("%s/RFE",refMC.Data()).Data()));
   // qualityPt->SetPrintInfo(kTRUE);

   // processdirecotry(qualityPt, dir.Data(),"",kMaxInt,1, refMC);
   ProcessDirecotry(qualityPt, dir.Data(),"",kMaxInt,kMaxInt, refMC);

   rc = qualityPt->Calculate();


   

   // TH1D *h = qualityPt->GetHistogram();
   // h->Draw();

   Printf("Process status : %s", (rc == 0) ? "FAILED !!!" : "OK");

   TFile *f = TFile::Open("out.root","RECREATE");
   qualityPt->Write();
   f->Close();

   // TGraphErrors *gr = finalPt->GetGraph();
   // if (!gr) return;
   // Printf("Drawing graph ...");
   // gr->Draw("APE");

}
