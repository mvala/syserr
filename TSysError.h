// -*-c++-*-

#ifndef _TSYSERROR_H_
#define _TSYSERROR_H_

#include <TNamed.h>

class TList;
class TGraphErrors;
class TSysError : public TNamed
{

public:

   enum EType { kNone = 0, kMean, kMinStdDev, kNumTypes };

   TSysError();
   TSysError(const char *name, const char *title);
   ~TSysError();

   void              SetGraph(TGraphErrors *gr, Bool_t doClone = kFALSE);
   void              SetHistogram(TH1D *h);
   void              SetType(TSysError::EType type) { fType = type; }
   void              SetTypeToList(TSysError::EType type);
   void              SetPrintInfo(Bool_t printInfo) { fPrintInfo = printInfo; }

   TList            *GetList() const { return fList; }
   TList            *GetInputList() const { return fInputList; }
   TList            *GetOutputList() const { return fOutputList; }
   TGraphErrors     *GetGraph() const { return fGraph; }
   TH1D             *GetHistogram() const { return fHist; }
   EType             GetType() const { return fType; }

   void              PrintHistogramInfo(TSysError *se, TH1D *h = 0);

   void              Add(TSysError *sysError);
   Bool_t            AddGraph(const char *filename, const char *tmpl = "%lg %lg %lg");
   Bool_t            AddGraphDirectory(const char *dirname, const char *filter = "*.txt",
                                       const char *tmpl = "%lg %lg %lg");

   void              AddInput(TObject *o);
   void              AddOutput(TObject *o);
   Bool_t            Calculate();
   Bool_t            CalculateMean();
   Bool_t            CalculateMinStdDev();

private:
   TList             *fList;        // list of TSysError
   TGraphErrors      *fGraph;       // current graph
   TH1D              *fHist;        // current histogram (representation of fGraph)

   TList             *fInputList;   // list of TSysError
   TList             *fOutputList;  // list of TSysError

   EType              fType;        // type of calculation
   Bool_t             fPrintInfo;   // flag if info should be printed (default is kFALSE)

   ClassDef(TSysError, 1)
};

#endif /* _TSYSERROR_H_ */
