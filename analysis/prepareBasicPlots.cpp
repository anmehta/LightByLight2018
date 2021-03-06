//  prepareAcoplanarityPlots.cpp
//
//  Created by Jeremi Niedziela on 31/07/2019.

#include "Helpers.hpp"
#include "EventProcessor.hpp"
#include "PhysObjectProcessor.hpp"
#include "ConfigManager.hpp"
#include "EventDisplay.hpp"
#include "Logger.hpp"

bool saveCalosFailingNEE = true;
bool saveTriphotonHists = true;
bool checkTriggers = false;

int nThreePhotonEvents = 0;

TGraphAsymmErrors *triggerScaleFactorsLowEta, *triggerScaleFactorsHighEta;
TTree *triggerScaleFactorsTree;
vector<float> SFweights;

vector<string> suffixes = {
  "all", "low_aco", "high_aco", "pass_qed", "pass_lbl",  "good_pass_qed", "good_pass_lbl",
  "good", "good_low_aco", "good_high_aco", "good_all", "two_photons", "good_two_photons"
};

vector<tuple<string, int, double, double>> histParams = {
  // title                   nBins min   max
  {"lbl_acoplanarity"       , 200 , 0   , 1.0   },
  {"lbl_photon_et"          , 100 , 0   , 100.0 },
  {"lbl_photon_eta"         , 6   ,-2.4 , 2.4   },
  {"lbl_photon_phi"         , 8   ,-4.0 , 4.0   },
  {"lbl_diphoton_mass"      , 80  , 0   , 200.0 },
  {"lbl_diphoton_rapidity"  , 6   ,-2.4 , 2.4   },
  {"lbl_diphoton_pt"        , 50  , 0   , 10.0  },
  {"lbl_triphoton_mass"     , 800 , 0   , 200.0 },
  {"lbl_triphoton_rapidity" , 24  ,-2.4 , 2.4   },
  {"lbl_triphoton_pt"       , 500 , 0   , 100.0 },
  {"lbl_cut_flow"           , 15  , 0   , 15    },
  {"lbl_nee_failing"        , 15  , 0   , 15    },
  {"lbl_HFp"                , 200 , 0   , 20    },
  {"lbl_HFm"                , 200 , 0   , 20    },
  {"lbl_EB_leading_tower"   , 200 , 0   , 20    },
  {"lbl_EE_leading_tower"   , 200 , 0   , 20    },
  {"lbl_HB_leading_tower"   , 200 , 0   , 20    },
  {"lbl_HE_leading_tower"   , 200 , 0   , 20    },
  {"lbl_HFp_leading_tower"  , 200 , 0   , 20    },
  {"lbl_HFm_leading_tower"  , 200 , 0   , 20    },
  {"lbl_n_all_photons"      , 100 , 0   , 100   },
  {"lbl_n_all_calo_towers"  , 100 , 0   , 100   },
  {"lbl_n_all_L1EG"         , 100 , 0   , 100   },
  {"lbl_zdc_energy"         , 1000, 0   , 50000 },
  {"lbl_n_zdc"              , 100 , 0   , 100   },
  {"lbl_bad_photon_et"      , 100 , 0   , 100.0 },
  {"lbl_bad_photon_eta"     , 6   ,-2.4 , 2.4   },
  {"lbl_bad_photon_phi"     , 8   ,-4.0 , 4.0   },
  {"lbl_n_pixel_tracks"     , 100 , 0   , 100   },
  {"lbl_zdc_sum_energy"     , 20000, 0   , 1000000 },
  {"lbl_zdc_sum_energy_pos" , 20000, 0   , 1000000 },
  {"lbl_zdc_sum_energy_neg" , 20000, 0   , 1000000 },
  
  {"qed_acoplanarity"       , 500 , 0   , 1.0   },
  {"qed_electron_pt"        , 400 , 0   , 200.0 },
  {"qed_electron_eta"       , 24  ,-2.4 , 2.4   },
  {"qed_electron_phi"       , 20  ,-4.0 , 4.0   },
  {"qed_dielectron_mass"    , 200 , 0   , 200.0 },
  {"qed_dielectron_rapidity", 30  ,-3.0 , 3.0   },
  {"qed_dielectron_pt"      , 100 , 0   , 10.0  },
  {"qed_cut_flow"           , 15  , 0   , 15    },
  {"qed_electron_cutflow"   , 15  , 0   , 15    },
  {"qed_HFp"                , 200 , 0   , 20    },
  {"qed_HFm"                , 200 , 0   , 20    },
  {"qed_EB_leading_tower"   , 200 , 0   , 20    },
  {"qed_EE_leading_tower"   , 200 , 0   , 20    },
  {"qed_HB_leading_tower"   , 200 , 0   , 20    },
  {"qed_HE_leading_tower"   , 200 , 0   , 20    },
  {"qed_HFp_leading_tower"  , 200 , 0   , 20    },
  {"qed_HFm_leading_tower"  , 200 , 0   , 20    },
  
  {"samesign_dielectron_mass"       , 200 , 0   , 200.0 },
  {"samesign_dielectron_rapidity"   , 30  ,-3.0 , 3.0   },
  {"samesign_dielectron_pt"         , 100 , 0   , 10.0  },
  
  {"nTracks"                , 100 , 0   , 100   },
  {"track_pt"               , 5000, 0   , 100   },
  {"track_eta"              , 100 ,-3.5 , 3.5   },
  {"track_phi"              , 100 ,-3.5 , 3.5   },
  {"track_missing_hits"     , 50  , 0   , 50    },
  {"track_valid_hits"       , 50  , 0   , 50    },
  {"track_purity"           , 10  , 0   , 10    },
  {"track_charge"           , 4   ,-2   , 2     },
  {"track_chi2"             , 1000, 0   , 100   },
  {"track_dxy"              ,300000,-15  , 15    },
  {"track_dz"               ,100000,-50  , 50    },
  {"track_dxy_over_sigma"   , 1000, 0   , 100   },
  {"track_dz_over_sigma"    , 1000, 0   , 100   },
  
  {"track_dxy_from_bs"      ,300000,-15  , 15    },
  {"track_dz_from_bs"       ,100000,-50  , 50    },
  
  {"track_dxy_1_track"      ,300000,-15  , 15    },
  {"track_dxy_2_track"      ,300000,-15  , 15    },
  {"track_dxy_3_track"      ,300000,-15  , 15    },
  {"track_dxy_ge4_track"    ,300000,-15  , 15    },
  
  {"track_dz_1_track"       ,100000,-50  , 50    },
  {"track_dz_2_track"       ,100000,-50  , 50    },
  {"track_dz_3_track"       ,100000,-50  , 50    },
  {"track_dz_ge4_track"     ,100000,-50  , 50    },

  {"track_vx"               ,300000,-15  , 15    },
  {"track_vy"               ,300000,-15  , 15    },
  {"track_vz"               ,300000,-50  , 50    },
  
  {"tracks_cut_flow"        , 15  , 0   , 15    },
  
  {"nDisplacedTracks"       , 2000, 0   , 2000  },
  {"nDedxHits"              , 2000, 0   , 2000  },
  {"nPixelClusters"         , 2000, 0   , 2000  },
  {"nPixelRecHits"          , 2000, 0   , 2000  },
  
  {"zdc_sum_energy_pos"     , 20000, 0   , 1000000 },
  {"zdc_sum_energy_neg"     , 20000, 0   , 1000000 },
  
};

