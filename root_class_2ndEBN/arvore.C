{
  auto file = new TFile("tree.root","recreate");
  auto arvre = new TTree("dados","My data Tree");

  int   event;
  float En;

  arvre->Branch("event",&event);
  arvre->Branch("energy",&En);

  auto pdf = new TF1("pdf","gaus",0,100);
  pdf->SetParameters(1,50,10);

  for(auto i=0; i<10000; i++){
    event = i;
    En = pdf->GetRandom();

    arvre->Fill();
  }

  arvre->Write();
  file->Close();
}
