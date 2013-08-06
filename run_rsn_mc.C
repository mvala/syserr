#ifndef __CINT__
#include <TROOT.h>
#include "TSysError.h"
#include "TSysErrorUtils.h"
#endif

void ProcessDirecotry(TObject *se, const char *dirname, const char *filter="")
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

   TObjArray *t = out.Tokenize("\n");
   TObjString *so;
   TString s;
   TIter next(t);
   TSysError *seTmp;
   while ((so = (TObjString *) next())) {
      s = so->GetString();
      seTmp = new TSysError(s.Data(), "");
      seTmp->AddGraphDirectory(TString::Format("%s/%s", dir,s.Data()).Data(), "", "%lg %lg %lg %lg");
      seTmp->SetType(TSysError::kMean);
      seTmp->SetTypeToList(TSysError::kMean);
      ((TSysError*)se)->Add(seTmp);
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

   dir = "/eos/saske.sk/scratch/ALICE/RSN_OUT/001";
   dir = "$HOME/ALICE/RSN_OUT/001";


   TSysError *bestMethodNumMCPt = new TSysError("bestMethodNumMCPt", "Best Method in num MC Pt");
   bestMethodNumMCPt->SetType(TSysError::kMinStdDev);

   ProcessDirecotry(bestMethodNumMCPt, dir.Data());

   rc = bestMethodNumMCPt->Calculate();

   Printf("Process status : %s", (rc == 0) ? "FAILED !!!" : "OK");

   // TGraphErrors *gr = finalPt->GetGraph();
   // if (!gr) return;
   // Printf("Drawing graph ...");
   // gr->Draw("APE");

}
