{
   auto c = new TCanvas("c","Graph2D example",0,0,600,400);
   Double_t x, y, z, P = 6.;
   Int_t np = 200;
   auto dt = new TGraph2D();
   dt->SetTitle("Graph title; X axis title; Y axis title; Z axis title");
   auto r = new TRandom();
   for (Int_t N=0; N<np; N++) {
      x = 2*P*(r->Rndm(N))-P;
      y = 2*P*(r->Rndm(N))-P;
      z = (sin(x)/x)*(sin(y)/y)+0.2;
      dt->SetPoint(N,x,y,z);
   }
   gStyle->SetPalette(1);
   dt->Draw("surf1");
}
