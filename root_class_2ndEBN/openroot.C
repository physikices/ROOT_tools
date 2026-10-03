{
  auto arq = new TFile("resultados.root","open");

  auto dados = (TGraph*) arq->Get("linear");

  dados->SetMarkerSize(1.5);
  dados->SetMarkerStyle(kFullSquare);
  dados->Draw("AP");

  arq->Close();
}