void fillTriphotonHists(Event &event, const map<string, TH1D*> &hists, string datasetName, bool saveEventDisplays=false)
{
  auto photons = event.GetPhysObjects(EPhysObjType::kGoodPhoton);
  PhysObjects isolatedPhotons;
  
  for(int i=0; i<photons.size(); i++){
    bool hasNerbyPhotons = false;
    for(int j=0; j<photons.size(); j++){
      if(i==j) continue;
      
      double deltaR = physObjectProcessor.GetDeltaR(*photons[i], *photons[j]);
      
      if(deltaR < 0.3){
        hasNerbyPhotons = true;
        break;
      }
    }
    if(!hasNerbyPhotons) isolatedPhotons.push_back(photons[i]);
  }
  
  if(isolatedPhotons.size() == 3){
    nThreePhotonEvents++;
    TLorentzVector triphoton = physObjectProcessor.GetTriphoton(*isolatedPhotons[0],
                                                                *isolatedPhotons[1],
                                                                *isolatedPhotons[2]);
    
    hists.at("lbl_triphoton_mass_all_"+datasetName)->Fill(triphoton.M());
    hists.at("lbl_triphoton_rapidity_all_"+datasetName)->Fill(triphoton.Rapidity());
    hists.at("lbl_triphoton_pt_all_"+datasetName)->Fill(triphoton.Pt());
    
    if(saveEventDisplays){
      PhysObjects emptyVector;
      saveEventDisplay(emptyVector, emptyVector, emptyVector, isolatedPhotons, emptyVector,
                       "~/Desktop/lbl_triphoton_event_displays_"+datasetName+"/");
    }
  }
}

void fillNobjectsHists(Event &event, const map<string, TH1D*> &hists, string datasetName, string suffix="")
{
  if(suffix != "") suffix += "_";
  
  hists.at("lbl_n_all_photons_"+suffix+datasetName)->Fill(event.GetPhysObjects(EPhysObjType::kPhoton).size());
  hists.at("lbl_n_all_calo_towers_"+suffix+datasetName)->Fill(event.GetPhysObjects(EPhysObjType::kCaloTower).size());
  hists.at("lbl_n_all_L1EG_"+suffix+datasetName)->Fill(event.GetPhysObjects(EPhysObjType::kL1EG).size());
  hists.at("lbl_n_pixel_tracks_"+suffix+datasetName)->Fill(event.GetPhysObjects(EPhysObjType::kPixelTrack).size());
  hists.at("lbl_n_zdc_"+suffix+datasetName)->Fill(event.GetPhysObjects(EPhysObjType::kZDC).size());
}

