{
  auto dados = new TGraphErrors("dat.txt");
  dados->SetMarkerSize(1.5);
  dados->SetMarkerStyle(38);
  dados->Draw("AP");

  // auto modelo = new TF1("modelo","[0]*x+[1]",0,10);
  // dados->Fit(modelo);

}
