// Uso: root -l -b -q 'gamma2T4c_ROOTsetup_v2.C'
//      root -l -b -q 'gamma2T4c_ROOTsetup_v2.C("outro_meta.txt", "saida.tex")'
//
// rootMetaData.txt:
//   titulo: texto (LaTeX ROOT)
//   aux:    Auxiliar (LaTeX ROOT) (opcional)
//   eixoX:  rótulo
//   eixoY:  rótulo
//   xlim:   min max      (opcional)
//   ylim:   min max      (opcional)
//   curvas:
//   arquivo1.dat  legenda 1
//   arquivo2.dat  legenda 2
// Linhas começando com '#' são ignoradas (metadados e dados).

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

static std::string Trim(std::string s) {
    const char* ws = " \t\r\n";
    s.erase(0, s.find_first_not_of(ws));
    s.erase(s.find_last_not_of(ws) + 1);
    return s;
}

static bool StartsWith(const std::string& s, const std::string& p) {
    return s.rfind(p, 0) == 0;
}

// Lê pares (x, y); ignora comentários, linhas inválidas e y<=0 se logY.
static bool LeDados(const std::string& nome, std::vector<double>& x,
                    std::vector<double>& y, bool logY) {
    std::ifstream arq(nome);
    if (!arq.is_open()) {
        std::cerr << "Erro ao abrir " << nome << "\n";
        return false;
    }
    std::string l;
    while (std::getline(arq, l)) {
        l = Trim(l);
        if (l.empty() || l[0] == '#') continue;
        std::istringstream iss(l);
        double xv, yv;
        if (!(iss >> xv >> yv)) continue;
        if (logY && yv <= 0) continue;
        x.push_back(xv);
        y.push_back(yv);
    }
    if (x.empty()) {
        std::cerr << "Arquivo vazio ou mal formatado: " << nome << "\n";
        return false;
    }
    return true;
}

