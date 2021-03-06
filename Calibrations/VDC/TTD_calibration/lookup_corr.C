/**************************************************************
  lookup_corr.C
  John Williamson    
  9th March, 2021

  Calibrate and plot effect of angular correction to lookup TTD method.

  Based on two MIT theses:
  - V. Jordan, MIT, 1994
  - W. Schmitt, MIT, 1993

  
*************************************************************/


#include "Load_more_rootfiles.C"
#include "file_def.h"
#include "TTD_namespace.h"
#include "TTDTable.h"

#define MAX_ENTRIES 1000000
#define MAX_HIT 1000
#define NPLANE 4

#define DEG_TO_RAD 0.017453278

const Double_t wire_sep = 0.0042426; // seperation of wires in VDC plane

const Double_t kTimeRes = 0.5e-9;    // (s)

std::vector<Double_t> wtime[NPLANE];
std::vector<Double_t> tanTh[NPLANE];
std::vector<Double_t> tanTh_alt[NPLANE];
std::vector<Double_t> trdist[NPLANE];

// cluster info 
std::vector<Int_t> WireNumbers[NPLANE];
std::vector<Int_t> CentralWireNo[NPLANE]; // Central wire no for cluster
vector <int> BegWireNo[NPLANE]; // Central wire no for cluster
std::vector<Int_t> EndWireNo[NPLANE]; // Central wire no for cluster

Int_t    nent[NPLANE];

// seperate vectors for 'good' wires that pass conditions of 5 consecutive wire hits 

std::vector<Double_t> wtimeGood[NPLANE];
std::vector<Double_t> tanThGood[NPLANE];
std::vector<Double_t> tanTh_altGood[NPLANE];
std::vector<Double_t> trdistGood[NPLANE];

// cluster info 
std::vector<Int_t> WireNumbersGood[NPLANE];
std::vector<Int_t> CentralWireNoGood[NPLANE]; // Central wire no for cluster
vector <int> BegWireNoGood[NPLANE]; // Central wire no for cluster
std::vector<Int_t> EndWireNoGood[NPLANE]; // Central wire no for cluster


Int_t    this_plane;

TTDTable* TTDTables[NPLANE];




double fcn(const double* par) {

  
  Double_t chisq = 0.0;
  Double_t delta = 0.0;

  
  for (Int_t i=0; i<nent[this_plane]; i++) {
    delta =  TTD_func::TTD_Corr(TTDTables[this_plane]->Convert(wtime[this_plane][i]), 1/tanTh_alt[this_plane][i],&(par[0])) - trdist[this_plane][i];

    
    chisq += delta*delta;
  }


  double f = chisq;

  return f;
}


