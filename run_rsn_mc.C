#ifndef __CINT__
#include <TROOT.h>
#include "TSysError.h"
#include "TSysErrorUtils.h"
#endif

void ProcessDirecotry(TObject *se, const char *dirname, const char *filter="", Int_t maxMeasurments=kMaxInt,
                      Int_t maxMethods = kMaxInt, TString refMC="/home/mvala/ALICE/RSN_OUT/MC_NUM/Pythia/STEP_01_TRUE/runs/")
{

   TString pwd = gSystem->WorkingDirectory();
   const char *dir = gSystem->ExpandPathName(dirname);

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

   
   TString outRefMC = gSystem->GetFromPipe(TString::Format("ls -1 %s%s", refMC.Data(), filter).Data());
   if (outRefMC.IsNull()) {
      ::Error("TSysError::AddGraphDirectory",
              TString::Format("RefMC: No files found in '%s' !!!", dir).Data());
      return kFALSE;

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
      seTmp->AddGraphDirectory(TString::Format("%s/%s/runs", dir,s.Data()).Data(), "", "%lg %lg %lg %lg", maxMeasurments);
      seTmp->SetType(TSysError::kMean);
      seTmp->SetTypeToList(TSysError::kRelativeErrorMC);
      ((TSysError*)se)->Add(seTmp);
      TIter nextSE(seTmp->GetList());
      TH1D *h;
      Int_t c2=0;
      while ((seTmp = (TSysError *) nextSE())) {
         so2 = (TObjString *) t2->At(c2); 
         s2 = so2->GetString();
         seTmp->AddInput(new TNamed("RefMC", TString::Format("%s%s",refMC.Data(),s2.Data()).Data()));
         c2++;
      }
      if (++c>=maxMethods) break;
   }
   gSystem->ChangeDirectory(pwd.Data());
}

void run_rsn_mc()
{

   gROOT->ProcessLine(".L TSysErrorUtils.cxx+g");
   gROOT->ProcessLine(".L TSysError.cxx+g");
   if (!gROOT->GetClass("TSysError")) return;

   Bool_t rc;

   TString dir;

   // dir = "/eos/saske.sk/scratch/ALICE/RSN_OUT/001";
   dir = "$HOME/ALICE/RSN_OUT/MC_NUM/Pythia/STEP_01";

   TSysError *bestMethodNumMCPt = new TSysError("bestMethodNumMCPt", "Best Method in num MC Pt");
   // bestMethodNumMCPt->SetType(TSysError::kMean);
   bestMethodNumMCPt->SetType(TSysError::kMinStdDev);
   // bestMethodNumMCPt->SetType(TSysError::kMinStdDev);
   bestMethodNumMCPt->SetPrintInfo(kTRUE);

   // ProcessDirecotry(bestMethodNumMCPt, dir.Data(),"",kMaxInt,2);
   ProcessDirecotry(bestMethodNumMCPt, dir.Data(),"",kMaxInt,kMaxInt);


   rc = bestMethodNumMCPt->Calculate();

   Printf("Process status : %s", (rc == 0) ? "FAILED !!!" : "OK");

   // TGraphErrors *gr = finalPt->GetGraph();
   // if (!gr) return;
   // Printf("Drawing graph ...");
   // gr->Draw("APE");

}
