// -*-c++-*-

#include <TError.h>
#include <TSystem.h>
#include <TGraphErrors.h>
#include <TList.h>
#include <TObjArray.h>
#include <TString.h>
#include <TObjString.h>
#include <TMath.h>
#include <TH1.h>
#include "TSysErrorUtils.h"

#include "TSysError.h"

ClassImp(TSysError)

TSysError::TSysError() : TNamed(),
   fList(0),
   fGraph(0),
   fHist(0),
   fInputList(0),
   fOutputList(0),
   fType(kNone),
   fPrintInfo(kFALSE)
{

}

TSysError::TSysError(const char *name, const char *title) : TNamed(name, title),
   fList(0),
   fGraph(0),
   fHist(0),
   fInputList(0),
   fOutputList(0),
   fType(kNone),
   fPrintInfo(kFALSE)
{

}

TSysError::~TSysError()
{

   // clearing list
   fList->Clear();
   fInputList->Clear();
   fOutputList->Clear();

   delete fList;
   delete fInputList;
   delete fOutputList;
   delete fGraph;
   delete fHist;

}

void TSysError::SetGraph(TGraphErrors *gr, Bool_t doClone)
{
   if (!gr) return;

   if (fGraph) delete fGraph;

   // do clone when users wants it
   if (doClone)
      fGraph = (TGraphErrors *) gr->Clone();
   else
      fGraph = gr;
}

void TSysError::SetHistogram(TH1D *h)
{
   if (!h) return;

   if (fHist) delete fHist;
   fHist = h;
}


void TSysError::Add(TSysError *sysError)
{
   if (!sysError) return;

   // create list if it was not created yet
   if (!fList) fList = new TList();

   // adds TSysError (someting like group)
   fList->Add(sysError);
}



Bool_t TSysError::AddGraph(const char *filename, const char *tmpl)
{
   Printf("Adding graph from file '%s' using template \"%s\"", filename, tmpl);

   // creating graph from file
   TGraphErrors *gr = new TGraphErrors(filename, tmpl);
   if (!gr) {
      ::Error("TSysError::AddGraph",
              TString::Format("Error creating graph from file '%s' !!!", filename).Data());
      return kFALSE;
   }

   // create list if it was not created yet
   if (!fList) fList = new TList();

   gr->SetName(TString::Format("%s_gr%03d", GetName(), fList->GetEntries() + 1).Data());

   TSysError *se = new TSysError(TString::Format("%s_%03d", GetName(), fList->GetEntries() + 1).Data(), "");
   se->SetGraph(gr);

   // TODO flag for useEY
   TH1D *h = TSysErrorUtils::Graph2Hist(gr, kFALSE);

   if (!h) return kFALSE;

   se->SetHistogram(h);

   // adding TSysError to the list
   fList->Add(se);

   return kTRUE;
}

Bool_t TSysError::AddGraphDirectory(const char *dirname, const char *filter, const char *tmpl, Int_t maxFiles)
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

   ::Info("TSysError::AddGraphDirectory", TString::Format("Adding dir %s", dir).Data());

   TObjArray *t = out.Tokenize("\n");
   TObjString *so;
   TString s;
   TIter next(t);
   Int_t c=0;
   while ((so = (TObjString *) next())) {
      s = so->GetString();
      AddGraph(s.Data(), tmpl);
      if (++c>=maxFiles) break;
   }

   gSystem->ChangeDirectory(pwd.Data());

   return kTRUE;
}

void TSysError::SetTypeToList(TSysError::EType type)
{
   TSysError *se;
   TIter next(fList);
   while ((se = (TSysError *) next())) se->SetType(type);
}

void TSysError::AddInput(TObject *o)
{
   if (!o) return;
   if (!fInputList) fInputList = new TList();
   fInputList->Add(o);
}

void TSysError:: AddOutput(TObject *o)
{
   if (!o) return;
   if (!fOutputList) fOutputList = new TList();
   fOutputList->Add(o);
}


void TSysError::PrintHistogramInfo(TSysError *se, TH1D *h)
{

   if (!fPrintInfo) return;

   TSysError *seTmp;
   TH1D *hTmp;
   if (se && h) {
      seTmp = se;
      hTmp = h;
   } else if (!se && h) {
      seTmp = new TSysError();
      hTmp = h;
   } else {
      seTmp = se;
      hTmp = se->GetHistogram();
   }

   if (!hTmp) return;
   Printf("========== Info for '%s' ================", GetName());
   hTmp->Print();
   Printf("'%s'[%d] => Mean=%f MeanError=%f", seTmp->GetName(), seTmp->GetType(), hTmp->GetMean(), hTmp->GetMeanError());
   Printf("'%s'[%d] => StdDev=%f StdDevError=%f", seTmp->GetName(), seTmp->GetType(), hTmp->GetStdDev(), hTmp->GetStdDevError());
   Printf("'%s'[%d] => RMS=%f RMSError=%f", seTmp->GetName(), seTmp->GetType(), hTmp->GetRMS(), hTmp->GetRMSError());

   if (!se && h) delete seTmp;
}