void fillPhotonHists(Event &event, const map<string, TH1D*> &hists, string datasetName, string suffix="")
{
  if(suffix != "") suffix += "_";
  for(auto photon : event.GetPhysObjects(EPhysObjType::kGoodPhoton)){
    hists.at("lbl_photon_et_" +suffix+datasetName)->Fill(photon->GetEt());
    hists.at("lbl_photon_eta_"+suffix+datasetName)->Fill(photon->GetEta());
    hists.at("lbl_photon_phi_"+suffix+datasetName)->Fill(photon->GetPhi());
  }
  double zdcEnergySum = 0;
  double zdcEnergySumPos = 0;
  double zdcEnergySumNeg = 0;
  
  for(auto zdc : event.GetPhysObjects(EPhysObjType::kZDC)){
    hists.at("lbl_zdc_energy_"+suffix+datasetName)->Fill(zdc->GetEnergy());
    zdcEnergySum += zdc->GetEnergy();
    
    if(zdc->GetZside() > 0) zdcEnergySumPos += zdc->GetEnergy();
    else                    zdcEnergySumNeg += zdc->GetEnergy();
  }
  hists.at("lbl_zdc_sum_energy_"+suffix+datasetName)->Fill(zdcEnergySum);
  hists.at("lbl_zdc_sum_energy_pos_"+suffix+datasetName)->Fill(zdcEnergySumPos);
  hists.at("lbl_zdc_sum_energy_neg_"+suffix+datasetName)->Fill(zdcEnergySumNeg);
  
  auto goodPhotons = event.GetPhysObjects(EPhysObjType::kGoodPhoton);
  
  for(auto photon : event.GetPhysObjects(EPhysObjType::kPhoton)){
    if(find(goodPhotons.begin(), goodPhotons.end(), photon) != goodPhotons.end()) continue;
    
    hists.at("lbl_bad_photon_et_" +suffix+datasetName)->Fill(photon->GetEt());
    hists.at("lbl_bad_photon_eta_"+suffix+datasetName)->Fill(photon->GetEta());
    hists.at("lbl_bad_photon_phi_"+suffix+datasetName)->Fill(photon->GetPhi());
  }
}

void fillZDCHists(Event &event, const map<string, TH1D*> &hists, string datasetName, string suffix="")
{
  if(suffix != "") suffix += "_";
  
  double zdcEnergySumPos = 0;
  double zdcEnergySumNeg = 0;
  
  for(auto zdc : event.GetPhysObjects(EPhysObjType::kZDC)){
    if(zdc->GetZside() > 0) zdcEnergySumPos += zdc->GetEnergy();
    else                    zdcEnergySumNeg += zdc->GetEnergy();
  }
  
  hists.at("zdc_sum_energy_pos_"+suffix+datasetName)->Fill(zdcEnergySumPos);
  hists.at("zdc_sum_energy_neg_"+suffix+datasetName)->Fill(zdcEnergySumNeg);
  
}

void fillDiphotonHists(Event &event, const map<string, TH1D*> &hists, string datasetName, string suffix="")
{
  TLorentzVector diphoton = physObjectProcessor.GetDiphoton(*event.GetPhysObjects(EPhysObjType::kGoodPhoton)[0],
                                                            *event.GetPhysObjects(EPhysObjType::kGoodPhoton)[1]);
  if(suffix != "") suffix += "_";
  hists.at("lbl_diphoton_mass_"     +suffix+datasetName)->Fill(diphoton.M());
  hists.at("lbl_diphoton_rapidity_" +suffix+datasetName)->Fill(diphoton.Rapidity());
  hists.at("lbl_diphoton_pt_"       +suffix+datasetName)->Fill(diphoton.Pt());
}

void fillElectronHists(Event &event, const map<string, TH1D*> &hists, string datasetName, string suffix="")
{
  if(suffix != "") suffix += "_";
  for(auto electron : event.GetPhysObjects(EPhysObjType::kGoodMatchedElectron)){
    hists.at("qed_electron_pt_" +suffix+datasetName)->Fill(electron->GetPt());
    hists.at("qed_electron_eta_"+suffix+datasetName)->Fill(electron->GetEta());
    hists.at("qed_electron_phi_"+suffix+datasetName)->Fill(electron->GetPhi());
  }
}

