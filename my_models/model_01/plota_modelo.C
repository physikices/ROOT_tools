// Reproduz (layout) a figura modelo: dois paineis, escala log em y.
//   Esquerdo: dsigma/dPT [nb/GeV] x PT [GeV]  <- tabela_PT.dat
//   Direito : sigma [nb] x W_gp [GeV]         <- tabela_W.dat
//
// Uso:
//   root -l -b -q 'plota_modelo.C'
//   root -l -b -q 'plota_modelo.C("tabela_PT.dat","tabela_W.dat","fig_modelo.pdf")'
//   (a extensao de "saida" decide o formato: .pdf, .png, .tex ...)
//
// Formato das tabelas: serie xlo xhi y eyl eyh  (veja gen_tabelas.py)
//
// Geometria medida na figura modelo (1021 x 376 px):
//   painel esq.: caixa dos eixos x 80-491,  y 25-329
//   painel dir.: caixa dos eixos x 561-972, y 25-329

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

struct Serie {
    std::string nome;
    std::vector<double> xlo, xhi, y, eyl, eyh;
};

static std::string Trim(std::string s) {
    const char* ws = " \t\r\n";
    s.erase(0, s.find_first_not_of(ws));
    s.erase(s.find_last_not_of(ws) + 1);
    return s;
}

static std::vector<Serie> LeTabela(const std::string& arq) {
    std::vector<Serie> v;
    std::ifstream f(arq);
    if (!f.is_open()) {
        std::cerr << "Erro ao abrir " << arq << "\n";
        return v;
    }
    std::string l;
    while (std::getline(f, l)) {
        l = Trim(l);
        if (l.empty() || l[0] == '#') continue;
        std::istringstream iss(l);
        std::string nome;
        double a, b, c, d, e;
        if (!(iss >> nome >> a >> b >> c >> d >> e)) continue;
        Serie* s = nullptr;
        for (auto& x : v) if (x.nome == nome) s = &x;
        if (!s) { v.push_back(Serie()); s = &v.back(); s->nome = nome; }
        s->xlo.push_back(a); s->xhi.push_back(b); s->y.push_back(c);
        s->eyl.push_back(d); s->eyh.push_back(e);
    }
    return v;
}

static const Serie* Acha(const std::vector<Serie>& v, const char* nome) {
    for (const auto& s : v) if (s.nome == nome) return &s;
    std::cerr << "Serie ausente na tabela: " << nome << "\n";
    return nullptr;
}

// Curva em degraus (histograma) a partir de [xlo,xhi] -> y
static TGraph* Degrau(const Serie* s, int cor, int estilo) {
    std::vector<double> x, y;
    for (size_t i = 0; i < s->y.size(); ++i) {
        x.push_back(s->xlo[i]); y.push_back(s->y[i]);
        x.push_back(s->xhi[i]); y.push_back(s->y[i]);
    }
    auto* g = new TGraph(x.size(), x.data(), y.data());
    g->SetLineColor(cor);
    g->SetLineStyle(estilo);
    g->SetLineWidth(2);
    return g;
}

// Pontos com barras de erro (x = meia largura do bin)
static TGraphAsymmErrors* Dados(const Serie* s, int cor, int marcador, double tam) {
    const int n = s->y.size();
    std::vector<double> xc(n), ex(n);
    for (int i = 0; i < n; ++i) {
        xc[i] = 0.5 * (s->xlo[i] + s->xhi[i]);
        ex[i] = 0.5 * (s->xhi[i] - s->xlo[i]);
    }
    auto* g = new TGraphAsymmErrors(n, xc.data(), s->y.data(), ex.data(), ex.data(),
                                    s->eyl.data(), s->eyh.data());
    g->SetMarkerStyle(marcador);
    g->SetMarkerSize(tam);
    g->SetMarkerColor(cor);
    g->SetLineColor(cor);
    g->SetLineWidth(1);
    return g;
}