Bool_t TSysError::Calculate()
{

   Printf("Doing Calculate '%s' type=%d fHist=%p", GetName(), fType, fHist);
   switch (fType) {
      case kMean:
         return CalculateMean();
         break;
      case kMinStdDev:
         return CalculateMinStdDev();
         break;
      case kRelativeErrorMC:
         return CalculateRelaticeErrorMC();
         break;
      default:
         TSysError *se;
         TIter next(fList);
         while ((se = (TSysError *) next())) {
            if (!se->Calculate()) return kFALSE;
         }
         break;
   }

   return kTRUE;
}

Bool_t TSysError::CalculateMean()
{

   Printf("Doing CalculateMean '%s' type=%d fHist=%p", GetName(), fType, fHist);

   if (!fHist) {
      TSysError *se;
      TIter next(fList);
      TH1D *h;
      while ((se = (TSysError *) next())) {
         se->Calculate();
         h = se->GetHistogram();
         if (!h) return kFALSE;

         if (!fHist) {
            fHist = (TH1D *) h->Clone();
            fHist->Reset();
            //  we don't want errors per bin
            fHist->Sumw2(kFALSE);
         }
         fHist->Fill(h->GetMean());
         PrintHistogramInfo(se, h);
      }
      PrintHistogramInfo(this);

   } else {
      Printf("We have hist !!!");
      PrintHistogramInfo(this);
   }
   return kTRUE;
}

Bool_t TSysError::CalculateMinStdDev()
{

   if ((!fList) || (fList->GetEntries() < 1)) return kFALSE;

   TSysError *se;
   TIter next(fList);
   TH1D *h;
   Int_t idx = 0;
   Int_t minIdx;
   Double_t minStdDev = TSysErrorUtils::kMaxDouble;
   while ((se = (TSysError *) next())) {
      h = se->GetHistogram();
      if (!h) {
         se->Calculate();
         h = se->GetHistogram();
         if (!h) return kFALSE;
      }
      if (minStdDev > h->GetStdDev()) {
         minStdDev = h->GetStdDev();
         minIdx = idx;
      }
      idx++;
   }

   Printf("========== CalculateMinStdDev ================");
   next.Reset();
   while ((se = (TSysError *) next())) {
      h = se->GetHistogram();
      PrintHistogramInfo(se);

      // Printf("%s => StdDev=%f StdDevError=%f", se->GetName(), h->GetStdDev(), h->GetStdDevError());
   }

   se = (TSysError *) fList->At(minIdx);
   Printf("*****************************************************************");
   Printf("'%s'[%d] => MinStdDev is %s", GetName(), GetType(), se->GetName());
   PrintHistogramInfo(se);
   Printf("*****************************************************************");
   return kTRUE;
}

Bool_t TSysError::CalculateRelaticeErrorMC() {
   
   Printf("Doing CalculateRelaticeErrorMC '%s' type=%d fHist=%p fList=%p", GetName(), fType, fHist, fList);

   // let's move fGraph and fHist to fInputList and remove it from fList
   TSysError *se = new TSysError(TString::Format("%s_DP",GetName()).Data(),"");
   se->SetGraph(fGraph);
   se->SetHistogram(fHist);
   if (!fList) fList = new TList();
   fList->Add(se);
   fGraph = 0;
   fHist = 0;

   TNamed *n = (TNamed*) GetInputList()->FindObject("RefMC");
   AddGraph(n->GetTitle(),"%lg %lg %lg %lg");

   se = (TSysError*) fList->At(0);
   TGraphErrors *grDP = se->GetGraph();

   se = (TSysError*) fList->At(1);
   TGraphErrors *grMC = se->GetGraph();

   // let's sort graphs
   grDP->Sort();
   grMC->Sort();

   // grDP->Print("all");
   // grMC->Print("all");

   if (grDP->GetN() != grMC->GetN()) return kFALSE;

   fGraph = new TGraphErrors();
   fGraph->SetName(TString::Format("%s_DP_MC",GetName()).Data());
   // Double_t *xDP = grDP->GetX();
   Double_t *yDP = grDP->GetY();
   // Double_t *exDP = grDP->GetEX();
   // Double_t *eyDP = grDP->GetEY();

   // Double_t *xMC = grMC->GetX();
   Double_t *yMC = grMC->GetY();
   // Double_t *exMC = grMC->GetEX();
   // Double_t *eyMC = grMC->GetEY();

   Double_t eRelative;
   Double_t sum=0.0;
   for (Int_t i=0; i<grDP->GetN(); i++) {
      if (yMC[i]>0.0) {
         eRelative = (yMC[i]-yDP[i])/yMC[i];
         // eRelative = (yDP[i]-yMC[i])/yMC[i];
         sum += eRelative;
      }
   }
   fHist = new TH1D(TString::Format("%s_DP_MC_hist",GetName()).Data(),"",1,0,2*sum);
   fHist->Fill(sum);
   // fPrintInfo = kTRUE;
   PrintHistogramInfo(this);

   return kTRUE;
}