void fillDielectronHists(Event &event, const map<string, TH1D*> &hists, string datasetName, string sample, string suffix="")
{
  if(suffix != "") suffix += "_";
  
  TLorentzVector dielectron = physObjectProcessor.GetDielectron(*event.GetPhysObjects(EPhysObjType::kGoodMatchedElectron)[0],
                                                                *event.GetPhysObjects(EPhysObjType::kGoodMatchedElectron)[1]);
  
  hists.at(sample+"_dielectron_mass_"     +suffix+datasetName)->Fill(dielectron.M());
  hists.at(sample+"_dielectron_rapidity_" +suffix+datasetName)->Fill(dielectron.Rapidity());
  hists.at(sample+"_dielectron_pt_"       +suffix+datasetName)->Fill(dielectron.Pt());
  
}

void fillNoiseHists(Event &event, const map<string, TH1D*> &hists, string datasetName, string sample, string suffix="")
{
  if(suffix != "") suffix += "_";
  
  event.SortCaloTowersByEnergy();
  
  map<ECaloType, bool> leadingFilled;
  for(auto calo : calotypes) leadingFilled[calo] = false;
  
  PhysObjects towers = event.GetPhysObjects(EPhysObjType::kCaloTower);
  
  for(auto tower : towers){
    
    ECaloType subdetHad = tower->GetTowerSubdetHad();
    ECaloType subdetEm = tower->GetTowerSubdetEm();
    
    if(subdetEm==kEB){
      if(   !event.IsOverlappingWithGoodPhoton(*tower)
         && !event.IsOverlappingWithGoodElectron(*tower)){
        if(!leadingFilled[kEB]){
          hists.at(sample+"_EB_leading_tower_"+suffix+datasetName)->Fill(tower->GetEnergyEm());
          leadingFilled[kEB] = true;
        }
      }
    }
    if(subdetEm==kEE){
      if(fabs(tower->GetEta()) < config.params("maxEtaEEtower")
         && !event.IsOverlappingWithGoodPhoton(*tower)
         && !event.IsOverlappingWithGoodElectron(*tower)
         && !physObjectProcessor.IsInHEM(*tower)){
        if(!leadingFilled[kEE]){
          hists.at(sample+"_EE_leading_tower_"+suffix+datasetName)->Fill(tower->GetEnergyEm());
          leadingFilled[kEE] = true;
        }
      }
    }
    if(subdetHad==kHB){
      if(!leadingFilled[kHB]){
        hists.at(sample+"_HB_leading_tower_"+suffix+datasetName)->Fill(tower->GetEnergyHad());
        leadingFilled[kHB] = true;
      }
    }
    if(subdetHad==kHE){
      if(!leadingFilled[kHE]){
        hists.at(sample+"_HE_leading_tower_"+suffix+datasetName)->Fill(tower->GetEnergyHad());
        leadingFilled[kHE] = true;
      }
    }
    if(subdetHad==kHFp){
      hists.at(sample+"_HFp_"+suffix+datasetName)->Fill(tower->GetEnergy());
      
      if(!leadingFilled[kHFp]){
        hists.at(sample+"_HFp_leading_tower_"+suffix+datasetName)->Fill(tower->GetEnergy());
        leadingFilled[kHFp] = true;
      }
    }
    if(subdetHad==kHFm){
      hists.at(sample+"_HFm_"+suffix+datasetName)->Fill(tower->GetEnergy());
      if(!leadingFilled[kHFm]){
        hists.at(sample+"_HFm_leading_tower_"+suffix+datasetName)->Fill(tower->GetEnergy());
        leadingFilled[kHFm] = true;
      }
    }
  }
}

