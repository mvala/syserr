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
   fType(kNone)
{

}

TSysError::TSysError(const char *name, const char *title) : TNamed(name, title),
   fList(0),
   fGraph(0),
   fHist(0),
   fType(kNone)
{

}

TSysError::~TSysError()
{

   // clearing list
   fList->Clear();

   delete fList;
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

   Printf("Adding fGraph='%s' and fHist=%s", gr->GetName(), h->GetName());

   // adding TSysError to the list
   fList->Add(se);

   return kTRUE;
}

Bool_t TSysError::AddGraphDirectory(const char *dirname, const char *filter, const char *tmpl)
{

   TString pwd = gSystem->WorkingDirectory();

   if (!gSystem->ChangeDirectory(dirname)) {
      ::Error("TSysError::AddGraphDirectory",
              TString::Format("Could not enter direcotry '%s' !!!", dirname).Data());
      return kFALSE;
   }
   TString out = gSystem->GetFromPipe(TString::Format("ls -1 %s", filter).Data());
   if (out.IsNull()) {
      ::Error("TSysError::AddGraphDirectory",
              TString::Format("No files found in '%s' !!!", dirname).Data());
      return kFALSE;

   }

   TObjArray *t = out.Tokenize("\n");
   TObjString *so;
   TString s;
   TIter next(t);
   while ((so = (TObjString *) next())) {
      s = so->GetString();
      AddGraph(s.Data(), tmpl);
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

void TSysError::PrintHistogramInfo(TSysError *se, TH1D *h) {

   TSysError *seTmp;
   TH1D *hTmp;
   if (se && h) {
      seTmp = se;
      hTmp = h;
   } else if (!se && h) {
      seTmp = new TSysError();
      hTmp = h;
   }
   else {
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
         h = se->GetHistogram();
         if (!h) return kFALSE;

         if (!fHist) {
            fHist = (TH1D *) h->Clone();
            fHist->Reset();
            //  we don't want errors per bin
            fHist->Sumw2(kFALSE);
         }
         fHist->Fill(h->GetMean());
         PrintHistogramInfo(se,h);
      }
   }
   PrintHistogramInfo(this);
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

   next.Reset();
   while ((se = (TSysError *) next())) {
      Printf("%s", se->GetName());
      h = se->GetHistogram();
      PrintHistogramInfo(se,h);
   }

   se = (TSysError *) fList->At(minIdx);
   Printf("'%s'[%d] => MinStdDev is %s", GetName(), GetType(), se->GetName());
   return kTRUE;
}