void lookup_corr(const char *arm, Int_t runnumber = -1){

   
  TChain* T = new TChain("T");

  if(!strcmp(arm,"L")){
  T = Load_more_rootfiles(runnumber);
  }
  else if (!strcmp(arm,"R")){  
    T = Load_more_rootfiles(runnumber);
  }
  else{
    cout << "arm must be L or R, " << arm << " not acceptable" << endl;
    return;
  }



  // PID cuts (different for Left and Right arms)
  Double_t Cer_cut = 0.0;
  Double_t Ps_cut = 0.0;
  Double_t Ps_Sh_cut_l = 0.0;
  Double_t Ps_Sh_cut_h = 0.0;
  

  Int_t trg = 0;
  // here is where trigger is defined
  if(!strcmp(arm,"L")){
    trg = 1;
    Cer_cut = 1500.0;
    Ps_cut = 0.3;
    Ps_Sh_cut_l = 0.625;
    Ps_Sh_cut_h = 1.1;
    cout << "left trigger" << endl;
  }
  else if (!strcmp(arm,"R")){
    cout << "right trigger" << endl;
    trg = 5;
    Cer_cut = 650.0;
    Ps_cut = 0.2;
    Ps_Sh_cut_l = 0.51;
    Ps_Sh_cut_h = 1.11;
    // trg = 6;
  }
  else{
    cout << "arm must be L or R, " << arm << " not acceptable" << endl;
    return;

  }


  const char plane[NPLANE][8] = {"u1", "u2", "v1", "v2"};
  Double_t ang[NPLANE] = {-45.0, -45.0, 45.0, 45.0};

  Double_t nhit[NPLANE], ntr;
  Double_t hittime[NPLANE][MAX_HIT], hittrknum[NPLANE][MAX_HIT],
    hittrdist[NPLANE][MAX_HIT];
  Double_t d_th[MAX_HIT], d_ph[MAX_HIT];
  Double_t cer_sum, ps_e, sh_e;
  Double_t tr_p[100];
  THaEvent* evt = 0;


  Int_t    ClustCount[NPLANE]; // count number of clusters sorted through

  // counting events and clusters passing condition
  Int_t    nentGood[NPLANE];
  Int_t    ClustCountGood[NPLANE];

  
  
  //  Int_t i, j, hit;

  Double_t evttype;

  // cluster slope and intercept from analyzer
  Double_t slope[NPLANE][MAX_HIT];
  Double_t intercept[NPLANE][MAX_HIT]; 

  Double_t nclust[NPLANE];
  Double_t clsiz[NPLANE][MAX_HIT];
  Double_t wireno[NPLANE][MAX_HIT];
  Double_t clpivot[NPLANE][MAX_HIT];
  Double_t clbeg[NPLANE][MAX_HIT];
  Double_t clend[NPLANE][MAX_HIT];
  

  
  // Set up branches

  T->SetBranchStatus("Event_Branch*", kTRUE);
  T->SetBranchAddress("Event_Branch", &evt);

  T->SetBranchStatus("DR.evtypebits", kTRUE);
  T->SetBranchAddress("DR.evtypebits", &evttype);

  T->SetBranchStatus(Form("%s.tr.n", arm), kTRUE);
  T->SetBranchAddress(Form("%s.tr.n", arm), &ntr);

  T->SetBranchStatus(Form("%s.tr.d_th", arm), kTRUE);
  T->SetBranchAddress(Form("%s.tr.d_th", arm), &d_th);
  T->SetBranchStatus(Form("%s.tr.d_ph", arm), kTRUE);
  T->SetBranchAddress(Form("%s.tr.d_ph", arm), &d_ph);

  T->SetBranchStatus(Form("%s.tr.p",arm),kTRUE);
  T->SetBranchAddress(Form("%s.tr.p",arm),tr_p);
  
  T->SetBranchStatus(Form("%s.cer.asum_c",arm),kTRUE);
  T->SetBranchAddress(Form("%s.cer.asum_c",arm),&cer_sum);
  
  if(!strcmp(arm,"L")){
    T->SetBranchStatus("L.prl1.e",kTRUE);
    T->SetBranchAddress("L.prl1.e",&ps_e);
    T->SetBranchStatus("L.prl2.e",kTRUE);
    T->SetBranchAddress("L.prl2.e",&sh_e);
  }
  else if(!strcmp(arm,"R")){
    T->SetBranchStatus("R.ps.e",kTRUE);
    T->SetBranchAddress("R.ps.e",&ps_e);
    T->SetBranchStatus("R.ps.e",kTRUE);
    T->SetBranchAddress("R.sh.e",&sh_e);
  }
  

  for(Int_t i = 0; i < NPLANE; i++ ){
    T->SetBranchStatus(Form("%s.vdc.%s.nhit", arm, plane[i]), kTRUE);
    T->SetBranchAddress(Form("%s.vdc.%s.nhit", arm, plane[i]), &nhit[i]);

    T->SetBranchStatus(Form("%s.vdc.%s.time", arm, plane[i]), kTRUE);
    T->SetBranchAddress(Form("%s.vdc.%s.time", arm, plane[i]), hittime[i]);

    T->SetBranchStatus(Form("%s.vdc.%s.trknum", arm, plane[i]), kTRUE);
    T->SetBranchAddress(Form("%s.vdc.%s.trknum", arm, plane[i]), hittrknum[i]);

//     T->SetBranchStatus(Form("%s.vdc.%s.ltrdist", arm, plane[i]), kTRUE);
//     T->SetBranchAddress(Form("%s.vdc.%s.ltrdist", arm, plane[i]), hittrdist[i]);
    T->SetBranchStatus(Form("%s.vdc.%s.trdist", arm, plane[i]), kTRUE);
    T->SetBranchAddress(Form("%s.vdc.%s.trdist", arm, plane[i]), hittrdist[i]);

    T->SetBranchStatus(Form("%s.vdc.%s.slope", arm, plane[i]), kTRUE);
    T->SetBranchAddress(Form("%s.vdc.%s.slope", arm, plane[i]), slope[i]);

    T->SetBranchStatus(Form("%s.vdc.%s.clpos", arm, plane[i]), kTRUE);
    T->SetBranchAddress(Form("%s.vdc.%s.clpos", arm, plane[i]), intercept[i]);

    T->SetBranchStatus(Form("%s.vdc.%s.nclust", arm, plane[i]), kTRUE);
    T->SetBranchAddress(Form("%s.vdc.%s.nclust", arm, plane[i]), &nclust[i]);

    T->SetBranchStatus(Form("%s.vdc.%s.clsiz", arm, plane[i]), kTRUE);
    T->SetBranchAddress(Form("%s.vdc.%s.clsiz", arm, plane[i]), clsiz[i]);
    
    T->SetBranchStatus(Form("%s.vdc.%s.wire", arm, plane[i]), kTRUE);
    T->SetBranchAddress(Form("%s.vdc.%s.wire", arm, plane[i]), wireno[i]);

    T->SetBranchStatus(Form("%s.vdc.%s.clpivot", arm, plane[i]), kTRUE);
    T->SetBranchAddress(Form("%s.vdc.%s.clpivot", arm, plane[i]), clpivot[i]);

    T->SetBranchStatus(Form("%s.vdc.%s.clbeg", arm, plane[i]), kTRUE);
    T->SetBranchAddress(Form("%s.vdc.%s.clbeg", arm, plane[i]), clbeg[i]);

    T->SetBranchStatus(Form("%s.vdc.%s.clend", arm, plane[i]), kTRUE);
    T->SetBranchAddress(Form("%s.vdc.%s.clend", arm, plane[i]), clend[i]);

    nent[i] = 0;
    ClustCount[i] = 0;
    ClustCountGood[i] = 0;
  }

  Double_t this_slope;

  Int_t NEntries = T->GetEntries();

  
  NEntries = (NEntries < 2e5) ? NEntries : 2e5;


  
  for(Int_t i = 0; i < NEntries; i++ ){
    T->GetEntry(i);

    if( (i%5000)==0 ) { cout << "Entry " << i << endl; }
    
    // if( ntr == 1 && TTD_func::passtrg(Int_t(evttype), trg) && cer_sum > Cer_cut && ps_e/(1e3*tr_p[0]) > Ps_cut && (ps_e+sh_e)/(1e3*tr_p[0]) > Ps_Sh_cut_l &&  (ps_e+sh_e)/(1e3*tr_p[0]) < Ps_Sh_cut_h ){
      if( ntr == 1 && TTD_func::passtrg(Int_t(evttype), trg) && cer_sum > Cer_cut){
      for(Int_t j = 0; j < NPLANE; j++ ){
	this_slope = d_th[0]*cos(ang[j]*DEG_TO_RAD) 
	  + d_ph[0]*sin(ang[j]*DEG_TO_RAD);
	
			
	Bool_t NewClust = kTRUE;
	std::vector<Double_t> Clust_times; // vector to hold cluster hits times
	std::vector<Double_t> Clust_tantheta; // vector to hold cluster tanetheta
	std::vector<Double_t> Clust_tantheta_alt; // vector to hold cluster tantheta alt
	std::vector<Double_t> Clust_dist; // vector to hold cluster dist
	std::vector<Double_t> Clust_wires; // vector to hold wire numbers
	std::vector<Bool_t> Clust_good; // vector to hold bool for 5 cluster events

	Int_t Pivot = 0; // var to hold pivot number
	Int_t BegWire = 0; // var to hold first wire
	Int_t EndWire = 0; // var to hold end wire	    

	for(Int_t hit = 0; hit < nhit[j]  && nent[j] < MAX_ENTRIES; hit++ ){
	  
	  if( 0 < hittime[j][hit] 
	      // &&  hittime[j][hit]  < 260.0e-9
	      // && hittrdist[j][hit]< 0.015
	      && hittime[j][hit]  < 1 // eliminate events with default large value for time
	      && TMath::Abs(hittrdist[j][hit])< 1 // eliminate events with default large value for distance
		  //		  && hittrknum[j][hit] == 1
	      && wireno[j][hit] >= clbeg[j][0]
	      && wireno[j][hit] <= clend[j][0]
	      ){
	    
	    
	    Clust_times.push_back(hittime[j][hit]);
	    Clust_tantheta.push_back(this_slope);
	    Clust_tantheta_alt.push_back(slope[j][0]);
	    Clust_dist.push_back(hittrdist[j][hit]);
	    Clust_wires.push_back(wireno[j][hit]);
	    
	    
	    if(NewClust){	      
	      Pivot = clpivot[j][0];
	      BegWire = clbeg[j][0];
	      EndWire = clend[j][0];
	      
	      NewClust = kFALSE;
	    }
	    
	  }
	}
	
	
	// check that clust vector passed conditions	
	
	if(Clust_wires.size() == 5){
	  if(Clust_wires[0] >= BegWire && Clust_wires[5] <= EndWire){
	    
	    ClustCountGood[j]++;
	    CentralWireNoGood[j].push_back(Pivot);
	    BegWireNoGood[j].push_back(BegWire);
	    EndWireNoGood[j].push_back(EndWire);
	    
	    
	    for(Int_t k = 0; k<5; k++){
	      wtimeGood[j].push_back(Clust_times[k]);
	      tanThGood[j].push_back(Clust_tantheta[k]);
	      tanTh_altGood[j].push_back(Clust_tantheta_alt[k]);
	      trdistGood[j].push_back(Clust_dist[k]);
	      WireNumbersGood[j].push_back(Clust_wires[k]);
	      nentGood[j]++;
	    }	    	    
	  }
	}
	
	ClustCount[j]++;
	CentralWireNo[j].push_back(Pivot);
	BegWireNo[j].push_back(BegWire);
	EndWireNo[j].push_back(EndWire);
	
	  
	  for(Int_t k = 0; k<Clust_wires.size(); k++){
	    wtime[j].push_back(Clust_times[k]);
	    tanTh[j].push_back(Clust_tantheta[k]);
	    tanTh_alt[j].push_back(Clust_tantheta_alt[k]);
	    trdist[j].push_back(Clust_dist[k]);
	    WireNumbers[j].push_back(Clust_wires[k]);
	    nent[j]++;
	  }	    	    
	  
	  
      }
	
    }
  }
  


  // plot 'real' drift distance versus time spectra for all planes
  TH2D *htime_dist_real[NPLANE];


  for(Int_t i = 0; i < NPLANE; i++ ){

    htime_dist_real[i] = new TH2D(Form("htime_dist_real_%s", plane[i]), Form("%s TTD", plane[i]), 290, -10,280, 200, 0.0, 0.015);
    htime_dist_real[i]->GetXaxis()->SetTitle("Drift Time(ns)");
    htime_dist_real[i]->GetXaxis()->CenterTitle();
    htime_dist_real[i]->GetYaxis()->SetTitle("Track Dist (m)");
    htime_dist_real[i]->GetYaxis()->CenterTitle();

    for(Int_t j = 0; j < nent[i]; j++ ){
      htime_dist_real[i]->Fill(1e9*(wtime[i][j]),trdist[i][j]);
    }

  }

  // noise cut objects (cut based on difference in distance from central distance gaussian at given time)
  NoiseCut* Real_cut[NPLANE];
  
  for(Int_t i = 0; i < NPLANE; i++ ){

    cout << "Pre-cut Nent = " << nent[i] << endl;
    
    
    Real_cut[i] =  new NoiseCut(htime_dist_real[i]);

    bool PassCut = true;

    for(Int_t j = 0; j < nent[i]; j++ ){

      PassCut = Real_cut[i]->PassNoiseCut(wtime[i][j],trdist[i][j]);

      if(!PassCut){
	// event failed noise cut
	wtime[i].erase(wtime[i].begin() + j);
	trdist[i].erase(trdist[i].begin() + j);
	tanTh[i].erase(tanTh[i].begin() + j);
	tanTh_alt[i].erase(tanTh_alt[i].begin() + j);
	j--; // vector length is reduced by one so to get to 'next' element of original vector we need to reduce iterator by one
	nent[i]--;

      }
      
    }


    cout << "Post-cut Nent = " << nent[i] << endl << endl;
    
    
  }


  // form and plot TTD tables for central, begining and end regions of each plane
  
  const char region[3][8] = {"Beg","Cen","End"};



  //  const Double_t RegNo[3] = {0.645,0.695,0.745};
  const Double_t RegNo[3] = {1.45,1.55,1.65};
  
  vector<double> tables[NPLANE][3];

  std::vector<double> Times[NPLANE][3];
  
  // Form histograms of time for three regions in plane

  TH1F *htime[NPLANE][3];

  TH1F* hslope[NPLANE];
  TH1F* hslopeGood[NPLANE];
  

  Double_t high = 450e-9;
  Double_t low = -10.0e-9;
  Int_t num_bins = (high - low)/kTimeRes;

  
  for(Int_t i = 0; i<NPLANE; i++){
    
    hslope[i] = new TH1F(Form("Slope_%s_%s", arm, plane[i]),Form("Slope %s %s", arm, plane[i]), 100, 1, 2);
    hslope[i]->GetXaxis()->SetTitle("Slope");
    
    hslopeGood[i] = new TH1F(Form("Good_Slope_%s_%s", arm, plane[i]),Form("Good Slope %s %s", arm, plane[i]), 100, 1, 2);
    hslopeGood[i]->GetXaxis()->SetTitle("Slope");
    
    for(Int_t j = 0; j<3; j++){
      htime[i][j] = new TH1F(Form("TDC_%s_%s_%s", arm, plane[i],region[j]),Form("TDC spectrum %s %s %s", arm, plane[i],region[j]), num_bins, low, high);
      }
  }


  for(Int_t i = 0; i<NPLANE; i++){
    
    for(Int_t j = 0; j<wtime[i].size(); j++){


      hslope[i]->Fill(1/tanTh_alt[i][j]);
      
      for(Int_t k = 0; k<3; k++){
	
	// condition checks if pivot wire is vicinity of defined regions wires (begining, centre and end of plane)
	if( TMath::Abs(1/tanTh_alt[i][j] - RegNo[k]) < 0.05) {
	  htime[i][k]->Fill(wtime[i][j]);
	}
      }      
    }


    for(Int_t j = 0; j<wtimeGood[i].size(); j++){      
      
      hslopeGood[i]->Fill(1/tanTh_altGood[i][j]);
      
    }
    
  }
  

  TCanvas* c1 = new TCanvas("c1", "Drift Time spectra for angular regions", 640, 480);
  c1->Divide(2,2);

  
  TCanvas* c2 = new TCanvas("c2", "Slopes", 640, 480);
  c2->Divide(2,2);

  TLegend* leg_DTS[NPLANE];

  TLine* Slope_Line[NPLANE][4];
  
  TLegend* leg_slope[NPLANE];
  
  for(Int_t i = 0; i<NPLANE; i++){
    c1->cd(i+1);
    
    htime[i][0]->SetLineColor(kRed);
    htime[i][0]->Draw();

    htime[i][1]->SetLineColor(kBlue);
    htime[i][1]->Draw("same");

    htime[i][2]->SetLineColor(kGreen);
    htime[i][2]->Draw("same");

    leg_DTS[i] = new TLegend(.65,.65,.9,.9,"Key");
    for(Int_t j = 0; j<3; j++){
      leg_DTS[i]->AddEntry(htime[i][j],Form(" Drift-time spectrum %s",region[j]),"l");
    }
    leg_DTS[i]->Draw("same");
    
    c2->cd(i+1);
    
    hslope[i]->Draw();
    hslopeGood[i]->SetLineColor(kRed);
    hslopeGood[i]->Draw("same");

    //Double_t MaxY = gPad->GetUymax();
    Double_t MaxY = hslope[i]->GetMaximum();    
    Slope_Line[i][0] = new TLine(RegNo[0]-0.05,0,RegNo[0]-0.05,MaxY);
    Slope_Line[i][1] = new TLine(RegNo[0]+0.05,0,RegNo[0]+0.05,MaxY);
    Slope_Line[i][2] = new TLine(RegNo[1]+0.05,0,RegNo[1]+0.05,MaxY);
    Slope_Line[i][3] = new TLine(RegNo[2]+0.05,0,RegNo[2]+0.05,MaxY);

    for(Int_t j = 0; j<4; j++){
      Slope_Line[i][j]->SetLineStyle(9);
      Slope_Line[i][j]->Draw("same");
    }
    
    
    leg_slope[i] = new TLegend(.1,.65,.37,.9,"Key");
    leg_slope[i]->AddEntry(hslope[i],"All slopes","l");
    leg_slope[i]->AddEntry(hslopeGood[i],"5-wire cluster slopes","l");
    leg_slope[i]->AddEntry(Slope_Line[i][0],"Boundaries of Beg, Cen and End regions","l");
    leg_slope[i]->Draw("same");
    
  }


  // form lookup tables for different angular regions
  
  // find first and last bins in time with entries for each plane    
  Int_t low_bin[NPLANE][3] = {0};
  Int_t high_bin[NPLANE][3] = {0};

    
  Double_t low_val[NPLANE][3] = {0};
  Double_t high_val[NPLANE][3] = {0};

 

  Bool_t first[NPLANE][3] = {false};

  for(Int_t i = 0; i<NPLANE; i++ ){
    for(Int_t j = 0; j<3; j++ ){
      first[i][j] = true;
    }
  }
  
  for(Int_t i = 0; i<NPLANE; i++ ){
    for(Int_t j = 0; j<3; j++ ){
      for(Int_t k = 0; k<htime[i][j]->GetNbinsX(); k++){
	
	if (htime[i][j]->GetBinContent(k) > 0){
	  
	  high_bin[i][j] = k;
	  
	  if (first[i][j] ){
	    low_bin[i][j] = k;
	    low_val[i][j] = (low_bin[i][j]+0.5)*kTimeRes + low;
	    first[i][j] = false;
	  }
	}            
      }
    }
  }

    
  // set provisional value of K normlisation parameter (determined by size of drift cell)
  Double_t K[NPLANE][3] = {1.0};
  

  
  Int_t NBins[NPLANE][3] = {0}; // per-plane entries in look-up tables   
  for(Int_t i = 0; i < NPLANE; i++ ){
    for(Int_t j = 0; j<3; j++ ){

      NBins[i][j] = high_bin[i][j] - low_bin[i][j];

      cout << "Plane: " << plane[i] << " region: " << region[j] << ", low_bin = " << low_bin[i][j] << ", high_bin = " << high_bin[i][j] << ", NBins = " << NBins[i][j] << endl;
      
      K[i][j] = 1.0;
    }
  }
     

  for(Int_t i = 0; i < NPLANE; i++ ){

    for(Int_t j = 0; j<3; j++){
        
      tables[i][j].push_back(htime[i][j]->GetBinContent(low_bin[i][j]));      
      // cout << "table[" << i << "][" << j <<"][0] = " << tables[i][j][0] << endl;
      // cout << "htime[" << i << "][" << j << "]->GetBinContent(low_bin[" << i<< "][" << j << "]) = " << htime[i][j]->GetBinContent(low_bin[i][j]) << endl;
      // cout << "low_bin[" << i << "][" << j << "] = " << low_bin[i][j] << endl;
      
      Times[i][j].push_back(htime[i][j]->GetBinCenter(low_bin[i][j]));
      // cout << "Bin center = " << htime[i][j]->GetBinCenter(low_bin[i][j]) << endl << endl;
      

      for(Int_t k=1; k<NBins[i][j]; k++) {
	tables[i][j].push_back(tables[i][j][k-1] + K[i][j]*htime[i][j]->GetBinContent(k+low_bin[i][j]));
	Times[i][j].push_back(htime[i][j]->GetBinCenter(k+low_bin[i][j]));
	//	cout << "i = " << i << ", j = " << j << ": Table = " << tables[i][j][k-1] + K[i][j]*htime[i][j]->GetBinContent(k+low_bin[i][j]) << ", time = " << htime[i][j]->GetBinCenter(k+low_bin[i][j]) << endl;
	
      }
      //      cout << endl;
    }

  }
  

  // plot TTD Tables

  TGraph* TableGraph[NPLANE][3];

  TMultiGraph* mg[NPLANE];

  TLegend *leg_TTD[NPLANE];
  
  Int_t colours[3] = {kRed,kBlue,kGreen};
  
  for(Int_t i = 0; i<NPLANE; i++){
    
    mg[i] = new TMultiGraph();

    leg_TTD[i] = new TLegend(.1,.65,.37,.9,"Key");
    
    for(Int_t j = 0; j<3; j++){
      //      Int_t VecSize = Times[i][j].size();
      //      TableGraph[i][j] = new TGraph(Times[i][j].size(),&(Times[i][j][0]),&(tables[i][j][0]));
      TableGraph[i][j] = new TGraph(Times[i][j].size(),&(Times[i][j][0]),&(tables[i][j][0]));
      TableGraph[i][j]->SetMarkerColor(colours[j]);
      TableGraph[i][j]->SetLineColor(colours[j]);
      TableGraph[i][j]->SetTitle(Form("%s %s TTD Table",plane[i],region[j]));

      mg[i]->Add(TableGraph[i][j]);
      leg_TTD[i]->AddEntry(TableGraph[i][j],Form("%s",region[j]),"l");
    }
  }


  TCanvas* c3 = new TCanvas("c3", "c3", 640, 480);
  c3->Divide(2,2);

  for(Int_t i = 0; i<NPLANE; i++){

    c3->cd(i+1);
    mg[i]->Draw("apl");

    leg_TTD[i]->Draw("same");
    // mg[i]->GetXaxis()->SetTitle("Time (s)");
    // mg[i]->GetYaxis()->SetTitle("Distance");
    mg[i]->SetTitle(Form("%s TTD Table; Time (s); Distance",plane[i]));
  }



  // place lookup tables in classes

  TTDTable* PlaneTables[NPLANE][3];
  
  for(Int_t i = 0; i < NPLANE; i++ ){
    for(Int_t j = 0; j<3; j++){
      
      PlaneTables[i][j] = new TTDTable(tables[i][j],low_val[i][j],NBins[i][j]);
    }
  }
      

  cout << endl;

  
  // normalise TTD lookup tables

  TH1D *hW1W2TTD[NPLANE][3];
  TH1D *hW4W5TTD[NPLANE][3];
  
  for(Int_t i = 0; i < NPLANE; i++ ){

    Double_t norm_max = 0.0;
    Double_t norm_low= 0.0;
    
    for(Int_t j = 0; j<3; j++){
    
      Double_t norm_max_test = tables[i][j].back()/0.013;

      if(norm_max_test > norm_max){
	norm_max = norm_max_test;
      }

      Double_t norm_low_test = norm_max_test/2.;
      
      if(norm_low == 0 || norm_low_test < norm_low){
	norm_low = norm_low_test;
      }      
    }


    for(Int_t j = 0; j<3; j++){
      
      hW1W2TTD[i][j] = new TH1D(Form("hW1W2TTD_%s_%s", plane[i],region[j]), Form("(Wires 1 & 2) TTD Normsalistion %s %s", plane[i],region[j]),100,norm_low,norm_max);
      hW1W2TTD[i][j]->GetXaxis()->SetTitle("TTD normalisation");
      hW1W2TTD[i][j]->SetLineColor(colours[j]);      
      
      
      hW4W5TTD[i][j] = (TH1D*) hW1W2TTD[i][j]->Clone(Form("hW1W2TTD_%s_%s", plane[i],region[j]));
      hW4W5TTD[i][j]->SetTitle( Form("(Wires 4 & 5) TTD Normsalistion %s %s", plane[i],region[j]));
    }

    cout << plane[i] << ": # good clusts = " << ClustCountGood[i] << endl;
    
    for(Int_t k = 0; k<ClustCountGood[i]; k++ ){

      //      cout << "plane = " << plane[i] << ", ClustNo = " << k << endl;
      
      Bool_t GoodClust = kTRUE;
      
      for(Int_t l = 0; l<5; l++){
	
	// check that wires are consecutive (no gaps in cluster)
	if(l>0){
	  if(! (WireNumbersGood[i][k*5 + l] == (WireNumbersGood[i][k*5 + l - 1])+1) ){
	    GoodClust = kFALSE;
	    break;
	  }	  
	}
	
	
	// check that pivot wire in cluster ins 3rd wire (5 wire clusters, checking that pivot is central wire)
	if(l==2){	  
	  if(!(WireNumbersGood[i][k*5 + l] == CentralWireNoGood[i][k])){
	    GoodClust = kFALSE;
	    break;
	  }
	    
	}
      }
      
      
      if(GoodClust){
	
	  Double_t AnaDiff_1_2 = -(trdistGood[i][k*5 + 1] - trdistGood[i][k*5 + 0]);
	  Double_t CalcDiff_1_2 = (1/tanTh_altGood[i][k*5+1]) * (wire_sep);
	  
	  Double_t AnaDiff_4_5 = trdistGood[i][k*5 + 4] - trdistGood[i][k*5 + 3];
	  Double_t CalcDiff_4_5 = (1/tanTh_altGood[i][k*5+3]) * (wire_sep);


	  for(Int_t j = 0; j<3; j++){
	    
	    if( TMath::Abs(1/tanTh_altGood[i][k*5+2] - RegNo[j]) < 0.05) {

	      
	      Double_t TTDTableDiff_1_2 = PlaneTables[i][j]->Convert(wtimeGood[i][k*5+1])-PlaneTables[i][j]->Convert(wtimeGood[i][k*5+0]);
	      
	      Double_t TTDW1W2Norm = -TTDTableDiff_1_2/CalcDiff_1_2;
	      
	      
	      Double_t TTDTableDiff_4_5 = PlaneTables[i][j]->Convert(wtimeGood[i][k*5+4])-PlaneTables[i][j]->Convert(wtimeGood[i][k*5+3]);
	      
	      Double_t TTDW4W5Norm = TTDTableDiff_4_5/CalcDiff_4_5;
	      
	      //	      hW1W2TTDSlope[i][j]->Fill(1/tanTh_alt[i][k*5+3],TTDW1W2Norm);



	      hW1W2TTD[i][j]->Fill(TTDW1W2Norm);
	  

	      hW4W5TTD[i][j]->Fill(TTDW4W5Norm);
	    }
	  }

      }
	  
    }           	
  }
   
  

  // plot normalisations for each plane and region
  
  TCanvas* c4 = new TCanvas("c4", "c4", 640, 480);
  c4->Divide(2,2);

  // mean of normalisation

  Double_t TTDNormMeans[NPLANE][3];

  for(Int_t i = 0; i<NPLANE; i++){
    c4->cd(i+1);
    for(Int_t j = 0; j<3; j++){

      if(j == 0){
	hW1W2TTD[i][j]->Draw();
	TTDNormMeans[i][j] = hW1W2TTD[i][j]->GetMean();
      }
      else{
	hW1W2TTD[i][j]->Draw("same");
	TTDNormMeans[i][j] = hW1W2TTD[i][j]->GetMean();
      }
      
    }
    leg_TTD[i]->Draw("same");
  }


  // normalise tables

  for(Int_t i = 0; i < NPLANE; i++ ){
    for(Int_t j = 0; j < 3; j++ ){
      for(Int_t k=0; k<NBins[i][j]; k++){

	tables[i][j][k] /= TTDNormMeans[i][j];
	
      }      
    }
  }


  //plot normalised TTD tables
  
  TGraph* TableGraphNorm[NPLANE][3];

  TMultiGraph* mgNorm[NPLANE];

    
  for(Int_t i = 0; i<NPLANE; i++){
    
    mgNorm[i] = new TMultiGraph();

    
    for(Int_t j = 0; j<3; j++){

      TableGraphNorm[i][j] = new TGraph(Times[i][j].size(),&(Times[i][j][0]),&(tables[i][j][0]));
      TableGraphNorm[i][j]->SetMarkerColor(colours[j]);
      TableGraphNorm[i][j]->SetLineColor(colours[j]);
      TableGraphNorm[i][j]->SetTitle(Form("%s %s TTD Table",plane[i],region[j]));

      mgNorm[i]->Add(TableGraphNorm[i][j]);
    }
  }


  TCanvas* c5 = new TCanvas("c5", "Normalised TTD tables", 640, 480);
  c5->Divide(2,2);

  for(Int_t i = 0; i<NPLANE; i++){

    c5->cd(i+1);
    mgNorm[i]->Draw("apl");

    leg_TTD[i]->Draw("same");
    mgNorm[i]->SetTitle(Form("%s TTD Table; Time (s); Distance",plane[i]));
  }


  // read normalised TTD from DB


  std::vector<Double_t> NormTable[NPLANE]; // velocity lookup table
  Int_t NBinsNorm[NPLANE];
  Double_t LowNorm[NPLANE];


  
  for(Int_t i = 0; i < NPLANE; i++ ){
  
    std::ifstream table_DB(Form("DB/lookup_tables/db_%s_%s_lookup_TTD_norm.vdc.%d.dat",arm,plane[i],runnumber));

    cout << "Reading " << Form("DB/lookup_tables/db_%s_%s_lookup_TTD_norm.vdc.%d.dat",arm,plane[i],runnumber) << endl;
    
    Int_t line_no = 0;
    
    std::string line;

    char* phrase = NULL;
    
    while (std::getline(table_DB, line))
      {
	std::istringstream iss(line);
	
	phrase = Form("%s.vdc.%s.ttd_table.nbins = ",arm,plane[i]);

	if(!line.find(phrase)){
	  
	  std::getline(table_DB, line);

	  NBinsNorm[i] = TTD_func::ReadNBins(line);

	  line_no++;

	}

	
	phrase = Form("%s.vdc.%s.ttd_table.low = ",arm,plane[i]);
	
	if(!line.find(phrase)){
	  
	  
	  std::getline(table_DB, line);

	  LowNorm[i] = TTD_func::ReadSingleVal<Double_t>(line);

	  line_no++;

	}
	

	phrase = Form("%s.vdc.%s.ttd_table.table",arm,plane[i]);


	if(!line.find(phrase)){

	  NormTable[i] = TTD_func::ReadLookupTable(table_DB, line_no, NBinsNorm[i]);
	  TTDTables[i] = new TTDTable(NormTable[i],LowNorm[i],NBinsNorm[i]);

	  line_no++;

	}	

	line_no++;
      }    
  }
  



  for(Int_t i = 0; i < NPLANE; i++){

    cout << "For " << plane[i] << ", Table = " << endl;
    
    Int_t j = 1;
    for( auto val : NormTable[i]){
      cout << val << " ";
      if(j%10==0){
	cout << endl;
      }
      
      j++;
    }

    cout << endl << endl;
  }


  
  // Set-up and perform minimisation


  ROOT::Math::Minimizer* min = ROOT::Math::Factory::CreateMinimizer("Minuit2", "Migrad");
  
  min->SetTolerance(0.001);
  min->SetMaxFunctionCalls(1000000); // for Minuit/Minuit2
  min->SetMaxIterations(10000);  // for GSL


  const Int_t NPara = 2;
  
  ROOT::Math::Functor f(&fcn,NPara); 
  
  min->SetFunction(f);


  // retrieve results of minimisations
  Double_t ROp[NPLANE] = {0.0};
  Double_t Theta0Op[NPLANE] = {0.0};
  Double_t Chi2Op[NPLANE] = {0.0};


  
  for(Int_t i = 0; i < NPLANE; i++ ){
  
    min->SetLimitedVariable(0,  "R", 0.0032, 0.0001, 0.001, 0.005);
    min->SetLimitedVariable(1,  "theta0", 1.4, 0.001,1.0, 2.0);
    
    this_plane = i;
    min->Minimize();
   
    const double *xs = min->X();
    ROp[i] = xs[0];
    Theta0Op[i] = xs[1];
    Chi2Op[i] = min->MinValue();
        
  }

  cout << endl << endl;
  cout << "Minimisation results : "  << endl << endl;

  for(Int_t i = 0; i < NPLANE; i++ ){

    cout << "For " << plane[i] << " : " << endl;
    cout << "R = " << ROp[i] << endl;
    cout << "Theta0 = " << Theta0Op[i] << endl;
    cout << "Chi2Op = " << Chi2Op[i] << endl << endl;
  }

  
  

  /*
  min->SetMaxIterations(GSL_maxCalls);  // for GSL
  min->SetMaxFunctionCalls(Minuit_maxCalls);   // for Minuit/Minuit2
  */



  // plot TTD table comparison to 'real' distance
  // plot with angular corrrections for comparison

  const double R = 0.0021;
  //  const double theta0 = 0.71;
  const double theta0 = 1.4;

  Double_t Pars[NPLANE][2];

  for(Int_t i = 0; i < NPLANE; i++ ){

    Pars[i][0] = ROp[i];
    Pars[i][1] = Theta0Op[i];
    
  }
  // Pars[0] = R;
  // Pars[1] = theta0;
  
  cout << "Pars set" << endl;



  Double_t ExtPars[NPLANE][4] = {0.0};


  for(Int_t i = 0; i < NPLANE; i++ ){
    for(Int_t j = 0; j < 4; j++ ){
      ExtPars[i][j] = 0.0;
    }
  }
  
  

  TTDTable* TTDTablesCorr[NPLANE];

  for(Int_t i = 0; i < NPLANE; i++ ){
      
    TTDTablesCorr[i] = new TTDTable(NormTable[i],LowNorm[i],NBinsNorm[i],Pars[i],ExtPars[i]);

    
  }

  
  
  // difference betwen 'real' dist and TTD dist
  TH1F* hDistDiff[NPLANE];
    // difference betwen 'real' dist and TTD dist versus slope
  TH2F* hDistDiffSlope[NPLANE];


  // difference betwen 'real' dist and TTD dist with correction
  TH1F* hDistDiffCorr[NPLANE];
  // difference betwen 'real' dist and TTD dist with correction versus slope
  TH2F* hDistDiffCorrSlope[NPLANE];

  THStack* hDistDiffStack[NPLANE];


  // plot TTD distance vs real distance
  TH2F *hreal_dist_table[NPLANE];

  // plot table drift distance versus time spectra for all planes
  TH2F *htime_dist_table[NPLANE];

  

  for(Int_t i = 0; i<NPLANE; i++){

    
    hDistDiff[i] = new TH1F(Form("Dist_diff_%s", plane[i]),Form("Dist Diff %s", plane[i]), 100, -0.0015, 0.0015);
    hDistDiff[i]->GetXaxis()->SetTitle("Real distance - Table distance");
    hDistDiff[i]->GetXaxis()->CenterTitle();
    
    hDistDiffSlope[i] = new TH2F(Form("Dist_diff_slope_%s", plane[i]),Form("Lookup table (No Angular correction) %s", plane[i]), 100, 1, 2, 100, -0.0015, 0.0015);
    hDistDiffSlope[i]->GetXaxis()->SetTitle("Slope");
    hDistDiffSlope[i]->GetYaxis()->SetTitle("Real distance - Table distance [m]");
    

    hDistDiffCorr[i] = new TH1F(Form("Dist_diff_corr_%s", plane[i]),Form("Dist Diff Corr %s", plane[i]), 100, -0.0015, 0.0015);
    hDistDiffCorr[i]->GetXaxis()->SetTitle("Real distance - Table distance [m]");
    hDistDiffCorr[i]->GetXaxis()->CenterTitle();
    
    hDistDiffCorrSlope[i] = new TH2F(Form("Dist_diff_corr_slope_%s", plane[i]),Form("Lookup table (with Angular correction) %s", plane[i]), 100, 1, 2, 100, -0.0015, 0.0015);
    hDistDiffCorrSlope[i]->GetXaxis()->SetTitle("Slope");
    hDistDiffCorrSlope[i]->GetYaxis()->SetTitle("Real distance - Table distance [m]");


    hreal_dist_table[i] = new TH2F(Form("hreal_dist_table_%s", plane[i]), Form("%s TTD Lookup table", plane[i]), 200, 0.0, 0.020, 200, 0.0, 0.020);
    hreal_dist_table[i]->GetXaxis()->SetTitle("'Real' Track Dist (m)");
    hreal_dist_table[i]->GetXaxis()->CenterTitle();
    hreal_dist_table[i]->GetYaxis()->SetTitle("Table Track Dist (m)");
    hreal_dist_table[i]->GetYaxis()->CenterTitle();

    htime_dist_table[i] = new TH2F(Form("htime_dist_table_%s", plane[i]), Form("%s TTD Lookup Table (with angular correction)", plane[i]), 760, -30, 350, 200, 0.0, 0.020);
    htime_dist_table[i]->GetXaxis()->SetTitle("Drift Time(ns)");
    htime_dist_table[i]->GetXaxis()->CenterTitle();
    htime_dist_table[i]->GetYaxis()->SetTitle("Dist from Time (m)");
    htime_dist_table[i]->GetYaxis()->CenterTitle();
    
    
    for(Int_t j = 0; j<wtime[i].size(); j++){
      
      Double_t RealDist = trdist[i][j];
      Double_t TTDDist = TTDTables[i]->Convert(wtime[i][j]);
      Double_t Slope = 1/tanTh_alt[i][j];

      Double_t TTDDistCorr = TTDTablesCorr[i]->ConvertAngleCorr(wtime[i][j], Slope);

      
      hDistDiff[i]->Fill(RealDist-TTDDist);
      hDistDiffSlope[i]->Fill(Slope,RealDist-TTDDist);

      hDistDiffCorr[i]->Fill(RealDist-TTDDistCorr);
      hDistDiffCorrSlope[i]->Fill(Slope,RealDist-TTDDistCorr);


      hreal_dist_table[i]->Fill(RealDist, TTDDistCorr);
      htime_dist_table[i]->Fill(wtime[i][j]*1e9, TTDDistCorr);
      
    }    
  }


  TCanvas* c6 = new TCanvas("c6", "Distance Comparison Plots", 1400, 1000);
  c6->Divide(2,2);

  TLegend *leg_comp[NPLANE];

    
  TCanvas* c7 = new TCanvas("c7", "Distance Comparison Plots vs Slope", 1400, 1000);
  c7->Divide(2,2);

  TCanvas* c8 = new TCanvas("c8", "Distance Comparison with correction Plots vs Slope", 1400, 1000);
  c8->Divide(2,2);


  TCanvas* c9 = new TCanvas("c9", "TTD Distance vs time", 1400, 1000);
  c9->Divide(2,2);

  
  TCanvas* c10 = new TCanvas("c10", "TTD Distance vs Real Distamce", 1400, 1000);
  c10->Divide(2,2);
  TLegend *leg_Dist[NPLANE];


  
  // function that display perfect linear correlation between 'real' distance and distance caluclated from time (y = x)  
  TF1 *Lin_cor = new TF1("Lin_cor","pol1(0)", 0.0, 0.015);
  Lin_cor->SetParameter(0,0.0);
  Lin_cor->SetParameter(1,1.0);
  Lin_cor->SetLineStyle(4); // dashed line
  

  for(Int_t i = 0; i<NPLANE; i++){

    c6->cd(i+1);

    hDistDiffStack[i] = new THStack(Form("hDistDiffStack_%i",i),Form("Real - TTD Distance, %s; Real distance - Table distance [m]",plane[i]));
      
    hDistDiffStack[i]->Add(hDistDiff[i]);
    //    hDistDiff[i]->Draw();
    hDistDiffCorr[i]->SetLineColor(kRed);
    hDistDiffStack[i]->Add(hDistDiffCorr[i]);
    //    hDistDiffCorr[i]->Draw("same");
   
    // hDistDiffStack[i]->GetXaxis()->SetTitle("Real distance - Table distance");
    // hDistDiffStack[i]->GetXaxis()->CenterTitle();
    
    hDistDiffStack[i]->Draw("nostack");
    
    leg_comp[i] = new TLegend(.1,.65,.37,.9,"Key");
    leg_comp[i]->SetFillColor(0);
    leg_comp[i]->SetTextSize(0.025);
    leg_comp[i]->AddEntry(hDistDiff[i],"Diff between real and TTD","l");
    leg_comp[i]->AddEntry(hDistDiffCorr[i],"#splitline{Diff between real and TTD}{with correction}","l");
    leg_comp[i]->Draw("same");



    

    c7->cd(i+1);
    gPad->SetLeftMargin(0.15);
    hDistDiffSlope[i]->Draw("colz");
    hDistDiffSlope[i]->GetYaxis()->SetTitleOffset(1.2);


    c8->cd(i+1);
    gPad->SetLeftMargin(0.15);
    hDistDiffCorrSlope[i]->Draw("colz");
    hDistDiffCorrSlope[i]->GetYaxis()->SetTitleOffset(1.2);


    c9->cd(i+1);
    gPad->SetLeftMargin(0.15);
    htime_dist_table[i]->Draw("colz");
    htime_dist_table[i]->GetYaxis()->SetTitleOffset(1.2);
    
    c10->cd(i+1);
    gPad->SetLeftMargin(0.15);
    hreal_dist_table[i]->Draw("colz");
    hreal_dist_table[i]->GetYaxis()->SetTitleOffset(1.2);
    Lin_cor->Draw("same");

    leg_Dist[i] = new TLegend(.2,.65,.57,.9,"Key");
    leg_Dist[i]->SetFillColor(0);
    leg_Dist[i]->SetTextSize(0.025);
    leg_Dist[i]->AddEntry(Lin_cor,"Exact linear correlation","l");
    leg_Dist[i]->Draw("same");
    
    
  }
  

  // save results (values of R and theta0 for each plane) to DB

  std::ofstream* outp[NPLANE]; 


  for(Int_t i = 0; i<NPLANE; i++){

    outp[i] = new std::ofstream;
    
    outp[i]->open(Form("DB/lookup_tables/db_%s_%s_lookup_TTD_angleCorr.vdc.%d.dat", arm, plane[i], runnumber) );

    *outp[i]<<arm<<".vdc."<<plane[i]<<".ttd_table.R = "<< endl;
    *outp[i]<<Pars[i][0]<<endl;
    *outp[i]<<arm<<".vdc."<<plane[i]<<".ttd_table.theta0 = "<<endl;
    *outp[i]<<Pars[i][1]<<endl;

    outp[i]->close();
  }



  // print results


  c6->Print(Form("plots/lookup_angle_corr/Dist_comp_%s_%i.png",arm,runnumber));
  c7->Print(Form("plots/lookup_angle_corr/Dist_Diff_Uncorrected_%s_%i.png",arm,runnumber));
  c8->Print(Form("plots/lookup_angle_corr/Dist_Diff_Corrected_%s_%i.png",arm,runnumber));

  c6->Print(Form("plots/lookup_angle_corr/Lookup_Corr_%s_%i.pdf(",arm,runnumber));
  c7->Print(Form("plots/lookup_angle_corr/Lookup_Corr_%s_%i.pdf",arm,runnumber));
  c8->Print(Form("plots/lookup_angle_corr/Lookup_Corr_%s_%i.pdf)",arm,runnumber));
  
}
