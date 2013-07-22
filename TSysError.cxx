// -*-c++-*-

#include <TError.h>
#include <TSystem.h>
#include <TGraphErrors.h>
#include <TList.h>
#include <TObjArray.h>
#include <TString.h>
#include <TObjString.h>
#include <TMath.h>
#include "TSysError.h"

ClassImp(TSysError);

TSysError::TSysError() : TNamed(),
   fList(0),
   fGraph(0),
   fMean(0.0),
   fSigma(0.0),
   fAlpha(0.0)
{

}

TSysError::TSysError(const char *name, const char *title) : TNamed(name, title),
   fList(0),
   fGraph(0),
   fMean(0.0),
   fSigma(0.0),
   fAlpha(0.0)
{

}

TSysError::~TSysError()
{

   // deleting list
   fList->Delete();

   delete fList;
   delete fGraph;

   // we don't delete parent
   // delete fParent

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

   // check if graph has same binning as first
   if (fList->GetEntries() > 0) {
      TSysError *se = (TSysError *) fList->At(0);
      TGraphErrors *grFirst = se->GetGraph();
      Double_t *x0 = grFirst->GetX();
      Double_t *x1 = gr->GetX();
      Double_t *ex0 = grFirst->GetEX();
      Double_t *ex1 = gr->GetEX();

      Double_t dx, dex;
      Double_t delta = 1e-4;
      for (Int_t i = 0; i < grFirst->GetN(); i++) {
         dx = TMath::Abs(x0[i] - x1[i]);
         dex = TMath::Abs(ex0[i] - ex1[i]);
         // Printf("dx=%f dex=%f", dx, dex);
         if ((dx > delta) || (dex > delta)) {
            ::Error("TSysError::AddGraph",
                    TString::Format("Graph from '%s' doesn't have same binning "
                                    "as first graph in list !!! Skipping ...",
                                    filename).Data());
            delete gr;
            return kFALSE;
         }
      }
   }

   TSysError *se = new TSysError();
   se->SetGraph(gr);

   // adding TSysError to the list
   fList->Add(se);

   return kTRUE;
}

Bool_t TSysError::AddGraphDirectory(const char *dirname, const char *filter, const char *tmpl)
{

   const char *pwd = gSystem->WorkingDirectory();

   if (!gSystem->ChangeDirectory(dirname)) {
      ::Error("TSysError::AddGraphDirectory",
              TString::Format("Could not enter direcotry '%s' !!!", dirname).Data());
      return kFALSE;
   }
   TString out = gSystem->GetFromPipe(TString::Format("ls %s", filter).Data());
   if (out.IsNull()) {
      ::Error("TSysError::AddGraphDirectory",
              TString::Format("No files found in '%s' !!!", dirname).Data());
      return kFALSE;

   }

   Printf("%s", out.Data());
   TObjArray *t = out.Tokenize("\n");
   TObjString *so;
   TString s;
   TIter next(t);
   while ((so = (TObjString *) next())) {
      s = so->GetString();
      AddGraph(s.Data(), tmpl);
   }

   gSystem->ChangeDirectory(pwd);

   return kTRUE;
}

void TSysError::Calculate()
{
   // we end calculate when there is nothing to calculate
   if ((!fList) || (!fList->GetEntries())) return;

   TIter next(fList);
   TSysError *se;
   TGraphErrors *gr;
   Double_t x, y, ex, ey;
   Double_t yTmp, eyTmp;
   Double_t yF, eyF;
   Int_t n = 0;

   if (!fGraph) {
      while ((se = (TSysError *) next())) {
         se->Calculate();
      }
   }

   Printf("Doing Calculate %s ...", GetName());

   // recreating current graph
   if (fGraph) { delete fGraph; fGraph = 0;}

   se = (TSysError *) fList->At(0);
   if (se->GetGraph()) {
      fGraph = (TGraphErrors *) se->GetGraph()->Clone();
      n = fGraph->GetN();
   }

   // check if we have only 1 graph in list (then we just clone it to fGraph)
   if (fList->GetEntries() == 1) {
      ::Warning("TSysError::Calculate", "Only 1 graph in list !!! Just cloning it to current one ...");
      return;
   }
   
   // reseting y and ey of fGraph (current final graph)
   for (Int_t i = 0; i < n; i++) {
      fGraph->GetPoint(i, x, y);
      fGraph->SetPoint(i, x, 0.0);
      fGraph->SetPointError(i, fGraph->GetErrorX(i), 0.0);
   }

   // loop over points (bins)
   for (Int_t i = 0; i < n; i++) {
      // reset tmp values and iter
      yTmp = 0.0;
      eyTmp = 0.0;
      next.Reset();
      while ((se = (TSysError *) next())) {
         gr = se->GetGraph();
         // if (!gr) {
         //    se->Calculate();
         //    gr = se->GetGraph();
         // }
         gr->GetPoint(i, x, y);
         ex = gr->GetErrorX(i);
         ey = gr->GetErrorY(i);
         Printf("[%d] x=%f y=%f ex=%f ey=%f", i, x, y, ex, ey);
         yTmp += y;
         eyTmp += TMath::Power(ey, 2);
      }

      yF = yTmp / n;
      eyF = TMath::Sqrt(eyTmp / (n - 1));
      // Printf("mean(yF)=%f error(eyF)=%f", yF, eyF);
      fGraph->GetPoint(i, x, y);
      fGraph->SetPoint(i, x, yF);
      fGraph->SetPointError(i, fGraph->GetErrorX(i), eyF);
   }

   if (fGraph) {
      fGraph->Print();

      // calculate mean
      yTmp = 0.0;
      eyTmp = 0.0;
      for (Int_t i = 0; i < n; i++) {
         gr->GetPoint(i, x, y);
         ex = gr->GetErrorX(i);
         ey = gr->GetErrorY(i);
         yTmp += y;
         eyTmp += TMath::Power(ey, 2);
      }
      n = fGraph->GetN();
      yF = yTmp / n;
      eyF = TMath::Sqrt(eyTmp / (n - 1));
      Printf("TODO mean(yF)=%f error(eyF)=%f", yF, eyF);

      // TODO - Kukni se do knizky
      fMean = yF;
      fSigma = TMath::Sqrt(eyTmp);
      fAlpha = fSigma / TMath::Sqrt(n - 1);

   }

}
