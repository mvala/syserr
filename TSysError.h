// -*-c++-*-

#ifndef _TSYSERROR_H_
#define _TSYSERROR_H_

#include <TNamed.h>

class TList;
class TGraphErrors;
class TSysError : public TNamed
{

public:
   TSysError();
   TSysError(const char *name, const char *title);
   ~TSysError();

   TList            *GetList() const { return fList; }
   TGraphErrors     *GetGraph() const { return fGraph; }
   void              SetUseWeight(Bool_t useWeight){ fUseWeight = useWeight; }
   void              SetDelta(Double_t delta) { fDelta = delta; }
   void              SetGraph(TGraphErrors *gr, Bool_t doClone = kFALSE);
   void              Add(TSysError *sysError);
   Bool_t            AddGraph(const char *filename, const char *tmpl = "%lg %lg %lg");
   Bool_t            AddGraphDirectory(const char *dirname, const char *filter = "*.txt",
                                       const char *tmpl = "%lg %lg %lg");
   void              Calculate();

private:
   TList             *fList;      // list of TSysError 
   TGraphErrors      *fGraph;     // pointer to current graph

   Bool_t             fUseWeight; // flag to use weight (it is used when possible)
   Double_t           fMean;      // mean value
   Double_t           fSigma;     // sigma
   Double_t           fAlpha;     // alpha (sigma/sqrt(N))

   Double_t           fDelta;  // small value used for coparison of 2 doubles

   ClassDef(TSysError, 1);
};

#endif /* _TSYSERROR_H_ */