void gamma2T4c_ROOTsetup_v2(const char* metaFile = "rootMetaData_PbPb.txt",
                         const char* outFile  = "../plots/tex_compile/stdRoot_out.tex") {
    // ========= OPÇÕES =========
    const bool   kLogY    = true;
    const double kGapAlpha = 0.75;

    // ========= ESTILO =========
    gROOT->SetBatch(kTRUE);
    gStyle->SetOptStat(0);
    gStyle->SetTextFont(42);
    gStyle->SetTitleFont(42, "XYZ");
    gStyle->SetLabelFont(42, "XYZ");
    gStyle->SetLabelSize(0.045, "XYZ");
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);

    // ========= LEITURA DOS METADADOS =========
    std::ifstream meta(metaFile);
    if (!meta.is_open()) {
        std::cerr << "Erro ao abrir " << metaFile << "\n";
        return;
    }

    std::string tituloInterno, eixoX, eixoY, auxTexto, linha;
    std::vector<std::string> arquivos, legendas;
    double xlim[2] = {0, 0}, ylim[2] = {0, 0};
    bool temXlim = false, temYlim = false;
    bool lendoCurvas = false;

    while (std::getline(meta, linha)) {
        linha = Trim(linha);
        if (linha.empty() || linha[0] == '#') continue;

        if      (StartsWith(linha, "titulo:")) tituloInterno = Trim(linha.substr(7));
        else if (StartsWith(linha, "eixoX:"))  eixoX = Trim(linha.substr(6));
        else if (StartsWith(linha, "eixoY:"))  eixoY = Trim(linha.substr(6));
        else if (StartsWith(linha, "aux:"))    auxTexto = Trim(linha.substr(4));
        else if (StartsWith(linha, "xlim:")) {
            std::istringstream iss(linha.substr(5));
            temXlim = static_cast<bool>(iss >> xlim[0] >> xlim[1]);
        } else if (StartsWith(linha, "ylim:")) {
            std::istringstream iss(linha.substr(5));
            temYlim = static_cast<bool>(iss >> ylim[0] >> ylim[1]);
        } else if (StartsWith(linha, "curvas:")) {
            lendoCurvas = true;
        } else if (lendoCurvas) {
            std::istringstream iss(linha);
            std::string nomeArquivo, rotulo;
            if (!(iss >> nomeArquivo)) continue;
            std::getline(iss, rotulo);
            arquivos.push_back(nomeArquivo);
            legendas.push_back(Trim(rotulo));
        }
    }
    meta.close();

    if (arquivos.empty()) {
        std::cerr << "Nenhuma curva definida em " << metaFile << "\n";
        return;
    }

    // ========= LEITURA DAS CURVAS =========
    const int colors[] = {
        kRed, kBlue, kGreen+2, kMagenta, kOrange+7, kCyan+2, kBlack,
        kViolet+1, kAzure+2, kSpring+5, kTeal+3, kPink+6, kGray+2
    };
    const int nColors = sizeof(colors) / sizeof(int);

    std::vector<TGraph*> graficos;
    std::vector<std::string> legGraficos;
    double dxmin = 1e300, dxmax = -1e300, dymin = 1e300, dymax = -1e300;

    for (size_t i = 0; i < arquivos.size(); ++i) {
        std::vector<double> x, y;
        if (!LeDados(arquivos[i], x, y, kLogY)) continue;

        dxmin = std::min(dxmin, *std::min_element(x.begin(), x.end()));
        dxmax = std::max(dxmax, *std::max_element(x.begin(), x.end()));
        dymin = std::min(dymin, *std::min_element(y.begin(), y.end()));
        dymax = std::max(dymax, *std::max_element(y.begin(), y.end()));

        auto* gr = new TGraph(x.size(), x.data(), y.data());
        gr->SetLineColor(colors[i % nColors]);
        gr->SetLineWidth(2);
        graficos.push_back(gr);
        legGraficos.push_back(legendas[i]);
    }

    if (graficos.empty()) {
        std::cerr << "Nenhuma curva válida para desenhar.\n";
        return;
    }

    // ========= RANGE DOS EIXOS =========
    double xmin = temXlim ? xlim[0] : dxmin;
    double xmax = temXlim ? xlim[1] : dxmax;
    double ymin, ymax;
    if (temYlim) {
        ymin = ylim[0]; ymax = ylim[1];
    } else if (kLogY) {
        ymin = std::pow(10, std::floor(std::log10(dymin)));
        ymax = std::pow(10, std::ceil (std::log10(dymax)));
    } else {
        double m = 0.05 * (dymax - dymin);
        ymin = dymin - m; ymax = dymax + m;
    }

    if (kLogY && ymin <= 0) {
        std::cerr << "ylim inválido para escala log (ymin <= 0)\n";
        return;
    }

    // ========= CANVAS E FRAME =========
    auto* c1 = new TCanvas("c1", "Plotagem flexível", 800, 600);
    if (kLogY) c1->SetLogy();
    c1->SetGrid(0, 0);

    TH1* frame = c1->DrawFrame(xmin, ymin, xmax, ymax);
    for (TAxis* ax : {frame->GetXaxis(), frame->GetYaxis()}) {
        ax->SetTitleSize(0.06);
        ax->SetTitleOffset(1.0);
        ax->SetLabelOffset(0.01);
    }
    frame->GetXaxis()->SetTitle(eixoX.c_str());
    frame->GetYaxis()->SetTitle(eixoY.c_str());
    frame->GetYaxis()->SetNdivisions(808, kTRUE);

    // ========= LEGENDA =========
    auto* legenda = new TLegend(0.73, 0.87, 0.83, 0.70);
    legenda->SetBorderSize(0);
    legenda->SetFillStyle(0);
    legenda->SetTextSize(0.035);

    for (size_t i = 0; i < graficos.size(); ++i)
        legenda->AddEntry(graficos[i], legGraficos[i].c_str(), "l");

    // ========= GAPS DE RAPIDEZ (atrás das curvas) =========
    struct RapidityGap { double xmin, xmax; int color, fillStyle; const char* label; };
    const std::vector<RapidityGap> gaps = {
        { -1.0, 1.0, kBlue-9,  3004, "ALICE" },
        {  2.0, 4.5, kGreen-7, 3005, "LHCb"  }
    };

    for (const auto& g : gaps) {
        double lo = std::max(g.xmin, xmin), hi = std::min(g.xmax, xmax);
        if (lo >= hi) continue;  // fora do range visível

        auto* box = new TBox(lo, ymin, hi, ymax);
        box->SetFillColorAlpha(g.color, kGapAlpha);
        box->SetFillStyle(g.fillStyle);
        box->SetLineColor(g.color);
        box->SetLineWidth(1);
        box->Draw("same");

        auto* legBox = new TBox();
        legBox->SetFillColorAlpha(g.color, kGapAlpha);
        legBox->SetFillStyle(g.fillStyle);
        legBox->SetLineColor(g.color);
        legenda->AddEntry(legBox, g.label, "f");
    }

    // ========= LINHA EM x = 0 =========
    if (xmin < 0 && xmax > 0) {
        auto* x0 = new TLine(0, ymin, 0, ymax);
        x0->SetLineStyle(2);
        x0->SetLineWidth(2);
        x0->SetLineColorAlpha(kGray+2, 0.75);
        x0->Draw("same");
    }

    // ========= BANDA HACHURADA ENTRE DUAS CURVAS =========
    if (graficos.size() >= 2) {
        TGraph* gA = graficos[0];
        TGraph* gB = graficos[1];

        // x comum às duas curvas E dentro do range visível
        double x0 = std::max({gA->GetX()[0], gB->GetX()[0], xmin});
        double x1 = std::min({gA->GetX()[gA->GetN()-1],
                              gB->GetX()[gB->GetN()-1], xmax});

        auto clampY = [&](double v) { return std::min(std::max(v, ymin), ymax); };

        std::vector<double> bx, by;
        // ida: ao longo de A
        for (int i = 0; i < gA->GetN(); ++i) {
            double xi = gA->GetX()[i];
            if (xi < x0 || xi > x1) continue;
            bx.push_back(xi);
            by.push_back(clampY(gA->GetY()[i]));
        }
        // volta: ao longo de B (interpolada nos x de A)
        for (int i = gA->GetN() - 1; i >= 0; --i) {
            double xi = gA->GetX()[i];
            if (xi < x0 || xi > x1) continue;
            bx.push_back(xi);
            by.push_back(clampY(gB->Eval(xi)));
        }

        auto* banda = new TGraph(bx.size(), bx.data(), by.data());
        banda->SetFillColor(kMagenta-6);
        // banda->SetFillStyle(3354);  // 3354 = hachura
        
        banda->SetFillColorAlpha(kMagenta-6, 0.2);   // 0 = invisível, 1 = opaco
        banda->SetFillStyle(1001);

        banda->SetLineWidth(0);
        banda->Draw("F");

        // entrada de legenda com TBox (o TGraph não mostra a hachura na legenda)
        auto* legBanda = new TBox();
        legBanda->SetFillColor(kMagenta-6);
        // legBanda->SetFillStyle(3354);  // 3354 = hachura
        
        legBanda->SetFillColorAlpha(kGray+2, 0.2);   // ou SetFillColor(kGray+1)
        legBanda->SetFillStyle(1001);

        legBanda->SetLineColor(kMagenta-6);
        // legenda->AddEntry(legBanda, "Band", "f");
    }

    // ========= CURVAS =========
    for (auto* gr : graficos) gr->Draw("L");
    legenda->Draw();

    // ========= TEXTOS =========
    TLatex tx;
    tx.SetNDC();
    tx.SetTextFont(42);

    tx.SetTextAlign(22);
    tx.SetTextSize(0.060);
    tx.SetTextColor(kBlack);
    tx.DrawLatex(0.25, 0.835, tituloInterno.c_str());

    tx.SetTextAlign(12);
    tx.SetTextSize(0.050);
    tx.SetTextColor(kGray+2);
    if (!auxTexto.empty())
        tx.DrawLatex(0.14, 0.777, auxTexto.c_str());
    // ========= SAÍDA =========
    c1->RedrawAxis();
    c1->Modified();
    c1->Update();

    gSystem->mkdir(gSystem->DirName(outFile), kTRUE);
    c1->Print(outFile);
}