void fillTracksHists(Event &event, const map<string, TH1D*> &hists, EDataset dataset, string suffix="")
{
  string name = datasetName.at(dataset);
  if(suffix != "") suffix += "_";
  
  PhysObjects tracksLowPt, tracksHighPt;
  
  int nTracks = (int)event.GetPhysObjects(EPhysObjType::kGeneralTrack).size();
  
  hists.at("nTracks_"+suffix+name)->Fill(nTracks);
  
  hists.at("nDisplacedTracks_"+suffix+name)->Fill(event.GetNdisplacedTracks());
  hists.at("nDedxHits_"+suffix+name)->Fill(event.GetNdedxHits());
  hists.at("nPixelClusters_"+suffix+name)->Fill(event.GetNpixelClusters());
  hists.at("nPixelRecHits_"+suffix+name)->Fill(event.GetNpixelRecHits());
  
  for(auto track : event.GetPhysObjects(EPhysObjType::kGeneralTrack)){
    double trackPt = track->GetPt();
    hists.at("track_pt_"+suffix+name)->Fill(trackPt);
    hists.at("track_eta_"+suffix+name)->Fill(track->GetEta());
    hists.at("track_phi_"+suffix+name)->Fill(track->GetPhi());
    
    hists.at("track_missing_hits_"  + suffix + name)->Fill(track->GetNmissingHits());
    hists.at("track_valid_hits_"    + suffix + name)->Fill(track->GetNvalidHits());
    hists.at("track_purity_"        + suffix + name)->Fill(track->GetPurity());
    hists.at("track_charge_"        + suffix + name)->Fill(track->GetCharge());
    hists.at("track_chi2_"          + suffix + name)->Fill(track->GetChi2());
    hists.at("track_dxy_"           + suffix + name)->Fill(track->GetDxy());
    
    hists.at("track_dxy_from_bs_"   + suffix + name)->Fill(track->GetXYdistanceFromBeamSpot(dataset));
    hists.at("track_dz_from_bs_"    + suffix + name)->Fill(track->GetZdistanceFromBeamSpot(dataset));
    
    if(nTracks==1){
      hists.at("track_dxy_1_track_" + suffix + name)->Fill(track->GetDxy());
      hists.at("track_dz_1_track_"  + suffix + name)->Fill(track->GetDz());
    }
    if(nTracks==2){
      hists.at("track_dxy_2_track_" + suffix + name)->Fill(track->GetDxy());
      hists.at("track_dz_2_track_"  + suffix + name)->Fill(track->GetDz());
    }
    if(nTracks==3){
      hists.at("track_dxy_3_track_" + suffix + name)->Fill(track->GetDxy());
      hists.at("track_dz_3_track_"  + suffix + name)->Fill(track->GetDz());
    }
    if(nTracks>=4){
      hists.at("track_dxy_ge4_track_" + suffix + name)->Fill(track->GetDxy());
      hists.at("track_dz_ge4_track_"  + suffix + name)->Fill(track->GetDz());
    }
    
    hists.at("track_dz_"            + suffix + name)->Fill(track->GetDz());
    hists.at("track_dxy_over_sigma_"+ suffix + name)->Fill(fabs(track->GetDxy()/track->GetDxyErr()));
    hists.at("track_dz_over_sigma_" + suffix + name)->Fill(fabs(track->GetDz()/track->GetDzErr()));
    hists.at("track_vx_"            + suffix + name)->Fill(track->GetVertexX());
    hists.at("track_vy_"            + suffix + name)->Fill(track->GetVertexY());
    hists.at("track_vz_"            + suffix + name)->Fill(track->GetVertexZ());
    
    if(trackPt < 0.1) tracksLowPt.push_back(track);
    else              tracksHighPt.push_back(track);
  }
  
  hists.at("nTracks_good_"+suffix+name)->Fill(event.GetPhysObjects(EPhysObjType::kGoodGeneralTrack).size());
  
  for(auto track : event.GetPhysObjects(EPhysObjType::kGoodGeneralTrack)){
    hists.at("track_pt_good_"+suffix+name)->Fill(track->GetPt());
    hists.at("track_eta_good_"+suffix+name)->Fill(track->GetEta());
    hists.at("track_phi_good_"+suffix+name)->Fill(track->GetPhi());
  }
}