// Banda em degraus entre duas series
static TGraph* Banda(const Serie* lo, const Serie* hi, int cor) {
    std::vector<double> x, y;
    const int n = lo->y.size();
    for (int i = 0; i < n; ++i) {
        x.push_back(hi->xlo[i]); y.push_back(hi->y[i]);
        x.push_back(hi->xhi[i]); y.push_back(hi->y[i]);
    }
    for (int i = n - 1; i >= 0; --i) {
        x.push_back(lo->xhi[i]); y.push_back(lo->y[i]);
        x.push_back(lo->xlo[i]); y.push_back(lo->y[i]);
    }
    auto* g = new TGraph(x.size(), x.data(), y.data());
    g->SetFillColor(cor);
    g->SetFillStyle(1001);
    g->SetLineWidth(0);
    return g;
}

static void EstiloLegenda(TLegend* l, bool moldura, int corMoldura) {
    l->SetTextFont(42);
    l->SetTextSize(0.030);
    l->SetMargin(0.17);
    if (moldura) {
        l->SetBorderSize(1);
        l->SetLineColor(corMoldura);
        l->SetFillColor(kWhite);
        l->SetFillStyle(1001);
    } else {
        l->SetBorderSize(0);
        l->SetFillStyle(0);
    }
}

static void ConfiguraPad(TPad* p, double L, double R, double B, double T) {
    p->SetMargin(L, R, B, T);
    p->SetLogy();
    p->SetTicks(1, 1);
    p->SetFillStyle(4000);        // transparente: os pads se sobrepoem
    p->SetFrameFillStyle(0);
    p->SetFrameLineWidth(1);
}

static TH1* Moldura(double x0, double y0, double x1, double y1, const char* tx,
                    const char* ty, int ndivX, double offY) {
    TH1* f = gPad->DrawFrame(x0, y0, x1, y1);
    f->GetXaxis()->SetTitle(tx);
    f->GetYaxis()->SetTitle(ty);
    for (TAxis* ax : {f->GetXaxis(), f->GetYaxis()}) {
        ax->SetLabelFont(42);
        ax->SetTitleFont(42);
        ax->SetLabelSize(0.036);
        ax->SetTitleSize(0.050);
    }
    f->GetXaxis()->SetNdivisions(ndivX, kTRUE);
    f->GetXaxis()->SetTitleOffset(0.95);
    f->GetXaxis()->SetLabelOffset(0.012);
    f->GetYaxis()->SetTitleOffset(offY);
    f->GetYaxis()->SetLabelOffset(0.008);
    f->GetXaxis()->SetTickLength(0.020);
    f->GetYaxis()->SetTickLength(0.015);
    return f;
}

static void Texto(TLatex& t, double x, double y, const char* s) { t.DrawLatex(x, y, s); }

