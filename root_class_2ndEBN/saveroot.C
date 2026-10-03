{
  auto arq = new TFile("resultados.root","create");

  auto dados = new TGraph("dat.txt");

  dados->SetName("linear");
  dados->Write();
  arq->Close();
}
