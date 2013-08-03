#ifndef __CINT__
#include <TROOT.h>
#include "TSysError.h"
#include "TSysErrorUtils.h"
#endif

void testHisto()
{

   TSysError *test = new TSysError("test", "Test Title");
   test->AddGraph("/home/mvala/ALICE/RSN_WORK/SysErr/TEST/MIX/RsnHistMini_Phi_PhiNsigma_KTPCnsig30_STD2010_PRIMARY",
                  "%lg %lg %lg %lg");
   TSysError *test_sub = (TSysError *)test->GetList()->At(0);
   test_sub->GetGraph()->Print();
   h = TSysErrorUtils::Graph2Hist(test_sub->GetGraph(), kFALSE);

   if (!h) return;
   h->Print("all");
   h->Draw();
   // Print statistics
   Double_t stats[4];
   h->GetStats(stats);
   Printf("sumw=%f sumw2=%f sumwx=%f sumwx2=%f", stats[0], stats[1], stats[2], stats[3]);
   Printf("Mean=%f MeanError=%f", h->GetMean(), h->GetMeanError());
   Printf("StdDev=%f StdDevError=%f", h->GetStdDev(), h->GetStdDevError());
   Printf("RMS=%f RMSError=%f", h->GetRMS(), h->GetRMSError());

}

void run()
{

   gROOT->ProcessLine(".L TSysErrorUtils.cxx+g");
   gROOT->ProcessLine(".L TSysError.cxx+g");
   if (!gROOT->GetClass("TSysError")) return;

   // TH1D *h = TSysErrorUtils::Graph2Hist(new TGraphErrors());
   // if (h) h->Print();
   // return;

   // testHisto();
   // return;


   const char *dir = "/eos/saske.sk/users/m/mvala/ALICE/Rsn_WORK/pp_2.76/SysErr/QUALITY";
   // const char *dir = "/cache/users/m/mvala/ALICE/Rsn_WORK/pp_2.76/SysErr/QUALITY";
   // const char *dir = "/home/mvala/ALICE/RSN_WORK/SysErr/QUALITY";
   // const char *dir = "/home/mvala/ALICE/RSN_WORK/SysErr/TEST";


   // TSysError *finalPt = new TSysError("finalPt","Final Pt");
   // finalPt->SetType(TSysError::kMean);

   TSysError *pt_tracking_LS = new TSysError("pt_tracking_LS", "Pt (tracking) Like Sign");
   pt_tracking_LS->SetType(TSysError::kMean);
   pt_tracking_LS->AddGraphDirectory(TString::Format("%s/LS", dir).Data(), "", "%lg %lg %lg %lg");
   pt_tracking_LS->SetTypeToList(TSysError::kMean);
   pt_tracking_LS->Calculate();

   // TSysError *pt_tracking_MIX = new TSysError("pt_tracking_MIX","Pt (tracking) Mixing");
   // pt_tracking_MIX->AddGraphDirectory(TString::Format("%s/MIX",dir).Data(),"","%lg %lg %lg %lg");

   // finalPt->Add(pt_tracking_LS);
   // finalPt->Add(pt_tracking_MIX);

   // calculate everyting
   // finalPt->Calculate();

   // TGraphErrors *gr = finalPt->GetGraph();
   // if (!gr) return;
   // Printf("Drawing graph ...");
   // gr->Draw("APE");

}
