#include <TH1.h>
#include <TGraphErrors.h>

namespace TSysErrorUtils
{
   /* Double_t kEpsilon = std::numeric_limits<double>::epsilon(); */
   Double_t kEpsilon = 1e-15;
   Double_t kMaxDouble = 1e100;
   TH1D *Graph2Hist(TGraphErrors *gr, Bool_t useGraphEY = kFALSE, Double_t min = 0.0);
}
