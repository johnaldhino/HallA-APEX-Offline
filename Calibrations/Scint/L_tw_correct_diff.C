/*
*************************************************************
7/1/21 John Williamson

Performs timewalk corrections for LHRS.

Uses common Hall A technique for timewalk calibration as stated in Hall A analyzer which cites:
  // Traditional correction according to
  // J.S.Brown et al., NIM A221, 503 (1984); T.Dreyer et al., NIM A309, 184 (1991)
  // Assume that for a MIP (peak ~2000 ADC channels) the correction is 0


This correction involves two coeffecients: one for a 'Minimising Ionising Particle' (MIP) whose timewalk offset is assumed to be zero, this is arbitary but is set at reasonable (and potentially different) value for S0 and S2. The second coeffecients is a mutliplicative factor and this is what must be calibrated.
*************************************************************

This script uses difference between left and right paddle TW effect to characterise and correct TW effect

 */

#include "file_def.h"
#include "Load_more_rootfiles.C"


TF1* f_cb;
TF1* f_line;


void L_tw_correct_diff(Int_t runno, Bool_t Coinc = true){

  TChain* T = Load_more_rootfiles(runno);

  // set levels of MIP for s0 and s2 (this must match definition in DB)

  // const Double_t s0_MIP = 500.0;
  // const Double_t s2_MIP = 200.0;


  const Int_t MaxS2Hit = 15;
  const Int_t NS2Pad = 16;

  Double_t s0_L = 0.0;
  Double_t s0_R = 0.0;
  
  Double_t s2_L[NS2Pad] = {0.0};
  Double_t s2_R[NS2Pad] = {0.0};


  // set-up branches to be read from TTree


    //Define Variables
  Double_t L_tr_n,L_cer_asum_c,L_ps_e,L_sh_e;
  Double_t L_tr_p[100],L_s0_trx[100],L_s2_try[100];
  Double_t L_s0_lt[10],L_s0_rt[10];
  Double_t L_s0_la_p[10],L_s0_ra_p[10];
  Double_t L_s0_nthit;
  Double_t L_s2_lt[16],L_s2_rt[16];
  Double_t L_s2_la_p[16],L_s2_ra_p[16];
  Double_t L_s2_la_c[16],L_s2_ra_c[16];
  Double_t L_s2_nthit;
  Double_t L_s2_t_pads[16];
  Double_t L_s2_trx[100];
  Double_t L_s2_trdx[100];
  Double_t evtypebits;
  Double_t L_s0_trpath[100],L_s2_trpath[100];

  
  //trigger
  Double_t Trig_type;

  Double_t Trig_time_T2[MaxS2Hit];
  Double_t Trig_No_T2;
  Double_t Trig_time_T5[MaxS2Hit];
  Double_t Trig_No_T5;

  
  //Define Branch Status/Addresses
  T->SetBranchStatus("*",0);
  T->SetBranchStatus("L.tr.n",1);
  T->SetBranchStatus("L.tr.p",1);
  T->SetBranchStatus("L.cer.asum_c",1);
  T->SetBranchStatus("L.prl1.e",1);
  T->SetBranchStatus("L.prl2.e",1);
  T->SetBranchStatus("L.s0.lt",1);
  T->SetBranchStatus("L.s0.rt",1);
  T->SetBranchStatus("L.s0.trx",1);  
  T->SetBranchStatus("L.s0.nthit",1);
  T->SetBranchStatus("L.s0.la_p",1);
  T->SetBranchStatus("L.s0.ra_p",1);
  T->SetBranchStatus("L.s2.lt",1);
  T->SetBranchStatus("L.s2.rt",1);
  T->SetBranchStatus("L.s2.la_p",1);
  T->SetBranchStatus("L.s2.ra_p",1);
  T->SetBranchStatus("L.s2.la_c",1);
  T->SetBranchStatus("L.s2.ra_c",1);
  T->SetBranchStatus("L.s2.try",1);
  T->SetBranchStatus("L.s2.trx",1);
  T->SetBranchStatus("L.s2.trdx",1);
  T->SetBranchStatus("L.s2.nthit",1);
  T->SetBranchStatus("L.s2.t_pads",1);
  T->SetBranchStatus("DL.evtypebits",1);
  T->SetBranchStatus("L.s0.trpath",1);
  T->SetBranchStatus("L.s2.trpath",1);
  T->SetBranchStatus("DL.evtype",1);
  T->SetBranchStatus("DR.rrawt2",1); 
  T->SetBranchStatus("Ndata.DR.rrawt2",1);
  T->SetBranchStatus("DR.rrawt5",1);
  T->SetBranchStatus("Ndata.DR.rrawt5",1);
  
  T->SetBranchAddress("L.tr.n",&L_tr_n);
  T->SetBranchAddress("L.tr.p",L_tr_p);
  T->SetBranchAddress("L.cer.asum_c",&L_cer_asum_c);
  T->SetBranchAddress("L.prl1.e",&L_ps_e);
  T->SetBranchAddress("L.prl2.e",&L_sh_e);
  T->SetBranchAddress("L.s0.lt",L_s0_lt);
  T->SetBranchAddress("L.s0.rt",L_s0_rt);
  T->SetBranchAddress("L.s0.trx",L_s0_trx);
  T->SetBranchAddress("L.s0.la_p",L_s0_la_p);
  T->SetBranchAddress("L.s0.ra_p",L_s0_ra_p);
  T->SetBranchAddress("L.s0.nthit",&L_s0_nthit);
  T->SetBranchAddress("L.s2.lt",L_s2_lt);
  T->SetBranchAddress("L.s2.rt",L_s2_rt);
  T->SetBranchAddress("L.s2.la_p",L_s2_la_p);
  T->SetBranchAddress("L.s2.ra_p",L_s2_ra_p);
  T->SetBranchAddress("L.s2.la_c",L_s2_la_c);
  T->SetBranchAddress("L.s2.ra_c",L_s2_ra_c);
  T->SetBranchAddress("L.s2.try",L_s2_try);
  T->SetBranchAddress("L.s2.trx",L_s2_trx);
  T->SetBranchAddress("L.s2.trdx",L_s2_trdx);
  T->SetBranchAddress("L.s2.nthit",&L_s2_nthit);
  T->SetBranchAddress("L.s2.t_pads",L_s2_t_pads);
  T->SetBranchAddress("DL.evtypebits",&evtypebits);
  T->SetBranchAddress("L.s0.trpath",L_s0_trpath);
  T->SetBranchAddress("L.s2.trpath",L_s2_trpath);
  T->SetBranchAddress("DL.evtype",&Trig_type);
  T->SetBranchAddress("DR.rrawt2", Trig_time_T2); 
  T->SetBranchAddress("Ndata.DR.rrawt2",&Trig_No_T2);
  T->SetBranchAddress("DR.rrawt5",Trig_time_T5);
  T->SetBranchAddress("Ndata.DR.rrawt5",&Trig_No_T5);

  Int_t nentries = T->GetEntries();
  //  nentries = 1e4;

  cout<<"Total Number of Events = "<<nentries<<endl;


  // set up histograms
  
  // s0


  Double_t diff_l = -0.05;
  Double_t diff_h = 0.05;
  
  TH1F* h_L_s0_t = new TH1F("h_L_s0_t","s0 left + right time",70,5.38e3,5.45e3);

  TH1F* h_L_s0_TW = new TH1F("h_L_s0_TW","s0 left + right TW effect",300,diff_l,diff_h);

  TH2F* h_L_s0_t_TW = new TH2F("h_L_s0_t_TW","s0 left + right time vs TW effect",50,diff_l,diff_h,60,-35,35);

  TProfile* hprof_L_s0_t_TW = new TProfile("hprof_L_s0_t_TW","s0 left + right time vs TW effect",50,diff_l,diff_h,-35,35);


  // S2

  
  TCut LCut = "L.tr.n==1 && abs(L.s2.trdx)<0.07 && (L.prl1.e/(L.gold.p*1000))>0.3 && ((L.prl1.e+L.prl2.e)/(L.gold.p*1000))>0.625 && ((L.prl1.e+L.prl2.e)/(L.gold.p*1000))<1.11 &&  L.cer.asum_c >650";
    
  TCut L_s2_Cut[NS2Pad] ;
  TCut L_s2_lCut[NS2Pad] ;
  TCut L_s2_rCut[NS2Pad] ;

  
  TH1F* h_L_s2_t[NS2Pad];
  TH1F* h_L_s2_ADC[NS2Pad];
  TH1F* hcorr_L_s2_ADC[NS2Pad];
  TF1* f_cb_ADC[NS2Pad];
  TF1* f_line_ADC[NS2Pad];
  TF1* s2_ADC_intersection[NS2Pad];
  TLine* s2_ADC_l_cut[NS2Pad];
  TLine* s2_ADC_h_cut[NS2Pad];
  TLine* s2_ADC_l_cut_t[NS2Pad];
  TLine* s2_ADC_h_cut_t[NS2Pad];
  Double_t s2_ADC_l_val[NS2Pad];
  Double_t s2_ADC_h_val[NS2Pad];
  Double_t s2_offset[NS2Pad];
  Double_t s2_k[NS2Pad];
  Double_t s2_MIP[NS2Pad];
  TH2F* h_L_s2_t_ADC[NS2Pad]; 
  TProfile* hprof_L_s2_t_ADC[NS2Pad];
  TProfile* hprof_L_s2_t_ADC_cut[NS2Pad];
  TH2F* hcorr_L_s2_t_ADC[NS2Pad]; 
  TProfile* hcorrprof_L_s2_t_ADC[NS2Pad];
  TH2F* hcorr2_L_s2_t_ADC[NS2Pad]; 
  TProfile* hcorr2prof_L_s2_t_ADC[NS2Pad];
  TH2F* h_L_s2_t_ADCl[NS2Pad]; 
  TProfile* hprof_L_s2_t_ADCl[NS2Pad];
  TH2F* h_L_s2_t_ADCr[NS2Pad]; 
  TProfile* hprof_L_s2_t_ADCr[NS2Pad];
  

  TH1F* h_L_s2_t_l[NS2Pad];
  TH1F* hcorr_L_s2_t_l[NS2Pad];
  TH1F* hcorr2_L_s2_t_l[NS2Pad];
  TH1F* h_L_s2_ADC_l[NS2Pad];
  TH1F* h_L_s2_ADC_l_check[NS2Pad];
  TF1* f_cb_ADC_l[NS2Pad];
  TF1* f_line_ADC_l[NS2Pad];
  TF1* s2_l_ADC_intersection[NS2Pad];
  TLine* s2_l_ADC_l_cut[NS2Pad];
  TLine* s2_l_ADC_h_cut[NS2Pad];
  TLine* s2_l_ADC_l_cut_t[NS2Pad];
  TLine* s2_l_ADC_h_cut_t[NS2Pad];
  Double_t s2_l_ADC_l_val[NS2Pad];
  Double_t s2_l_ADC_h_val[NS2Pad];
  Double_t s2_offset_l[NS2Pad];
  Double_t s2_k_l[NS2Pad];
  Double_t s2_MIP_l[NS2Pad];
  TH2F* h_L_s2_tl_ADC[NS2Pad];
  TProfile* hprof_L_s2_tl_ADC[NS2Pad];
  TH2F* h_L_s2_tl_ADCl[NS2Pad];
  TProfile* hprof_L_s2_tl_ADCl[NS2Pad];
  TH2F* h_L_s2_tl_ADCr[NS2Pad];
  TProfile* hprof_L_s2_tl_ADCr[NS2Pad];
  Double_t tw_stop[NS2Pad];

  
  
  TH1F* h_L_s2_t_r[NS2Pad];
  TH1F* h_L_s2_ADC_r[NS2Pad];
  TF1* f_cb_ADC_r[NS2Pad];
  // TH2F* h_L_s2_t_ADC_r[NS2Pad];
  // TProfile* hprof_L_s2_t_ADC_r[NS2Pad];
  TH2F* h_L_s2_tr_ADC[NS2Pad];
  TProfile* hprof_L_s2_tr_ADC[NS2Pad];
  TH2F* h_L_s2_tr_ADCl[NS2Pad];
  TProfile* hprof_L_s2_tr_ADCl[NS2Pad];
  TH2F* h_L_s2_tr_ADCr[NS2Pad];
  TProfile* hprof_L_s2_tr_ADCr[NS2Pad];
  

  TCanvas* c3[NS2Pad];
  TCanvas* c4[NS2Pad];

  Double_t s2_mean[NS2Pad];
  Double_t s2_mean_l[NS2Pad];
  Double_t s2_mean_r[NS2Pad];

  Double_t s2_tw_mean[NS2Pad];
  Double_t s2_tw_sigma[NS2Pad];
    
  Double_t s2_tw_l[NS2Pad];
  Double_t s2_tw_h[NS2Pad];
  
  Double_t s2_tw_slope[NS2Pad];
  Double_t s2_tw_off[NS2Pad];
  Double_t s2_tw_off_sl[NS2Pad];
  

  Double_t s2_tw_p0[NS2Pad];
  Double_t s2_tw_p1[NS2Pad];
  Double_t s2_tw_p2[NS2Pad];
  Double_t s2_tw_p3[NS2Pad];

  TF1* s2_tw_line[NS2Pad];
  TF1* s2_tw_pol[NS2Pad];

  // variable limits
  
  const Double_t fTdc2T = 0.5e-9;

  
  // time limits (difference from mean)
  Double_t tdiff_l = -10;
  Double_t tdiff_h = 10;
  Int_t tdiff_nbins = (tdiff_h - tdiff_l);
  
  // time limits (no correction) 
  Double_t time_l = 5.5e3; // left + right pmts
  Double_t time_h = 5.56e3; // left + right pmts
  Int_t time_nbins = (time_h - time_l);
  
  // left time 
  Double_t timel_l = 2.80e3; // left pmts
  Double_t timel_h = 2.86e3; // left pmts
  Int_t timel_nbins = (timel_h - timel_l);

  // right time 
  Double_t timer_l = 2.74e3; // right pmts
  Double_t timer_h = 2.80e3; // right pmts
  Int_t timer_nbins = (timer_h - timer_l);

  
  
  // single pmt TW lims
  Double_t tw_s_low = 0.0;
  Double_t tw_s_high = 0.08;
  Int_t tw_s_nbins = 50;

  Double_t tw_l_low = tw_s_low;
  Double_t tw_l_high = tw_s_high;
  Int_t tw_l_nbins = tw_s_nbins;
		     
  Double_t tw_r_low = tw_s_low;
  Double_t tw_r_high = tw_s_high;
  Int_t tw_r_nbins = tw_s_nbins;

  
  // both arm pmt TW lims
  Double_t tw_b_low = -0.05;
  Double_t tw_b_high = 0.05;
  Int_t tw_b_nbins = 50;


  // parameters for S2 and trigger timing cuts

  Double_t T2LowCut = 1683;
  Double_t T2HighCut = 1710;

  Double_t T5LowCut = 1625;
  Double_t T5HighCut = 1634;

  Bool_t T2Pass = false;
  Bool_t T5Pass = false;
		     		     
  Bool_t CoincPass = false;

  for(Int_t i = 0; i<NS2Pad; i++){



    // define cuts
    L_s2_Cut[i] = Form("L.s2.nthit==1 && L.s2.t_pads==%i && L.s2.la_c[%i]>100 && L.s2.ra_c[%i]>100",i,i,i);

    L_s2_lCut[i] = Form("L.s2.nthit==1 && L.s2.t_pads==%i && L.s2.la_c[%i]>100",i,i);

    L_s2_rCut[i] = Form("L.s2.nthit==1 && L.s2.t_pads==%i && L.s2.ra_c[%i]>100",i,i);
    

    h_L_s2_t[i] = new TH1F(Form("h_L_s2_t_%i",i),Form("s2 left + right time, %i",i),time_nbins,time_l,time_h);

    h_L_s2_t_l[i] = new TH1F(Form("h_L_s2_t_l_%i",i),Form("s2 left time, %i",i),timel_nbins,timel_l,timel_h);
    
    hcorr_L_s2_t_l[i] = new TH1F(Form("hcorr_L_s2_t_l_%i",i),Form("(lin corr) s2 left time, %i",i),timel_nbins,timel_l,timel_h);

    hcorr2_L_s2_t_l[i] = new TH1F(Form("hcorr2_L_s2_t_l_%i",i),Form("(pol3 corr) s2 left time, %i",i),timel_nbins,timel_l,timel_h);
 
    h_L_s2_t_r[i] = new TH1F(Form("h_L_s2_t_r_%i",i),Form("s2 right time, %i",i),timer_nbins,timer_l,timer_h);


    h_L_s2_ADC[i] = new TH1F(Form("h_L_s2_ADC_%i",i),Form("s2 left - right TW effect, %i",i),tw_b_nbins,tw_b_low,tw_b_high);

    hcorr_L_s2_ADC[i] = new TH1F(Form("hcorr_L_s2_ADC_%i",i),Form("s2 left - right TW effect, %i",i),tw_b_nbins,tw_b_low,tw_b_high);

    

    h_L_s2_t_ADC[i] = new TH2F(Form("h_L_s2_t_ADC_%i",i),Form("s2 left time vs (L-R) TW effect, %i",i),tw_b_nbins,tw_b_low,tw_b_high,tdiff_nbins,tdiff_l,tdiff_h);

    hprof_L_s2_t_ADC[i] = new TProfile(Form("hprof_L_s2_t_ADC_%i",i),Form("s2 left time vs (L-R) TW effect, %i",i),tw_b_nbins,tw_b_low,tw_b_high,tdiff_l,tdiff_h);


    
    hcorr_L_s2_t_ADC[i] = new TH2F(Form("hcorr_L_s2_t_ADC_%i",i),Form("(lincorr) s2 left time vs (L-R) TW effect, %i",i),tw_b_nbins,tw_b_low,tw_b_high,tdiff_nbins,tdiff_l,tdiff_h);

    hcorrprof_L_s2_t_ADC[i] = new TProfile(Form("hcorrprof_L_s2_t_ADC_%i",i),Form("(lin corr) s2 left time vs (L-R) TW effect, %i",i),tw_b_nbins,tw_b_low,tw_b_high,tdiff_l,tdiff_h);

    hcorr2_L_s2_t_ADC[i] = new TH2F(Form("hcorr2_L_s2_t_ADC_%i",i),Form("(pol3 corr) s2 left time vs (L-R) TW effect, %i",i),tw_b_nbins,tw_b_low,tw_b_high,tdiff_nbins,tdiff_l,tdiff_h);

    hcorr2prof_L_s2_t_ADC[i] = new TProfile(Form("hcorr2prof_L_s2_t_ADC_%i",i),Form("(pol3 corr) s2 left time vs (L-R) TW effect, %i",i),tw_b_nbins,tw_b_low,tw_b_high,tdiff_l,tdiff_h);


  
  }
  

  
  for(Int_t i=0;i<nentries;i++){

    T2Pass = false;
    T5Pass = false;
    CoincPass = true;
    
    if(i%100000==0) cout << " events processed = " << i << endl;
    T->GetEntry(i);


    for(Int_t j = 0; j < Trig_No_T2; j++){      
      if (Trig_time_T2[j] > T2LowCut && Trig_time_T2[j] < T2HighCut){	
	T2Pass = true;
      }
    }

    
    for(Int_t j = 0; j < Trig_No_T5; j++){      
      if (Trig_time_T5[j] > T5LowCut && Trig_time_T5[j] < T5HighCut){	
	T5Pass = true;
      }
    }

    if((T2Pass && T5Pass && Trig_type == 6) || !Coinc){
      CoincPass = true;
    }
    
    
    if(L_tr_n==1 && L_cer_asum_c>1500 && (L_ps_e+L_sh_e)/(1000.*L_tr_p[0])>0.8 && L_s0_nthit==1 && CoincPass){
      h_L_s0_t->Fill(L_s0_lt[0] + L_s0_rt[0]);
      h_L_s0_TW->Fill((1/TMath::Sqrt(L_s0_la_p[0])) - (1/TMath::Sqrt(L_s0_ra_p[0])));
      //      h_L_s0_t_ADC->Fill();

    }
      
    for(Int_t j=0;j<16;j++){
      if(L_tr_n==1 && L_cer_asum_c>2500&&L_s2_t_pads[0]==j&&L_s2_nthit==1&&(L_ps_e+L_sh_e)/L_tr_p[0]/1000>0.7 && L_s2_lt[j]>0 && L_s2_rt[j]>0 && abs(L_s2_trdx[0])<0.07 && ((L_s2_la_c[j]>160 && L_s2_ra_c[j]>160) || ( (j==4 || j==5) && (L_s2_la_c[j]>160 || L_s2_ra_c[j]>160))) && CoincPass){
	// abs(L.s2.trdx)<0.07" 

	h_L_s2_t[j]->Fill(L_s2_lt[j] + L_s2_rt[j]);
	h_L_s2_ADC[j]->Fill((1/TMath::Sqrt(L_s2_la_c[j])) - (1/TMath::Sqrt(L_s2_ra_c[j])));

	  
	h_L_s2_t_l[j]->Fill(L_s2_lt[j]);
	//	  h_L_s2_ADC_l[j]->Fill((1/TMath::Sqrt(L_s2_la_p[j])));


	h_L_s2_t_r[j]->Fill(L_s2_rt[j]);
	//	  h_L_s2_ADC_r[j]->Fill((1/TMath::Sqrt(L_s2_ra_p[j])));


      }

    }

  }
  

  TCanvas *c1 = new TCanvas("c1","s0 timing");
  c1->Divide(2,1);
  c1->cd(1);


  
  h_L_s0_t->Draw();
  
  
  Double_t s0_t_mean = h_L_s0_t->GetMean();

  cout << "s0_t_mean = " << s0_t_mean << endl;

  c1->cd(2); 
  h_L_s0_TW->Draw();

  h_L_s0_TW->Fit("gaus","QR","",diff_l,diff_h);

  // limits for fit of difference

  
  Double_t s0_tw_mean = 0.0;
  Double_t s0_tw_sigma = 0.0;

  Double_t s0_tw_l = 0.0;
  Double_t s0_tw_h = 0.0;

 
  // some runs do not have LHRS S0 on (so retrieving fit would fail)
  if(h_L_s0_TW->GetFunction("gaus")){
  
    s0_tw_mean = h_L_s0_TW->GetFunction("gaus")->GetParameter(1);
    s0_tw_sigma = h_L_s0_TW->GetFunction("gaus")->GetParameter(2);
    
    s0_tw_l = s0_tw_mean - 3*s0_tw_sigma;
    s0_tw_h = s0_tw_mean + 3*s0_tw_sigma;
  }
    
  
  for(Int_t i = 0; i<NS2Pad; i++){

    // define cut

    c3[i] = new TCanvas(Form("c3_%i",i),Form("S2 timing, %i",i));
    c3[i]->Divide(4,1);

    c3[i]->cd(2);

    h_L_s2_t_l[i]->Draw();
    
    s2_mean_l[i] = h_L_s2_t_l[i]->GetMean();

    
    c3[i]->cd(1);

    h_L_s2_ADC[i]->Draw();
    h_L_s2_ADC[i]->Fit("gaus","QR","",diff_l,diff_h);


    if( h_L_s2_ADC[i]->GetFunction("gaus")){
      s2_tw_mean[i] = h_L_s2_ADC[i]->GetFunction("gaus")->GetParameter(1);
      s2_tw_sigma[i] = h_L_s2_ADC[i]->GetFunction("gaus")->GetParameter(2);
    }
    else{
      s2_tw_mean[i] = 0.0;
      s2_tw_sigma[i] = 0.0;
    }
    
    s2_tw_l[i] = s2_tw_mean[i] - 4*s2_tw_sigma[i];
    s2_tw_h[i] = s2_tw_mean[i] + 4*s2_tw_sigma[i];
    
    
  }




  for(Int_t i=0;i<nentries;i++){
    
    if(i%100000==0) cout << " events processed = " << i << endl;
    T->GetEntry(i);
    
    if(L_tr_n==1 && L_cer_asum_c>1500 && (L_ps_e+L_sh_e)/(1000.*L_tr_p[0])>0.8 && L_s0_nthit==1){
      
      h_L_s0_t_TW->Fill((1/TMath::Sqrt(L_s0_la_p[0])) - (1/TMath::Sqrt(L_s0_ra_p[0])), L_s0_lt[0] + L_s0_rt[0]-s0_t_mean);
      hprof_L_s0_t_TW->Fill((1/TMath::Sqrt(L_s0_la_p[0])) - (1/TMath::Sqrt(L_s0_ra_p[0])), L_s0_lt[0] + L_s0_rt[0] - s0_t_mean);

    }
      
    for(Int_t j=0;j<16;j++){
      if(L_tr_n==1 && L_cer_asum_c>2500&&L_s2_t_pads[0]==j&&L_s2_nthit==1&&(L_ps_e+L_sh_e)/L_tr_p[0]/1000>0.7 && L_s2_lt[j]>0 && L_s2_rt[j]>0 && abs(L_s2_trdx[0])<0.07 && L_s2_la_c[j]>160 && L_s2_ra_c[j]>160 ){


	// Double_t TW_l = 1/TMath::Sqrt(L_s2_la_p[j]);
	// Double_t TW_r = 1/TMath::Sqrt(L_s2_ra_p[j]);
	Double_t TW_l = 1/TMath::Sqrt(L_s2_la_c[j]);
	Double_t TW_r = 1/TMath::Sqrt(L_s2_ra_c[j]);
	Double_t TW_b = TW_l - TW_r;
	  

	Double_t b_time =  L_s2_lt[j] + L_s2_rt[j] - s2_mean[j];
	Double_t l_time =  L_s2_lt[j] - s2_mean_l[j];
	Double_t r_time =  L_s2_rt[j] - s2_mean_r[j];
	  

	// cout << "TW_b = " << TW_b << endl;
	// cout << "l_time = " << l_time << endl << endl;
	h_L_s2_t_ADC[j]->Fill(TW_b,l_time);
	hprof_L_s2_t_ADC[j]->Fill(TW_b,l_time);	  	  	 	  

      }

    }

  }
  

  
  TCanvas *c2 = new TCanvas("c2","s0 timing vs TW effects");

  c2->Divide(2,1);
  c2->cd(1);
  
  h_L_s0_t_TW->Draw("colz");

  c2->cd(2);
  hprof_L_s0_t_TW->Draw();
  

  if(s0_tw_l != 0 && s0_tw_h !=0 && s0_tw_l != s0_tw_h){
    hprof_L_s0_t_TW->Fit("pol1","QR","",s0_tw_l,s0_tw_h);
  }
  
  
  
 
  
  Double_t s0_offset = 0;
  Double_t s0_k = 0;

  // can now derive ADC_MIP for s0 from these parameters

  Double_t s0_MIP = 0;

  cout << "s0 k = " << s0_k << ", s0 ADC_MIP = " << s0_MIP << endl;

  


    
  for(Int_t i = 0; i<NS2Pad; i++){

    // left + right time
    
    c4[i] = new TCanvas(Form("c4_%i",i),Form("s2 timing vs TW effects, %i",i));

    c4[i]->Divide(2,3);
    c4[i]->cd(1);
    
    h_L_s2_t_ADC[i]->Draw("colz");

      
    c4[i]->cd(2);
    

    s2_tw_line[i] = new TF1(Form("s2_tw_line_%i",i),"pol1",s2_tw_l[i],s2_tw_h[i]);
    s2_tw_pol[i] = new TF1(Form("s2_tw_pol_%i",i),"pol3",s2_tw_l[i],s2_tw_h[i]);

    
    hprof_L_s2_t_ADC[i]->Fit(Form("s2_tw_line_%i",i),"QR","",s2_tw_l[i],s2_tw_h[i]);


    if( s2_tw_line[i] ){

      s2_tw_off[i] = s2_tw_line[i]->GetParameter(0);     
      s2_tw_slope[i] = s2_tw_line[i]->GetParameter(1);

      if(s2_tw_off[i] != 0 && s2_tw_slope[i] != 0){
	s2_tw_off_sl[i] = s2_tw_off[i]/s2_tw_slope[i];
      }
      
      
      // s2_tw_pol[i]->SetParameter(0,s2_tw_off[i]);
      // s2_tw_pol[i]->SetParameter(1,s2_tw_slope[i]);

      hprof_L_s2_t_ADC_cut[i] = (TProfile*) hprof_L_s2_t_ADC[i]->Clone(Form("hprof_L_s2_t_ADC_%i",i));
      hprof_L_s2_t_ADC_cut[i]->Add(s2_tw_line[i],-1);
      hprof_L_s2_t_ADC_cut[i]->Fit(Form("s2_tw_pol_%i",i),"QR0","",s2_tw_l[i],s2_tw_h[i]);            

      s2_tw_pol[i]->SetParameter(0,s2_tw_off[i]);
      s2_tw_pol[i]->SetParameter(1,s2_tw_slope[i]);
      
      hprof_L_s2_t_ADC[i]->Fit(Form("s2_tw_pol_%i",i),"QR0","",s2_tw_l[i],s2_tw_h[i]);

      hprof_L_s2_t_ADC_cut[i] = (TProfile*) hprof_L_s2_t_ADC[i]->Clone(Form("hprof_L_s2_t_ADC_%i",i));
      hprof_L_s2_t_ADC_cut[i]->Add(s2_tw_line[i],-1);


      s2_tw_p0[i] = s2_tw_pol[i]->GetParameter(0);
      s2_tw_p1[i] = s2_tw_pol[i]->GetParameter(1);
      s2_tw_p2[i] = s2_tw_pol[i]->GetParameter(2);
      s2_tw_p3[i] = s2_tw_pol[i]->GetParameter(3);
      
      
    }
    else{
      s2_tw_off[i] = 0.0;
      s2_tw_slope[i] = 0.0;
    }


    s2_tw_line[i]->SetLineColor(kGreen+2);
    s2_tw_line[i]->Draw("same");

    s2_tw_pol[i]->SetLineColor(kRed);
    s2_tw_pol[i]->Draw("same");

      
  }



  for(Int_t i=0;i<nentries;i++){
    
    if(i%100000==0) cout << " events processed = " << i << endl;
    T->GetEntry(i);

    for(Int_t j=0;j<16;j++){
      if(L_tr_n==1 && L_cer_asum_c>2500&&L_s2_t_pads[0]==j&&L_s2_nthit==1&&(L_ps_e+L_sh_e)/L_tr_p[0]/1000>0.7 && L_s2_lt[j]>0 && L_s2_rt[j]>0 && abs(L_s2_trdx[0])<0.07 && ( (L_s2_la_c[j]>160 && L_s2_ra_c[j]>160) || ( (j==4 || j==5) && (L_s2_la_c[j]>160 || L_s2_ra_c[j]>160) ) ) ) {
	
	Double_t TW_l = 1/TMath::Sqrt(L_s2_la_c[j]);
	Double_t TW_r = 1/TMath::Sqrt(L_s2_ra_c[j]);
	Double_t TW_b = TW_l - TW_r;

	
	Double_t tw_corr = s2_tw_slope[j]*TW_b + s2_tw_off[j];
	//	Double_t tw_corr = s2_tw_slope[j]*(TW_b + s2_tw_off_sl[j]);
	Double_t l_time =  L_s2_lt[j] - s2_mean_l[j] - tw_corr;

	Double_t tw_polcorr = (TMath::Power(TW_b,3)*s2_tw_p3[j]) + (TMath::Power(TW_b,2)*s2_tw_p2[j]) + (TMath::Power(TW_b,1)*s2_tw_p1[j]) + s2_tw_p0[j];
	Double_t l_time_pol =  L_s2_lt[j] - s2_mean_l[j] - tw_polcorr;
	
	// cout << "Chan = " << j << endl;
	// cout << "slope = " << s2_tw_slope[j] << endl;
	// cout << "off = " << s2_tw_off[j] << endl;
	// cout << "tw_corr = " << tw_corr << endl << endl;
	
	Double_t l_time_uncor =  L_s2_lt[j] - tw_corr;
	Double_t l_time_uncor_pol =  L_s2_lt[j] - tw_polcorr;
	

	
	hcorr_L_s2_t_l[j]->Fill(l_time_uncor);

	hcorr_L_s2_t_ADC[j]->Fill(TW_b,l_time);
	hcorrprof_L_s2_t_ADC[j]->Fill(TW_b,l_time);

	hcorr2_L_s2_t_l[j]->Fill(l_time_uncor_pol);
	
	hcorr2_L_s2_t_ADC[j]->Fill(TW_b,l_time_pol);
	hcorr2prof_L_s2_t_ADC[j]->Fill(TW_b,l_time_pol);

	
      }
    }
  }


  
  for(Int_t i = 0; i<NS2Pad; i++){
    
    c3[i]->cd(3);
    
    hcorr_L_s2_t_l[i]->Draw();

    c3[i]->cd(4);

    hcorr2_L_s2_t_l[i]->Draw();

    c4[i]->cd(3);

    hcorr_L_s2_t_ADC[i]->Draw("colz");

    
    c4[i]->cd(4);
    hcorrprof_L_s2_t_ADC[i]->Draw();

    
    c4[i]->cd(5);

    hcorr2_L_s2_t_ADC[i]->Draw("colz");

    
    c4[i]->cd(6);
    hcorr2prof_L_s2_t_ADC[i]->Draw();
    

    // hprof_L_s2_t_ADC_cut[i]->Draw();

  }


  

  // save canvases to pdf
  
  c1->Print(Form("time_walk/plots/L_s0_tw_%i.pdf(",runno));
  c2->Print(Form("time_walk/plots/L_s0_tw_%i.pdf)",runno));

  
  for(int i=0;i<NS2Pad;i++){
    
    if(i==0){
      c3[i]->Print(Form("time_walk/plots/L_s2_tw_%i.pdf(",runno));
      c4[i]->Print(Form("time_walk/plots/L_s2_tw_%i.pdf",runno));
    }
    else if(i==NS2Pad-1){
      c3[i]->Print(Form("time_walk/plots/L_s2_tw_%i.pdf",runno));
      c4[i]->Print(Form("time_walk/plots/L_s2_tw_%i.pdf)",runno));
    }
    else{
      c3[i]->Print(Form("time_walk/plots/L_s2_tw_%i.pdf",runno));
      c4[i]->Print(Form("time_walk/plots/L_s2_tw_%i.pdf",runno));
    }
  }
 
  
  
  // Print all corrections coeffecients and print to DB

  
  
  // S2 csv outfile

  ofstream oofile_csv_1P(Form("time_walk/DB/LHRS_diff_tw_1P_%i_S2.csv",runno));

  oofile_csv_1P<<"L_P1,L_P0"<<endl;

  cout << "Printing 1P S2 parameters:" << endl;
  cout<<"L_P1,L_P0"<<endl;
 
  for (Int_t ii=0;ii<16;ii++)
    {
      oofile_csv_1P<<s2_tw_slope[ii]<<","<<s2_tw_off_sl[ii]<< endl;
      cout << s2_tw_slope[ii]<<","<<s2_tw_off_sl[ii] << endl;
    }

  cout << endl << endl;
  
  ofstream oofile_csv_3P(Form("time_walk/DB/LHRS_diff_tw_3P_%i_S2.csv",runno));
  
  oofile_csv_3P<<"L_P3,L_P2,L_P1,L_P0"<<endl;
  cout << "Printing 3P S2 parameters:" << endl;
  cout<<"L_P3,L_P2,L_P1,L_P0"<<endl;
  
  for (Int_t ii=0;ii<16;ii++)
    {
      oofile_csv_3P<<s2_tw_p3[ii]<<","<<s2_tw_p2[ii]<<","<<s2_tw_p1[ii]<<","<<s2_tw_p0[ii]<< endl;
      cout<<s2_tw_p3[ii]<<","<<s2_tw_p2[ii]<<","<<s2_tw_p1[ii]<<","<<s2_tw_p0[ii]<< endl; 
    }

  cout << endl << endl;

  // S0 csv outfile
  ofstream oofile_csv_s0(Form("time_walk/DB/LHRS_tw_%i_S0.csv",runno));
  
  oofile_csv_s0<<"L_tw_param,L_MIP"<<endl;

  cout << "S0 parameters" << endl;
  cout<<"L_tw_param, L_MIP"<<endl;
  
  oofile_csv_s0<<s0_k<<","<<s0_MIP<<endl;
  cout<<s0_k<<","<<s0_MIP<<endl;

  cout << endl << endl;

  
}

