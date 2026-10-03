{
  auto file = new TFile("tree.root","open");
  auto tree = (TTree*) file->Get("dados");

  int   event;
  float energy;


  tree->SetBranchAddress("event",&event);
  tree->SetBranchAddress("energy",&energy);

  auto hist = new TH1F("hist","Tree Energy",100,0,100);

  for(auto i=0; i<tree->GetEntries(); i++) {
    tree->GetEntry(i);
    hist->Fill(energy);
  }

  hist->Draw();

  // auto hist2 = new TH1F(hist);
  // hist2->Draw();

  // file->Close();

}
