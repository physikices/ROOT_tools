void stdRoot_sty() {
  // ========= CONFIGURAÇÃO BÁSICA =========
  gROOT->SetBatch(kTRUE);
  gStyle->SetOptStat(0);
  gStyle->SetTextFont(42);
  gStyle->SetTitleFont(42, "XYZ");
  gStyle->SetLabelFont(42, "XYZ");
  gStyle->SetTitleSize(0.05, "XYZ");
  gStyle->SetTitleOffset(0, "XYZ");
  gStyle->SetLabelSize(0.045, "XYZ");
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);

  // ========= CONFIGURAÇÃO ESPECÍFICA ======
  std::ifstream infile("../plots/output2root.dat");
  std::string line;
  std::vector<std::string> header;
  std::vector<double> x, y;

  // 1) Ler cabeçalho (linhas começando com #)
  while (std::getline(infile, line)) {
    if (line.size() > 0 && line[0] == '#') {
      header.push_back(line.substr(1)); // remove o '#'
    } else {
      // não é cabeçalho -> volta o ponteiro para reler
      std::stringstream ss(line);
      double xv, yv;
      ss >> xv >> yv;
      x.push_back(xv);
      y.push_back(yv);
      break;
    }
  }

  // 2) Ler o resto dos dados
  double xv, yv;
  while (infile >> xv >> yv) {
    x.push_back(xv);
    y.push_back(yv);
  }
  infile.close();

  // 3) Criar gráfico
  TGraph *gr = new TGraph(x.size(), x.data(), y.data());
  gr->SetTitle("");
  gr->SetMarkerStyle(21);
  gr->SetMarkerColor(kMagenta+1);
  gr->SetMarkerSize(0.0);
  gr->SetLineStyle(1);
  gr->SetLineColor(kMagenta+1);
  gr->SetLineWidth(3);

  // --- Define títulos dos eixos somente se houver cabeçalho ---
  if (header.size() > 4) {
    gr->GetXaxis()->SetTitle(header[3].c_str());
    gr->GetYaxis()->SetTitle(header[4].c_str());
  } else {
    gr->GetXaxis()->SetTitle("x");
    gr->GetYaxis()->SetTitle("y");
  }

  gr->GetXaxis()->SetTitleSize(0.06);
  gr->GetXaxis()->SetTitleOffset(1.2);
  gr->GetXaxis()->SetLabelOffset(0.01);

  gr->GetYaxis()->SetTitleSize(0.06);
  gr->GetYaxis()->SetTitleOffset(1.8);
  gr->GetYaxis()->SetLabelOffset(0.01);

  TCanvas* c1 = new TCanvas("c1","test",800,720);
  c1->SetLeftMargin(0.15);
  c1->SetBottomMargin(0.20);
  c1->SetTickx();
  c1->SetTicky();
  // c1->SetLogy();
  c1->SetLogx();

  // gr->GetXaxis()->SetRangeUser(0.00000001, 0.9);
  gr->GetYaxis()->SetRangeUser(0, 2.0);
  gr->Draw("ALP");

  // 4) Usar cabeçalho no título/legenda se disponível
  if (header.size() > 1) {
    double xmax = 0.90, xmin = 0.37, ymax = 0.85, ymin = 0.70;
    TLegend *leg = new TLegend(xmin, ymin, xmax, ymax);
    leg->SetTextSize(0.033);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->AddEntry(gr, header[1].c_str(), "lp");
    leg->Draw();
  }

  // título superior: energia + colisor (se existir)
  if (header.size() > 2) {
    TLatex latex;
    latex.SetNDC();
    latex.SetTextSize(0.04);
    latex.SetTextAlign(13); // left-top
    latex.DrawLatex(0.50, 0.85, Form("%s", header[2].c_str()));
  }

  // Linha horizontal (referência)
  // TLine *hline = new TLine(0.0, 1.0, 0.8, 1.0);
  // hline->SetLineStyle(3);
  // hline->SetLineWidth(2);
  // hline->SetLineColor(kGray+1);
  // hline->Draw("same");

  // c1->SaveAs("../plots/stdRoot_out.eps");
  // c1->SaveAs("../plots/stdRoot_out.pdf");
  c1->Print("../plots/tex_compile/stdRoot_out.tex");
}