void fillLbLHistograms(Event &event, const map<string, TH1D*> &hists, EDataset dataset)
{
  string name = datasetName.at(dataset);
  
  int cutThrough=0;
  hists.at("lbl_cut_flow_all_"+name)->Fill(cutThrough++); // 0 - Initial
  
  if(checkTriggers && !event.HasTrigger(kDoubleEG2noHF)) return;
  hists.at("lbl_cut_flow_all_"+name)->Fill(cutThrough++); // 1 - Trigger
  
  if(event.GetPhysObjects(EPhysObjType::kGoodPhoton).size() != 2) return;
  hists.at("lbl_cut_flow_all_"+name)->Fill(cutThrough++); // 2 - Two good photons
  
  TLorentzVector diphoton = physObjectProcessor.GetDiphoton(*event.GetPhysObjects(EPhysObjType::kGoodPhoton)[0],
                                                            *event.GetPhysObjects(EPhysObjType::kGoodPhoton)[1]);
  
  if(diphoton.M() < config.params("diphotonMinMass")) return;
  hists.at("lbl_cut_flow_all_"+name)->Fill(cutThrough++); // 3 - Diphoton mass
  
  if(event.GetPhysObjects(EPhysObjType::kGoodGeneralTrack).size() > config.params("maxNtracks")) return;
  hists.at("lbl_cut_flow_all_"+name)->Fill(cutThrough++); // 4 - CHE (general tracks)
  
  if(event.GetPhysObjects(EPhysObjType::kPixelTrack).size() > config.params("maxNpixelTracks")) return;
  hists.at("lbl_cut_flow_all_"+name)->Fill(cutThrough++); // 5 - CHE (pixel tracks)
  
  if(event.GetNpixelRecHits() > config.params("maxNpixelRecHits")) return;
  hists.at("lbl_cut_flow_all_"+name)->Fill(cutThrough++); // 6 - CHE (pixel hits)
  
  if(event.GetTotalZDCenergy()    > config.params("maxTotalZDCenergy")) return;
  if(event.GetTotalZDCenergyPos() > config.params("maxTotalZDCenergyPerSide") &&
     event.GetTotalZDCenergyNeg() > config.params("maxTotalZDCenergyPerSide")) return;
  
  hists.at("lbl_cut_flow_all_"+name)->Fill(cutThrough++); // 7 - NEE (ZDC)
  
  if(saveCalosFailingNEE){
    map<ECaloType, bool> failingCalo;
    bool failedNEE = event.HasAdditionalTowers(failingCalo);
    for(ECaloType caloType : calotypes){
      if(failingCalo[caloType]) hists.at("lbl_nee_failing_all_"+name)->Fill(caloType);
    }
    if(failedNEE) return;
  }
  else{
    if(event.HasAdditionalTowers()) return;
  }
  hists.at("lbl_cut_flow_all_"+name)->Fill(cutThrough++); // 8 - NEE (Calos)
  
  if(saveTriphotonHists) fillTriphotonHists(event, hists, name);
  

  if(diphoton.Pt() > config.params("diphotonMaxPt")) return;
  hists.at("lbl_cut_flow_all_"+name)->Fill(cutThrough++); // 9 - Diphoton pt
  
  if(fabs(diphoton.Rapidity()) > config.params("diphotonMaxRapidity")) return;
  hists.at("lbl_cut_flow_all_"+name)->Fill(cutThrough++); // 10 - Diphoton rapidity
  
  double aco = physObjectProcessor.GetAcoplanarity(*event.GetPhysObjects(EPhysObjType::kGoodPhoton)[0],
                                                   *event.GetPhysObjects(EPhysObjType::kGoodPhoton)[1]);
  hists.at("lbl_acoplanarity_all_"+name)->Fill(aco);
  
  string suffix;
  if(aco > 0.01){
    suffix = "high_aco";
  }
  else{
    suffix = "low_aco";
    hists.at("lbl_cut_flow_all_"+name)->Fill(cutThrough++); // 11 - Acoplanarity
  }
  
  fillNobjectsHists(event, hists, name, suffix);
  fillNobjectsHists(event, hists, name, "all");
  fillPhotonHists(  event, hists, name, suffix);
  fillPhotonHists(  event, hists, name, "all");
  fillDiphotonHists(event, hists, name, suffix);
  fillDiphotonHists(event, hists, name, "all");
  fillNoiseHists(   event, hists, name, "lbl", suffix);
  fillNoiseHists(   event, hists, name, "lbl", "all");
  fillTracksHists(  event, hists, dataset, "pass_lbl");

  // we found a good event!! cool, dump it to the log
  Log(2) << "PASSING! Run=" << event.GetRunNumber() << " LS=" << event.GetLumiSection() << " Evt=" << event.GetEventNumber() << " aco=" << aco << "\n";
}

void fillCHEhistograms(Event &event, const map<string, TH1D*> &hists, EDataset dataset)
{
  if(checkTriggers && !event.HasTrigger(kDoubleEG2noHF)) return;
  if(event.HasAdditionalTowers()) return;
  
  fillTracksHists(event, hists, dataset, "all");
  
  auto photons = event.GetPhysObjects(EPhysObjType::kGoodPhoton);
  if(photons.size() != 2) return;

  TLorentzVector diphoton = physObjectProcessor.GetDiphoton(*photons[0], *photons[1]);
  if(diphoton.M()  < config.params("diphotonMinMass")) return;
  if(diphoton.Pt() > config.params("diphotonMaxPt")) return;
  
  fillTracksHists(event, hists, dataset, "two_photons");
  
  double aco = physObjectProcessor.GetAcoplanarity(*photons[0], *photons[1]);
  fillTracksHists(event, hists, dataset, aco > config.params("diphotonMaxAco") ? "high_aco" : "low_aco");
}

tuple<double, double, double> getScaleFactor(const shared_ptr<PhysObject> &electron)
{
  double et   = electron->GetPt();
  double eta  = fabs(electron->GetEta());
  
  TGraphAsymmErrors *graph = eta < 1.2 ? triggerScaleFactorsLowEta : triggerScaleFactorsHighEta;
  
  for(int i=0; i<graph->GetN(); i++){
    double x, y;
    double xErrLow  = graph->GetErrorXlow(i);
    double xErrHigh = graph->GetErrorXhigh(i);
    double yErrLow  = graph->GetErrorYlow(i);
    double yErrHigh = graph->GetErrorYhigh(i);
    
    graph->GetPoint(i, x, y);
    
    if(et > x-xErrLow && et < x+xErrHigh) return {y, yErrLow, yErrHigh};
  }
  
  cout<<"WARNING -- could not find trigger scale factor for eta: "<<eta<<"\tet: "<<et<<endl;
  return {1, 0, 0};
}

