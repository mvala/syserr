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
   fUseWeight(kTRUE),
   fMean(0.0),
   fSigma(0.0),
   fAlpha(0.0),
   fDelta(1e-6)
{

}

TSysError::TSysError(const char *name, const char *title) : TNamed(name, title),
   fList(0),
   fGraph(0),
   fUseWeight(kTRUE),
   fMean(0.0),
   fSigma(0.0),
   fAlpha(0.0),
   fDelta(1e-6)
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
      for (Int_t i = 0; i < grFirst->GetN(); i++) {
         dx = TMath::Abs(x0[i] - x1[i]);
         dex = TMath::Abs(ex0[i] - ex1[i]);
         // Printf("dx=%f dex=%f", dx, dex);
         if ((dx > fDelta) || (dex > fDelta)) {
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

  TString pwd = gSystem->WorkingDirectory();

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

void TSysError::Calculate()
{
   // we end calculate when there is nothing to calculate
   if ((!fList) || (!fList->GetEntries())) return;

   TIter next(fList);
   TSysError *se;
   TGraphErrors *gr;
   Double_t x, y, ex, ey, w;
   Double_t ySum, eySum, wSum;
   Double_t yMean, eyMean;
   Int_t nBins, nMeasurements;

   if (!fGraph) {
      while ((se = (TSysError *) next())) {
         se->Calculate();
      }
   }

   ::Info("TSysError::Calculate", "%s ...", GetName());

   // recreating current graph
   if (fGraph) { delete fGraph; fGraph = 0;}

   se = (TSysError *) fList->At(0);
   if (se->GetGraph()) {
      fGraph = (TGraphErrors *) se->GetGraph()->Clone();
      nBins = fGraph->GetN();
   }

   // check if we have only 1 graph in list (then we just clone it to fGraph)
   if (fList->GetEntries() == 1) {
      ::Warning("TSysError::Calculate", "Only 1 graph in list !!! Just cloning it to current one ...");
      fGraph->Print();
      return;
   }

   // reseting y and ey of fGraph (current final graph)
   for (Int_t i = 0; i < nBins; i++) {
      fGraph->GetPoint(i, x, y);
      fGraph->SetPoint(i, x, 0.0);
      
      if ((y>fDelta) && (fGraph->GetErrorY(i) < fDelta)) {
         fUseWeight = kFALSE;
      }
      fGraph->SetPointError(i, fGraph->GetErrorX(i), 0.0);
   }

   if (!fUseWeight) {
      ::Warning("TSysError::Calculate", "Calculating without weights !!!");
      w = 1.0;
   }

   // loop over points (bins)
   for (Int_t i = 0; i < nBins; i++) {
      // reset tmp values and iter
      ySum = 0.0;
      eySum = 0.0;
      wSum = 0.0;
      nMeasurements = 0;

      // loop over graphs to calculate mean value
      next.Reset();
      while ((se = (TSysError *) next())) {
         if (!fUseWeight) {
            // let's get points
            gr = se->GetGraph();
            gr->GetPoint(i, x, y);
            ex = gr->GetErrorX(i);
            ey = gr->GetErrorY(i);
            ySum += y;
         }
         nMeasurements++;
      }

      // calculate mean, in case we are not doing weights
      if (!fUseWeight) yMean = ySum / nMeasurements;

      next.Reset();
      // loop over graphs to calculate errors
      while ((se = (TSysError *) next())) {
         // let's get points
         gr = se->GetGraph();
         gr->GetPoint(i, x, y);
         ex = gr->GetErrorX(i);
         ey = gr->GetErrorY(i);


         Printf("[%d] x=%f y=%f ex=%f ey=%f", i, x, y, ex, ey);

         // calculating sums and weight
         if (fUseWeight) {
            // if y and ey are zero we break
            if ((y<fDelta) && (ey < fDelta)) {
               break;
            }
            w = 1 / TMath::Power(ey, 2);
            ySum += y * w;
            wSum += w;
            eySum += 1 / TMath::Power(ey, 2);
         } else {
            // yMean is already calculated
            // here we calculate sigma instead of alpha
            eySum += TMath::Power(y - yMean, 2);
         }
      }

      // doing final calculation
      if (fUseWeight) {
         // if ySum or eySum is zero (we continue)
         if ((ySum<fDelta) && (eySum < fDelta)) {
            fGraph->SetPoint(i, x, 0.0);
            fGraph->SetPointError(i, ex, 0.0);
            continue;
         }
         yMean = ySum / wSum;
         eyMean = 1 / TMath::Sqrt(eySum);
      } else {
         // yMean is already calculated

         // here sigma=sqrt((1/N-1)*sum(d^2))
         eyMean = TMath::Sqrt(eySum / (nMeasurements - 1));
         // alpha = sigma/sqrt(N)
         eyMean /= TMath::Sqrt(nMeasurements);
      }
      // Printf("mean(yMean)=%f error(eyMean)=%f", yMean, eyMean);
      fGraph->GetPoint(i, x, y);
      fGraph->SetPoint(i, x, yMean);
      fGraph->SetPointError(i, fGraph->GetErrorX(i), eyMean);
   }

   if (fGraph) {
      fGraph->Print();

      // setting number of bins
      nBins = fGraph->GetN();

      // calculate mean
      ySum = 0.0;
      eySum = 0.0;
      wSum = 0.0;
      for (Int_t i = 0; i < nBins; i++) {
         gr->GetPoint(i, x, y);
         ex = gr->GetErrorX(i);
         ey = gr->GetErrorY(i);
         if ((y<fDelta) && (ey < fDelta)) continue;
         w = 1 / TMath::Power(ey, 2);
         ySum += y * w;
         wSum += w;
         eySum += 1 / TMath::Power(ey, 2);
      }
      yMean = ySum / wSum;
      eyMean = 1 / TMath::Sqrt(eySum);
      // TODO - Kukni se do knizky
      fMean = yMean;
      // we will not calculate sigma for now
      fSigma = -1.0;
      fAlpha = eyMean;
      Printf("TODO mean(fMean)=%e error(fAlpha)=%e", fMean, fAlpha);

   }

}
