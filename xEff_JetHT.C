
#include <vector>
#include <iostream>
#include <Riostream.h>
#include <TH1D.h>
#include <TH1F.h>
#include <TRandom.h>
#include <TLorentzVector.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TGraphAsymmErrors.h>
#include <TH2F.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TLatex.h>
#include <TPad.h>
#include <TFrame.h>
#include <TEfficiency.h>


#include "untuplizer_v07.h"
#include "deltaRdeltaPhiComputation.h"
#include "tdrstyle.C"

// void xEff_JetHT(const char* inpaths,int npaths) 
// void xEff_JetHT(const char* inpaths) {
void xEff_JetHT(vector<string> inpaths) {
    int countNum = 0;
    int countDen = 0;

    // get TTree from file ...
    TreeReader data(inpaths);

    // useful to determine which type of variable to use for which branches
    // data.Print();

    // ----------- HISTOS ----------------

    TH1F* h_muPtDen = new TH1F("h_muPtDen", "h_muPtDen", 30, 0, 150);
    TH1F* h_muPtNum = new TH1F("h_muPtNum", "h_muPtNum", 30, 0, 150);

    // ----------- END HISTOS ------------

    // event loop
    for (Long64_t ev = 0; ev < data.GetEntriesFast(); ev++) {
        data.GetEntry(ev);
        if(ev == 0){
            cout << "Total events" << " " << data.GetEntriesFast() << endl;
        }


        // if(ev % 1000 == 0) {
        //     cout << "##########################" << endl;
        //     cout << "ev: " << ev << endl;
        // } 


        // double muon+photon trigger bit
        ULong64_t HLTEleMuX = (ULong64_t)data.GetLong64("HLTEleMuX");
        bool hltMuPhoBit = (HLTEleMuX&2) ? true : false;



        // jet trigger bit
        ULong64_t HLTJet = (ULong64_t)data.GetLong64("HLTJet");
        bool hltJetBit = (HLTJet&1024 || HLTJet&2048 || HLTJet&4096 || HLTJet&8192 || HLTJet&16384 || HLTJet&32768 || HLTJet&65536 || HLTJet&131072 || HLTJet&262144 || HLTJet&524288) ? true : false;

        // jet trigger matching
        Int_t nJet = data.GetInt("nJet");
        // float* jetPt = data.GetPtrFloat("jetPt");
        bool jetTriggerMatchBit = false;
        if (nJet > 0) {
            ULong64_t* jetFiredTrgs = (ULong64_t*)data.GetPtrLong64("jetFiredTrgs");
            jetTriggerMatchBit = (jetFiredTrgs[0]&1 || jetFiredTrgs[0]&2 || jetFiredTrgs[0]&4 || jetFiredTrgs[0]&8 || jetFiredTrgs[0]&16 || jetFiredTrgs[0]&32 || jetFiredTrgs[0]&64 || jetFiredTrgs[0]&128 || jetFiredTrgs[0]&256 || jetFiredTrgs[0]&512) ? true : false;
        } 
        
        // cout << "jetTriggerMatchBit: " << jetTriggerMatchBit << endl;
        
        // photon jet deltaR
        Int_t nPho = data.GetInt("nPho");
        bool phoJetDeltaR = false;
        if (nPho >0 ){
            float* phoEta = data.GetPtrFloat("phoEta");
            float* phoPhi = data.GetPtrFloat("phoPhi");
            float* jetEta = data.GetPtrFloat("jetEta");
            float* jetPhi = data.GetPtrFloat("jetPhi");
            phoJetDeltaR = (deltaR(*phoEta, *phoPhi, *jetEta, *jetPhi) > 1.0) ? true : false;    
        }
        


        // denominator bit
        bool denominatorBit = hltJetBit && jetTriggerMatchBit;
        // bool denominatorBit = hltJetBit;
        // cout << "denominatorBit: " << denominatorBit << endl;

        // numerator bit
        // bool numeratorBit = denominatorBit && hltMuPhoBit && phoJetDeltaR;
        bool numeratorBit = denominatorBit && hltMuPhoBit;

        // cout << "#########" << endl;
        //  cout << "denominatorBit: " << denominatorBit << endl;
        //  cout << "hltMuPhoBit: " << hltMuPhoBit << endl;
        //  cout << "phoJetDeltaR: " << phoJetDeltaR << endl;


        // muon pt
        float* muPt = data.GetPtrFloat("muPt");

        // fill histos - muon
        Int_t nMu = data.GetInt("nMu");
        if (nMu > 0) {
            if (denominatorBit) {
                countDen++;
                h_muPtDen->Fill(muPt[0]);
            } 
            if (numeratorBit) {
                // cout << "numeratorBit: " << numeratorBit << endl;
                countNum++;
                h_muPtNum->Fill(muPt[0]);
            }
        }


    } //end for

    cout << "countNum: " << countNum << endl;
    cout << "countDen: " << countDen << endl;

    // Efficiency
    TEfficiency* h_EffPtMu = new TEfficiency(*h_muPtNum, *h_muPtDen);
    TCanvas* c1 = new TCanvas();
    h_EffPtMu->Draw("AP");
    gSystem->Exec("rm -rf plots ; mkdir plots");
    c1->SaveAs("plots/h_EffPtMu.png");
    c1->SaveAs("plots/h_EffPtMu.pdf");
    c1->SaveAs("plots/h_EffPtMu.root");




} //xEff_JetHT