int getScaleFactorBin(const shared_ptr<PhysObject> &electron)
{
  double et   = electron->GetEt();
  double eta  = fabs(electron->GetEta());
  
  TGraphAsymmErrors *graph = triggerScaleFactorsLowEta;
  
  int bin = 1;
  
  for(int i=0; i<graph->GetN(); i++){
    double x, y;
    double xErrLow  = graph->GetErrorXlow(i);
    double xErrHigh = graph->GetErrorXhigh(i);
    graph->GetPoint(i, x, y);
    
    if(et > x-xErrLow && et < x+xErrHigh) break;
    
    bin++;
  }
  
  if(eta > 1.2) bin += graph->GetN();
  
  return bin;
}

void fillQEDHistograms(Event &event, const map<string, TH1D*> &hists, EDataset dataset)
{
  string name = datasetName.at(dataset);
  
  int cutThrough=0;
  hists.at("qed_cut_flow_all_"+name)->Fill(cutThrough++); // 0 - Initial
    
  if(checkTriggers && !event.HasTrigger(kDoubleEG2noHF)) return;
  hists.at("qed_cut_flow_all_"+name)->Fill(cutThrough++); // 1 - Trigger
  
  if(event.GetPhysObjects(EPhysObjType::kGoodMatchedElectron).size() != 2) return;
  hists.at("qed_cut_flow_all_"+name)->Fill(cutThrough++); // 2 - Two good electrons
  
  auto electron1 = event.GetPhysObjects(EPhysObjType::kGoodMatchedElectron)[0];
  auto electron2 = event.GetPhysObjects(EPhysObjType::kGoodMatchedElectron)[1];
    
  if(electron1->GetCharge() == electron2->GetCharge()){
    fillDielectronHists(event, hists, name, "samesign", "all");
    return;
  }
  hists.at("qed_cut_flow_all_"+name)->Fill(cutThrough++); // 3 - Opposite charge
  
  if(triggerScaleFactorsLowEta && triggerScaleFactorsHighEta){
    
    auto [sf1, sf1ErrLow, sf1ErrHigh] = getScaleFactor(electron1);
    auto [sf2, sf2ErrLow, sf2ErrHigh] = getScaleFactor(electron2);
    
    if(sf1!=1 && sf1ErrLow!=0 && sf1ErrHigh!=0 &&
       sf2!=1 && sf2ErrLow!=0 && sf2ErrHigh!=0){
      
      const int nWeights = 2 * triggerScaleFactorsLowEta->GetN();
      
      SFweights.clear();
      
      SFweights.push_back(sf1*sf2);
      
      for (int ivar=1; ivar<nWeights; ivar++){
        double sf1p=sf1, sf2p=sf2;
        if(getScaleFactorBin(electron1)==ivar) sf1p += (sf1ErrLow + sf1ErrHigh)/2.;
        if(getScaleFactorBin(electron2)==ivar) sf2p += (sf2ErrLow + sf2ErrHigh)/2.;
        SFweights.push_back(sf1p*sf2p);
      }
      
      triggerScaleFactorsTree->Fill();
    }
  }
    
  
  TLorentzVector dielectron = physObjectProcessor.GetDielectron(*event.GetPhysObjects(EPhysObjType::kGoodMatchedElectron)[0],
                                                                *event.GetPhysObjects(EPhysObjType::kGoodMatchedElectron)[1]);
  
  if(dielectron.M() < config.params("dielectronMinMass")) return;
  hists.at("qed_cut_flow_all_"+name)->Fill(cutThrough++); // 4 - Dielectron mass
  
  if(event.GetPhysObjects(EPhysObjType::kGoodGeneralTrack).size() != 2) return;
  hists.at("qed_cut_flow_all_"+name)->Fill(cutThrough++); // 5 - CHE
  
  if(event.GetTotalZDCenergy()    > config.params("maxTotalZDCenergy")) return;
  if(event.GetTotalZDCenergyPos() > config.params("maxTotalZDCenergyPerSide") &&
     event.GetTotalZDCenergyNeg() > config.params("maxTotalZDCenergyPerSide")) return;
  
  hists.at("qed_cut_flow_all_"+name)->Fill(cutThrough++); // 6 - ZDC
  
  if(event.HasAdditionalTowers()) return;
  hists.at("qed_cut_flow_all_"+name)->Fill(cutThrough++); // 7 - NEE
  
  if(dielectron.Pt() > config.params("dielectronMaxPt")) return;
  hists.at("qed_cut_flow_all_"+name)->Fill(cutThrough++); // 8 - Dielectron pt
  
  if(fabs(dielectron.Rapidity()) > config.params("dielectronMaxRapidity")) return;
  hists.at("qed_cut_flow_all_"+name)->Fill(cutThrough++); // 9 - Dielectron rapidity
  
  double aco = physObjectProcessor.GetAcoplanarity(*event.GetPhysObjects(EPhysObjType::kGoodMatchedElectron)[0],
                                                   *event.GetPhysObjects(EPhysObjType::kGoodMatchedElectron)[1]);
  
  hists.at("qed_acoplanarity_all_"+name)->Fill(aco);
  
  fillElectronHists(  event, hists, name, "all");
  fillDielectronHists(event, hists, name, "qed", "all");
  fillNoiseHists(     event, hists, name, "qed", "all");
  fillTracksHists(    event, hists, dataset, "pass_qed");
}

