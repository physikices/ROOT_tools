{
  gRandom = new TRandom(234);

  auto pdf = new TF1("pdf","gaus",0,100);
  pdf->SetParameters(1,50,10);
  // pdf->Draw();

  auto hist = new TH1F("hist","My Hist",100,0,100);
  hist->Draw();

  for(auto i=0; i<10000; i++) 
    hist->Fill(
        pdf->GetRandom()
    );
}
