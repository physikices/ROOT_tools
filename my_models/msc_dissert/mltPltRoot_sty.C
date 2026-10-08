void mltPltRoot_sty() {

  // ========= CONFIGURAÇÃO BÁSICA =========
  gROOT->SetBatch(kTRUE);
  gStyle->SetOptStat(0);
  gStyle->SetTextFont(42);
  gStyle->SetTitleFont(42, "XYZ");
  gStyle->SetLabelFont(42, "XYZ");
  gStyle->SetTitleSize(0.05, "XYZ");
  gStyle->SetLabelSize(0.045, "XYZ");
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);

  // ========= DIRETÓRIOS =========
  TString inDir  = "../plots/";
  TString outDir = "../plots/";
  TString outDirTex = "../plots/tex_compile/";

  // ========= DEFINIÇÃO DO STRUCT =========
  struct PlotConfig {
    TString file;
    TString label;
    Color_t color;
    Style_t marker;
    Style_t linestyle;
    double msize;
  };

  struct EnergyConfig {
    TString energy;
    TString collider;
    std::vector<PlotConfig> plots;
  };

  // ========= LISTA DE ENERGIAS =========
  std::vector<EnergyConfig> energies = {
    {"14 TeV", "LHC", {
      {"LHC_14TeV_PLB_mod1.dat",   "PLB   J0, model 1", kBlue+1,    20, 1, 0.0},
      {"LHC_14TeV_PLB_mod2.dat",   "PLB   J2, model 2", kRed+1,     21, 1, 0.0},
      {"LHC_14TeV_PRD_mod1.dat",   "PRD  J0, model 1",  kGreen+2,   22, 1, 0.0},
      {"LHC_14TeV_PRD_mod2.dat",   "PRD  J0, model 2",  kMagenta+1, 23, 1, 0.0},
      {"LHC_14TeV_arXiv_mod1.dat", "arXiv J0, model 1", kOrange+1,  24, 1, 0.0},
      {"LHC_14TeV_arXiv_mod2.dat", "arXiv J0, model 2", kCyan+2,    25, 1, 0.0}
    }},
    {"100 TeV", "FCC", {
      {"FCC_100TeV_PLB_mod1.dat",   "PLB   J0, model 1", kBlue+1,    21, 1, 0.0},
      {"FCC_100TeV_PLB_mod2.dat",   "PLB   J2, model 2", kRed+1,     22, 1, 0.0},
      {"FCC_100TeV_PRD_mod1.dat",   "PRD  J0, model 1",  kGreen+2,   23, 1, 0.0},
      {"FCC_100TeV_PRD_mod2.dat",   "PRD  J0, model 2",  kMagenta+1, 24, 1, 0.0},
      {"FCC_100TeV_arXiv_mod1.dat", "arXiv J0, model 1", kOrange+1,  25, 1, 0.0},
      {"FCC_100TeV_arXiv_mod2.dat", "arXiv J0, model 2", kCyan+2,    26, 1, 0.0}
    }},
    {"8.16 TeV", "LHC", {
      {"LHC_8.16TeV_PLB_mod1.dat",   "PLB   J0, model 1", kBlue+1,    21, 1, 0.0},
      {"LHC_8.16TeV_PLB_mod2.dat",   "PLB   J2, model 2", kRed+1,     22, 1, 0.0},
      {"LHC_8.16TeV_PRD_mod1.dat",   "PRD  J0, model 1",  kGreen+2,   23, 1, 0.0},
      {"LHC_8.16TeV_PRD_mod2.dat",   "PRD  J0, model 2",  kMagenta+1, 24, 1, 0.0},
      {"LHC_8.16TeV_arXiv_mod1.dat", "arXiv J0, model 1", kOrange+1,  25, 1, 0.0},
      {"LHC_8.16TeV_arXiv_mod2.dat", "arXiv J0, model 2", kCyan+2,    26, 1, 0.0}
    }},
    {"63 TeV", "FCC", {
      {"FCC_63TeV_PLB_mod1.dat",   "PLB   J0, model 1", kBlue+1,    21, 1, 0.0},
      {"FCC_63TeV_PLB_mod2.dat",   "PLB   J2, model 2", kRed+1,     22, 1, 0.0},
      {"FCC_63TeV_PRD_mod1.dat",   "PRD  J0, model 1",  kGreen+2,   23, 1, 0.0},
      {"FCC_63TeV_PRD_mod2.dat",   "PRD  J0, model 2",  kMagenta+1, 24, 1, 0.0},
      {"FCC_63TeV_arXiv_mod1.dat", "arXiv J0, model 1", kOrange+1,  25, 1, 0.0},
      {"FCC_63TeV_arXiv_mod2.dat", "arXiv J0, model 2", kCyan+2,    26, 1, 0.0}
    }},
    {"5.36 TeV", "LHC", {
      {"LHC_5.36TeV_PLB_mod1.dat",   "PLB   J0, model 1", kBlue+1,    21, 1, 0.0},
      {"LHC_5.36TeV_PLB_mod2.dat",   "PLB   J2, model 2", kRed+1,     22, 1, 0.0},
      {"LHC_5.36TeV_PRD_mod1.dat",   "PRD  J0, model 1",  kGreen+2,   23, 1, 0.0},
      {"LHC_5.36TeV_PRD_mod2.dat",   "PRD  J0, model 2",  kMagenta+1, 24, 1, 0.0},
      {"LHC_5.36TeV_arXiv_mod1.dat", "arXiv J0, model 1", kOrange+1,  25, 1, 0.0},
      {"LHC_5.36TeV_arXiv_mod2.dat", "arXiv J0, model 2", kCyan+2,    26, 1, 0.0}
    }},
    {"39 TeV", "FCC", {
      {"FCC_39TeV_PLB_mod1.dat",   "PLB   J0, model 1", kBlue+1,    21, 1, 0.0},
      {"FCC_39TeV_PLB_mod2.dat",   "PLB   J2, model 2", kRed+1,     22, 1, 0.0},
      {"FCC_39TeV_PRD_mod1.dat",   "PRD  J0, model 1",  kGreen+2,   23, 1, 0.0},
      {"FCC_39TeV_PRD_mod2.dat",   "PRD  J0, model 2",  kMagenta+1, 24, 1, 0.0},
      {"FCC_39TeV_arXiv_mod1.dat", "arXiv J0, model 1", kOrange+1,  25, 1, 0.0},
      {"FCC_39TeV_arXiv_mod2.dat", "arXiv J0, model 2", kCyan+2,    26, 1, 0.0}
    }}
  };

  // ========= CANVAS DIVIDIDO (sem espaço) =========
  int N = energies.size();
  int ncols = 2;
  int nrows = (N+1)/2;
  TCanvas* c1 = new TCanvas("c1", "Comparacao multi-energias", 800, 800);
  c1->Divide(ncols, nrows, 0, 0);  // sem espaçamento entre pads

  // ========= LOOP NAS ENERGIAS =========
  for (int ie=0; ie<N; ie++) {
    c1->cd(ie+1);

    // margens reduzidas para "colar" gráficos
    gPad->SetLeftMargin(0.15);
    gPad->SetRightMargin(0.02);
    // gPad->SetBottomMargin(0.12);
    // gPad->SetTopMargin(0.05);
    // gPad->SetLeftMargin(0);
    // gPad->SetRightMargin(0);
    gPad->SetBottomMargin(0.12);
    gPad->SetTopMargin(0);
    // gPad->SetLogy();

    TMultiGraph* mg = new TMultiGraph();

    // --- carregar os dados de energia
    for (auto &cfg : energies[ie].plots) {
      if (gSystem->AccessPathName(inDir + cfg.file)) {
        std::cout << "Aviso: " << inDir + cfg.file << " não encontrado. Pulando...\n";
        continue;
      }
      TGraph* gr = new TGraph(inDir + cfg.file);
      gr->SetLineColor(cfg.color);
      gr->SetMarkerColor(cfg.color);
      gr->SetMarkerStyle(cfg.marker);
      gr->SetMarkerSize(cfg.msize);
      gr->SetLineStyle(cfg.linestyle);
      gr->SetLineWidth(1);
      mg->Add(gr, "LP");
    }

    mg->Draw("A");
    // double ymin = mg->GetYaxis()->GetXmin();
    // double ymax = mg->GetYaxis()->GetXmax();
    mg->GetXaxis()->SetTitle("Y");
    mg->GetYaxis()->SetTitle("#sigma_{#gamma#gamma} #rightarrow T_{4c}");
    mg->GetXaxis()->CenterTitle();
    mg->GetYaxis()->CenterTitle();
    mg->GetXaxis()->SetTitleOffset(1.0);
    mg->GetYaxis()->SetTitleOffset(1.5);
    mg->GetXaxis()->SetLabelOffset(0.01);
    mg->GetYaxis()->SetLabelOffset(0.01);
    // mg->SetMaximum(ymax * 1.1);   // dá um fator de respiro no topo
    // mg->SetMinimum(ymin * 0.8);   // opcional, para ajustar embaixo

    // --- legenda dos modelos
    TLegend* leg = new TLegend(0.72, 0.65, 0.80, 0.95);
    leg->SetTextSize(0.04);
    leg->SetBorderSize(0);
    leg->SetFillColor(kWhite);
    for (int i=0; i<mg->GetListOfGraphs()->GetSize(); i++) {
      leg->AddEntry((TGraph*)mg->GetListOfGraphs()->At(i), energies[ie].plots[i].label, "lp");
    }
    leg->Draw();

    // --- faixa de rapidez
    // auto rLeg = new TLegend(0.20,0.80,0.20,0.84);
    // rLeg->SetTextSize(0.05);
    // rLeg->SetBorderSize(0);
    // rLeg->SetFillStyle(0);
    // rLeg->AddEntry((TObject*)0,"-4.5 #leq #it{Y} #leq 2.0","");
    // rLeg->Draw();

    // --- título superior: energia + colisor
    TLatex latex;
    latex.SetNDC();
    latex.SetTextSize(0.06);
    latex.SetTextAlign(13); // left-top
    latex.DrawLatex(0.20, 0.94, Form("%s  (%s)", energies[ie].energy.Data(), energies[ie].collider.Data()));
  }

  // ========= EXPORTAR =========
  // c1->SaveAs(outDir + "mltPltRoot_out.png");
  // c1->SaveAs(outDir + "mltPltRoot_out.pdf");
  c1->Print(outDirTex + "mltPltRoot_out.tex");
}