/// Creates histograms, cut through and event counters for given dataset name, for each
/// histogram specified in `histParams` vector.
void InitializeHistograms(map<string, TH1D*> &hists, string datasetType, string suffix="")
{
  if(suffix != "") suffix = "_" + suffix;
  
  for(auto &[histName, nBins, min, max] : histParams){
    string title = histName + suffix + "_" + datasetType;
    if(hists.find(title) != hists.end()) continue;
    hists[title] = new TH1D(title.c_str(), title.c_str(), nBins, min, max);
  }
}

int main(int argc, char* argv[])
{
  cout<<"Starting prepareBasicPlots\n";
  
  if(argc != 6){
    cout<<"This app requires 5 parameters:\n";
    cout<<"./prepareBasicPlots configPath inputPath outputPath datasetName[Data|QED_SC|QED_SL|LbL|CEP] triggerScaleFactorsPath\n";
    exit(0);
  }
  
  cout<<"Reading input arguments"<<endl;
  
  string configPath              = argv[1];
  string inputPath               = argv[2];
  string outputPath              = argv[3];
  string sampleName              = argv[4];
  string triggerScaleFactorsPath = argv[5];
  
  
  cout<<"Config: "<<configPath<<endl;
  cout<<"Input: "<<inputPath<<endl;
  cout<<"Output: "<<outputPath<<endl;
  cout<<"Sample name: "<<sampleName<<endl;
  cout<<"Trigger scale factors path: "<<triggerScaleFactorsPath<<endl;
  
  config = ConfigManager(configPath);
  
  TFile *triggerScaleFactorsFile = nullptr;
  if(sampleName == "Data"){
    triggerScaleFactorsFile = TFile::Open(triggerScaleFactorsPath.c_str());
    
    if(!triggerScaleFactorsFile){
      cout<<"ERROR -- could not open file with trigger scale factors: "<<triggerScaleFactorsPath<<endl;
    }
    else{
      triggerScaleFactorsLowEta   = (TGraphAsymmErrors*)triggerScaleFactorsFile->Get("triggerScaleFactors_vsEt_etaBelow1p2");
      triggerScaleFactorsHighEta  = (TGraphAsymmErrors*)triggerScaleFactorsFile->Get("triggerScaleFactors_vsEt_etaAbove1p2");
    }
  }
  
  
  
  map<string, TH1D*> hists;
  
  TFile *outFile = new TFile(outputPath.c_str(), "recreate");
  
  triggerScaleFactorsTree = new TTree("triggerScaleFactorsTree", "triggerScaleFactorsTree");
  triggerScaleFactorsTree->Branch("SFweights", &SFweights);
  
  
  EDataset dataset = nDatasets;
  
  if(sampleName == "Data")    dataset = kData;
  if(sampleName == "QED_SC")  dataset = kMCqedSC;
  if(sampleName == "QED_SL")  dataset = kMCqedSL;
  if(sampleName == "LbL")     dataset = kMClbl;
  if(sampleName == "CEP")     dataset = kMCcep;
  
  
  auto events = make_unique<EventProcessor>(inputPath, dataset);
  
  for(string suffix : suffixes){
    InitializeHistograms(hists, sampleName, suffix);
  }
  
  
  if(dataset == nDatasets){
    cout<<"ERROR -- unknown dataset name provided: "<<sampleName<<"\n";
    exit(0);
  }
  
  for(int iEvent=0; iEvent<events->GetNevents(); iEvent++){
    if(iEvent%1000 == 0)  Log(1)<<"Processing event "<<iEvent<<"\n";
    if(iEvent%10000 == 0) cout<<"Processing event "<<iEvent<<"\n";
    if(iEvent >= config.params("maxEvents")) break;
    
    auto event = events->GetEvent(iEvent);
    
    // run this here just to save electron cut flow hist
    event->GetPhysObjects(EPhysObjType::kGoodElectron, hists.at("qed_electron_cutflow_all_"+sampleName));
    event->GetPhysObjects(EPhysObjType::kGoodGeneralTrack, hists.at("tracks_cut_flow_all_"+sampleName));
    
    fillLbLHistograms(*event, hists, dataset);
    fillCHEhistograms(*event, hists, dataset);
    fillQEDHistograms(*event, hists, dataset);
    fillZDCHists(*event, hists, sampleName, "all");
  }
  
  outFile->cd();
  for(auto &[histName, hist] : hists) hist->Write();
  

  cout<<"N events with 3 photons: "<<nThreePhotonEvents<<"\n";
  
  outFile->cd();
  triggerScaleFactorsTree->Write();
  outFile->Close();
  
}
