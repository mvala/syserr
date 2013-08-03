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

   gr->SetName(TString::Format("%s_gr%03d",GetName(),fList->GetEntries()+1).Data());

   TSysError *se = new TSysError();
   se->SetGraph(gr);

   // TODO flag for useEY
   TH1D *h = TSysErrorUtils::Graph2Hist(gr, kFALSE);
   if (fHist) delete fHist;
   fHist = h;

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

void TSysError::SetTypeToList(TSysError::EType type) {
      TSysError *se;
      TIter next(fList);
      while ((se = (TSysError *) next())) se->SetType(type);
}



Double_t TSysError::Calculate() {

   
   switch (fType) {
      case kMean:
         // Calcularte Mean
         return CalculateMean();
         break;
      default:
         TSysError *se;
         TIter next(fList);
         while ((se = (TSysError *) next())) {
            se->Calculate();
         }
         break;
   }

   return 0.0;
}

Double_t TSysError::CalculateMean() {

   if (fHist) {
      return 0.0;
   } else {
      // TSysError *se;
      // TIter next(fList);
      // Double_t arr[fList->GetEntries()];
      // Int_t i = 0;
      // Int_t n = 0;
      // while ((se = (TSysError *) next())) {
      //    arr[i] = se->CalculateMean();
      //    sum += arr[i];
      //    i++;
      //    n++;
      // }

      return 0.0;
   }

   
   // if (fGraph) {
      
   //    Int_t i;
   //    Double_t x, y;
   //    // Double_t ex, ey;
   //    Double_t sumXY = 0.0;
   //    Double_t sumY = 0.0;
   //    Int_t n = fGraph->GetN();
   //    for (i=0; i<n; i++) {
   //       fGraph->GetPoint(i, x, y);
   //       // ex = fGraph->GetErrorX(i);
   //       // ey = fGraph->GetErrorY(i);
   //       Printf("x=%f y=%f x*y=%f", x, y, x*y);
   //       sumXY += x*y;
   //       sumY += y;
   //    }
   //    Double_t mean = sumXY/sumY;
   //    Printf("Mean is %f", mean);

   //    Double_t sumD = 0.0;
   //    for (i=0; i<n; i++) {
   //       fGraph->GetPoint(i, x, y);
   //       // ex = fGraph->GetErrorX(i);
   //       // ey = fGraph->GetErrorY(i);
   //       Printf("x=%f mean=%f d^2=%f", x, mean, TMath::Power(x-mean,2));
   //       sumD += TMath::Power(x-mean,2);
   //    }
   //    Double_t sigma = 0.0;
   //    Double_t alpha = 0.0;
   //    if (n > 1) {
   //       sigma =  TMath::Sqrt(sumD/(n-1));
   //       alpha = sigma/TMath::Sqrt(n);
   //    }
   //    Printf("Sigma is %f", sigma);
   //    Printf("Alpha is %f", alpha);
   //    return mean;
      
   // } else {
   //    Double_t sum = 0.0;
   //    TSysError *se;
   //    TIter next(fList);
   //    Double_t arr[fList->GetEntries()];
   //    Int_t i = 0;
   //    Int_t n = 0;
   //    while ((se = (TSysError *) next())) {
   //       arr[i] = se->CalculateMean();
   //       sum += arr[i];
   //       i++;
   //       n++;
   //    }

   //    Double_t mean = sum/n;
   //    Printf("Mean of means is %f", mean);
   //    i = 0;
   //    next.Reset();
   //    while ((se = (TSysError *) next())) {
   //       Printf("%f", TMath::Power(arr[i]-mean,2));
   //       sum += TMath::Power(arr[i++]-mean,2);
   //    }
      
   //    Double_t sigma = 0.0;
   //    Double_t alpha = 0.0;
   //    if (n > 1) {
   //       sigma =  TMath::Sqrt(sum/(n-1));
   //       alpha = sigma/TMath::Sqrt(n);
   //    }
   //    Printf("Sigma of means is %f", sigma);
   //    Printf("Alpha of means is %f", alpha);
   //    return mean;
   // }

   return 0.0;
}
