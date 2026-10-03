{
  auto f = new TF1{"P","sin([1]/x)",0.1,10};

  f->SetParName(0,"amplitude");
  f->SetParName(1,"comprimento");

  f->SetParameter("amplitude",2.5);
  f->SetParameter("comprimento",0.5);

  f->SetNpx(10000);
  f->Draw();

  f->SetTitle("Hello ROOT");
  f->GetXaxis()->SetTitle("E_{#nu} (MeV)");
  f->GetYaxis()->SetTitle("P_{#nu} (MeV)");
  
  f->SetLineColor(kMagenta);
}
