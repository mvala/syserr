// -*-c++-*-

#ifndef _TSYSERROR_H_
#define _TSYSERROR_H_

#include <TNamed.h>

class TList;
class TGraphErrors;
class TSysError : public TNamed
{

public:

   enum EType { kNone=0, kMean, kNumTypes };

   TSysError();
   TSysError(const char *name, const char *title);
   ~TSysError();

   void              SetGraph(TGraphErrors *gr, Bool_t doClone = kFALSE);
   void              SetType(TSysError::EType type) { fType = type; }
   void              SetTypeToList(TSysError::EType type);

   TList            *GetList() const { return fList; }
   TGraphErrors     *GetGraph() const { return fGraph; }
   TH1D             *GetHistogram() const { return fHist; }

   void              Add(TSysError *sysError);
   Bool_t            AddGraph(const char *filename, const char *tmpl = "%lg %lg %lg");
   Bool_t            AddGraphDirectory(const char *dirname, const char *filter = "*.txt",
                                       const char *tmpl = "%lg %lg %lg");

   Double_t          Calculate();
   Double_t          CalculateMean();

private:
   TList             *fList;      // list of TSysError 
   TGraphErrors      *fGraph;     // current graph
   TH1D              *fHist;      // current histogram (representation of fGraph)

   EType              fType;

   ClassDef(TSysError, 1)
};

#endif /* _TSYSERROR_H_ */