void plota_modelo(const char* tabPT = "tabela_PT.dat",
                  const char* tabW  = "tabela_W.dat",
                  const char* saida = "fig_modelo.pdf") {
    // ========= GEOMETRIA (pixels da figura modelo) =========
    const double Wpx = 1021, Hpx = 376;
    const double p1x1 = 510;                 // pad esq.: 0 .. 510
    const double p2x0 = 466;                 // pad dir.: 466 .. 1021 (sobrepoe o esq.)
    const double ax1L = 80,  ax1R = 491;     // eixos painel esq.
    const double ax2L = 561, ax2R = 972;     // eixos painel dir.
    const double axT  = 25,  axB  = 329;

    // ========= CORES =========
    const int cTeal   = TColor::GetColor("#00a6a6");
    const int cNavy   = TColor::GetColor("#1a1a9c");
    const int cBand   = TColor::GetColor("#b8cacb");
    const int cGrey   = TColor::GetColor("#8c8c8c");
    const int cLegBox = TColor::GetColor("#cccccc");

    gROOT->SetBatch(kTRUE);
    gStyle->SetOptStat(0);
    gStyle->SetTextFont(42);
    gStyle->SetEndErrorSize(0);              // sem "tampinhas" nas barras de erro
    gStyle->SetLineScalePS(1.5);

    // ========= LEITURA DAS TABELAS =========
    auto t1 = LeTabela(tabPT);
    auto t2 = LeTabela(tabW);
    if (t1.empty() || t2.empty()) return;

    const Serie *a1 = Acha(t1, "ts1_ho"), *a2 = Acha(t1, "ts1_py"),
                *a3 = Acha(t1, "s0_ho"),  *a4 = Acha(t1, "s0_py"),
                *d1 = Acha(t1, "h1_all"), *d2 = Acha(t1, "h1_bsub");
    const Serie *b1 = Acha(t2, "ts1_ho"), *b2 = Acha(t2, "ts1_tune"),
                *b3 = Acha(t2, "s0_ho"),  *b4 = Acha(t2, "s0_tune"),
                *bl = Acha(t2, "band_lo"), *bh = Acha(t2, "band_hi"),
                *dd = Acha(t2, "h1_2010");
    if (!a1||!a2||!a3||!a4||!d1||!d2||!b1||!b2||!b3||!b4||!bl||!bh||!dd) return;

    std::cout << "Elementos: painel esq. = 4 curvas (" << a1->y.size() << " bins) + "
              << d1->y.size() << " pontos H1 + " << d2->y.size() << " pontos H1 (b subtraido); "
              << "painel dir. = 4 curvas (" << b1->y.size() << " bins) + banda + "
              << dd->y.size() << " pontos H1\n";

    // ========= CANVAS E PADS =========
    auto* c = new TCanvas("c", "fig_modelo", (int)Wpx, (int)Hpx);
    c->SetFillColor(kWhite);

    auto* p1 = new TPad("p1", "", 0.0, 0.0, p1x1 / Wpx, 1.0);
    auto* p2 = new TPad("p2", "", p2x0 / Wpx, 0.0, 1.0, 1.0);
    const double w1 = p1x1, w2 = Wpx - p2x0;
    ConfiguraPad(p1, ax1L / w1, (w1 - ax1R) / w1, (Hpx - axB) / Hpx, axT / Hpx);
    ConfiguraPad(p2, (ax2L - p2x0) / w2, (Wpx - ax2R) / w2, (Hpx - axB) / Hpx, axT / Hpx);
    p1->Draw();
    p2->Draw();

    TLatex tx;
    tx.SetNDC();
    tx.SetTextFont(42);
    tx.SetTextAlign(12);
    tx.SetTextSize(0.034);
    tx.SetTextColor(kBlack);

    // ================= PAINEL ESQUERDO =================
    p1->cd();
    Moldura(0, 1e-5, 10, 25, "#it{P}_{T} [GeV]", "d#sigma/d#it{P}_{T} [nb/GeV]", 405, 1.18);

    auto* g1 = Degrau(a1, cTeal, 3);     // pontilhado
    auto* g2 = Degrau(a2, cTeal, 2);     // tracejado
    auto* g3 = Degrau(a3, cNavy, 2);     // tracejado
    auto* g4 = Degrau(a4, cNavy, 1);     // solido
    auto* gd1 = Dados(d1, kBlack, 2, 1.3);   // marcador "+"
    auto* gd2 = Dados(d2, cGrey, 2, 1.3);

    for (auto* g : {g3, g4, g1, g2}) g->Draw("L");
    gd2->Draw("PZ");
    gd1->Draw("PZ");

    auto* l1 = new TLegend(0.575, 0.614, 0.962, 0.920);
    EstiloLegenda(l1, true, cLegBox);
    l1->SetHeader("Before tune:", "C");
    l1->AddEntry(g1, "HO2.6.7: ^{3}S_{1}^{[1]}", "l");
    l1->AddEntry(g2, "HO2.6.7 + PYTHIA8.310: ^{3}S_{1}^{[1]}", "l");
    l1->AddEntry(g3, "HO2.6.7: ^{1}S_{0}^{[8]}", "l");
    l1->AddEntry(g4, "HO2.6.7 + PYTHIA8.310: ^{1}S_{0}^{[8]}", "l");
    l1->AddEntry(gd1, "H1 data: NPB 472 (1996) 3-31,", "pe");
    l1->AddEntry((TObject*)nullptr, "EPJC 25 (2002) 41-53,", "");
    l1->AddEntry((TObject*)nullptr, "EPJC 68 (2010) 401-420", "");
    l1->AddEntry(gd2, "H1 data: b #rightarrow J/#psi subtracted", "pe");
    l1->Draw();

    const double xl = 0.185;
    Texto(tx, xl, 0.383, "#it{ep} #rightarrow #it{e} #oplus J/#psi X");
    Texto(tx, xl, 0.346, "#sqrt{s_{ep}} = 320 GeV");
    Texto(tx, xl, 0.309, "Q^{2} < 2.5 GeV^{2}");
    Texto(tx, xl, 0.271, "0.3 < z < 0.9");
    Texto(tx, xl, 0.234, "#langle O_{J/#psi}(^{3}S_{1}^{[1]}) #rangle = 1.79 GeV^{3}, m_{c} = 1.5 GeV");
    Texto(tx, xl, 0.197, "#langle O_{J/#psi}(^{1}S_{0}^{[8]}) #rangle = 0.01 GeV^{3}, m_{c} = 1.6 GeV");
    Texto(tx, xl, 0.160, "CT18NLO");
    p1->RedrawAxis();

    // ================= PAINEL DIREITO =================
    p2->cd();
    Moldura(60, 1.45, 240, 164, "W_{#gamma p} [GeV]", "#sigma [nb]", 209, 1.05);

    auto* banda = Banda(bl, bh, cBand);
    banda->Draw("F");

    auto* h1 = Degrau(b1, cTeal, 3);     // pontilhado
    auto* h2 = Degrau(b2, cTeal, 4);     // traco-ponto
    auto* h3 = Degrau(b3, cNavy, 2);     // tracejado
    auto* h4 = Degrau(b4, cNavy, 1);     // solido
    auto* hd = Dados(dd, kBlack, 33, 1.5);   // losango cheio
    for (auto* g : {h3, h4, h1, h2}) g->Draw("L");
    hd->Draw("PZ");

    auto* l2 = new TLegend(0.175, 0.734, 0.640, 0.930);
    EstiloLegenda(l2, false, cLegBox);
    l2->AddEntry(hd, "H1 data EPJC 68 (2010) 401-420", "pe");
    l2->AddEntry(h1, "HO2.6.7: ^{3}S_{1}^{[1]}", "l");
    l2->AddEntry(h2, "HO2.6.7 + PYTHIA8.310 + tune: ^{3}S_{1}^{[1]}", "l");
    l2->AddEntry(h3, "HO2.6.7: ^{1}S_{0}^{[8]}", "l");
    l2->AddEntry(h4, "HO2.6.7 + PYTHIA8.310 + tune: ^{1}S_{0}^{[8]}", "l");
    l2->Draw();

    // bloco superior direito
    Texto(tx, 0.69, 0.895, "#gamma p #rightarrow J/#psi X");
    Texto(tx, 0.69, 0.840, "P_{T}^{J/#psi} > 1 GeV");
    Texto(tx, 0.69, 0.795, "Q^{2} < 2.5 GeV^{2}");
    Texto(tx, 0.69, 0.752, "0.3 < z < 0.9");
    // bloco inferior esquerdo
    const double xl2 = 0.196;
    Texto(tx, xl2, 0.348, "#langle O_{J/#psi}(^{3}S_{1}^{[1]}) #rangle = 1.79 GeV^{3}, m_{c} = 1.5 GeV");
    Texto(tx, xl2, 0.295, "#langle O_{J/#psi}(^{1}S_{0}^{[8]}) #rangle = 0.01 GeV^{3}, m_{c} = 1.6 GeV");
    Texto(tx, xl2, 0.215, "#mu_{R} = #mu_{F} = m_{T}");
    Texto(tx, xl2, 0.168, "CT18NLO");
    p2->RedrawAxis();

    // ========= SAIDA =========
    c->cd();
    c->Modified();
    c->Update();
    c->Print(saida);
    std::cout << "Figura salva em " << saida << "\n";
}
