#ifndef __CINT__
#include <TROOT.h>
#include <TFile.h>
#include <TSystem.h>
#include <TObjString.h>
#include "TSysError.h"
#include "TSysErrorUtils.h"
#endif

void ProcessDirecotry(TObject *se, const char *dirname, const char *filter="", Int_t maxMeasurments=kMaxInt,
                      Int_t maxMethods = kMaxInt,
                      TString refMC="",TString fileFilter="RFE", Bool_t useOwnRef=kFALSE)
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
   TString s,s2, strRef;
   TIter next(t);
   TSysError *seTmp;
   Int_t c=0;
   TList *l;
   while ((so = (TObjString *) next())) {
      s = so->GetString();
      seTmp = new TSysError(s.Data(), "");
      seTmp->AddGraphDirectory(TString::Format("%s/%s/", dir,s.Data()).Data(), fileFilter.Data(), "%lg %lg %lg %lg", maxMeasurments);
//       seTmp->SetType(TSysError::kMaxValueFromBin);
      strRef = TString::Format("%s/%s",refMC.Data(),fileFilter.Data()).Data();
      if (useOwnRef) {
         strRef = TString::Format("%s/%s",dir,s.Data()).Data();
//          Printf(strRef.Data());
         strRef = gSystem->GetFromPipe(TString::Format("readlink %s",strRef.Data()).Data());
         for (Int_t i=1;i<=5;i++) {
            strRef.ReplaceAll(TString::Format("/%d/",i).Data(),"/0/");
            strRef.ReplaceAll(TString::Format("_PRIMARY_0%d",i).Data(),"_PRIMARY_00");
         }
         strRef +="/";
         strRef +=fileFilter.Data();
      }
//       Printf(strRef.Data());
      seTmp->AddInput(new TNamed("RefMC",strRef.Data())) ;
      seTmp->SetType(TSysError::kMaxValueFromBinPercent);
      seTmp->SetTypeToList(TSysError::kAbsoluteDevFromRef);
      ((TSysError *)se)->Add(seTmp);
      TIter nextSE(seTmp->GetList());
      TH1D *h;
      Int_t c2=0;
      while ((seTmp = (TSysError *) nextSE())) {
         so2 = (TObjString *) t2->At(c2);
         s2 = so2->GetString();
         seTmp->AddInput(new TNamed("RefMC", strRef.Data()));
         c2++;
      }
      if (++c>=maxMethods) break;
   }
   gSystem->ChangeDirectory(pwd.Data());
}

void run_rsn_sys_err_bad(TString dirType, TString fileName, TString base,TString refMC, TSysError::EType type, Bool_t useOwnRef=kFALSE)
{

   Bool_t rc;

   TString dir;
   Printf("%s %s %s %s",dirType.Data(), fileName.Data(), base.Data(), refMC.Data());


   dir = base + dirType + "/";

   TSysError *qualityPt = new TSysError("bestMethodNumMCPt", "Best Method in num MC Pt");
   // qualityPt->SetType(TSysError::kMean);
//    qualityPt->SetType(TSysError::kMaxValueFromBin);
//    qualityPt->SetType(TSysError::kStdDevValueFromBinPercent);
//    qualityPt->SetType(TSysError::kStdErrValueFromBinPercent);
   qualityPt->SetType(type);




//    qualityPt->SetType(TSysError::kMaxValueFromBinPercent);
//    qualityPt->SetType(TSysError::kMaxStdDevValueFromBinPercent);

   // qualityPt->SetType(TSysError::kMinStdDev);
//    qualityPt->AddInput(new TNamed("RefMC", TString::Format("%s/%s",refMC.Data(),fileName.Data()).Data()));
   // qualityPt->SetPrintInfo(kTRUE);

   // processdirecotry(qualityPt, dir.Data(),"",kMaxInt,1, refMC);
   ProcessDirecotry(qualityPt, dir.Data(),"",kMaxInt,kMaxInt, refMC,fileName.Data(),useOwnRef);

   rc = qualityPt->Calculate();

//    TString fileOpt="APPEND";
//    if (gSystem->AccessPathName(TString::Format("%s/results/out.root",base.Data()))) {
//       fileOpt="NEW";
//    }
   TString outFileName = TString::Format("%s/results/out.root",base.Data());
   outFileName.ReplaceAll("//","/");
   TFile *f = TFile::Open(outFileName.Data(),"UPDATE");
   TH1D *h = qualityPt->GetHistogram();
   h->SetName(dirType.Data());
   h->Write();
   f->Close();
   Printf("Output files saved %s",outFileName.Data());

//    ofstream out;
// //    dirType = base + "results/" + dirType + ".txt";
// //    dirType = base + "results/out.root";
//    Printf(dirType.Data());
//    out.open(gSystem->ExpandPathName(dirType.Data()));
//    for (Int_t i=1; i<=h->GetNbinsX();i++) {
//       out << h->GetXaxis()->GetBinCenter(i) << " " << h->GetBinContent(i) << " ";
//       out << h->GetXaxis()->GetBinWidth(i) << " " << 0.0 << endl;
//    }
//    out.close();

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
