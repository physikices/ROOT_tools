// Uso: root -l -b -q 'gamma2T4c_ROOTsetup_v3.C("rootMetaData_PbPb.txt","../plots/tex_compile/PbPb.tex")'
//      root -l -b -q 'gamma2T4c_ROOTsetup_v3.C("rootMetaData_pp.txt","../plots/tex_compile/pp.tex")'
//      root -l -b -q 'gamma2T4c_ROOTsetup_v3.C("rootMetaData_pPb.txt","../plots/tex_compile/pPb.tex")'
//
// Mesmo formato de metadados da v2. Mudancas da v3 (layout estilo matplotlib):
//   - paleta teal/navy, estilos de linha (solido, tracejado, pontilhado, traco-ponto)
//   - legenda no canto superior direito, com moldura cinza clara e fundo branco
//   - aux + titulo viram bloco de texto alinhado a esquerda, no canto inferior esquerdo
//   - banda entre as duas primeiras curvas em cinza-azulado
//   - moldura fina, ticks para dentro nos 4 lados, margens estilo matplotlib
//
// Ajuste fino: todas as posicoes/cores estao no bloco "LAYOUT" abaixo.

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

// Le pares (x, y); ignora comentarios, linhas invalidas e y<=0 se logY.
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

void gamma2T4c_ROOTsetup_v3(const char* metaFile = "rootMetaData_PbPb.txt",
                            const char* outFile  = "../plots/tex_compile/stdRoot_out.pdf") {
    // ========= OPCOES =========
    const bool   kLogY     = true;
    const double kGapAlpha = 0.75;
    const bool   kBandNaLegenda = false;

    // ========= LAYOUT (estilo matplotlib) =========
    // margens do pad: esquerda, direita, baixo, topo
    const double mL = 0.14, mR = 0.04, mB = 0.14, mT = 0.04;
    // legenda (NDC): borda direita, topo, largura, altura por entrada
    const double legRight = 0.96, legTop = 0.96, legW = 0.20, legDy = 0.058;
    const double legTextSize = 0.040;
    // bloco de texto inferior esquerdo (NDC): x, y da 1a linha, passo, tamanho
    const double txX = 0.17, txY = 0.27, txDy = 0.065, txSize = 0.048;

    const int cTeal   = TColor::GetColor("#1aa3a3");
    const int cNavy   = TColor::GetColor("#1f1f9e");
    const int cBand   = TColor::GetColor("#9fbcbc");
    const int cLegBox = TColor::GetColor("#cccccc");

    // cores/estilos por curva (1=solido, 2=tracejado, 3=pontilhado, 4=traco-ponto)
    const int colors[] = { cNavy, cTeal, cNavy, cTeal, kBlack, kGray+2, kRed+1, kOrange+7 };
    const int styles[] = {     1,     2,     2,     3,      4,       1,      1,         1 };
    const int nColors  = sizeof(colors) / sizeof(int);

    // ========= ESTILO =========
    gROOT->SetBatch(kTRUE);
    gStyle->SetOptStat(0);
    gStyle->SetTextFont(42);
    gStyle->SetTitleFont(42, "XYZ");
    gStyle->SetLabelFont(42, "XYZ");
    gStyle->SetLabelSize(0.045, "XYZ");
    gStyle->SetPadTickX(1);          // ticks nos 4 lados, para dentro
    gStyle->SetPadTickY(1);
    gStyle->SetTickLength(0.025, "X");
    gStyle->SetTickLength(0.018, "Y");
    gStyle->SetFrameLineWidth(1);
    gStyle->SetLineScalePS(1.5);

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
        const size_t k = graficos.size();          // indice entre curvas validas
        gr->SetLineColor(colors[k % nColors]);
        gr->SetLineStyle(styles[k % nColors]);
        gr->SetLineWidth(2);
        graficos.push_back(gr);
        legGraficos.push_back(legendas[i]);
    }

    if (graficos.empty()) {
        std::cerr << "Nenhuma curva valida para desenhar.\n";
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
        std::cerr << "ylim invalido para escala log (ymin <= 0)\n";
        return;
    }

    // ========= CANVAS E FRAME =========
    auto* c1 = new TCanvas("c1", "Plotagem flexivel", 816, 600);
    c1->SetMargin(mL, mR, mB, mT);
    if (kLogY) c1->SetLogy();
    c1->SetGrid(0, 0);
    c1->SetFrameLineWidth(1);

    TH1* frame = c1->DrawFrame(xmin, ymin, xmax, ymax);
    for (TAxis* ax : {frame->GetXaxis(), frame->GetYaxis()}) {
        ax->SetTitleSize(0.06);
        ax->SetTitleOffset(1.0);
        ax->SetLabelOffset(0.01);
    }
    frame->GetXaxis()->SetTitle(eixoX.c_str());
    frame->GetYaxis()->SetTitle(eixoY.c_str());
    frame->GetXaxis()->SetNdivisions(510, kTRUE);
    frame->GetYaxis()->SetNdivisions(808, kTRUE);

    // ========= GAPS DE RAPIDEZ =========
    struct RapidityGap { double xmin, xmax; int color, fillStyle; const char* label; };
    const std::vector<RapidityGap> gaps = {
        { -1.0, 1.0, kBlue-9,  3004, "ALICE" },
        {  2.0, 4.5, kGreen-7, 3005, "LHCb"  }
    };
    int nGapsVis = 0;
    for (const auto& g : gaps)
        if (std::max(g.xmin, xmin) < std::min(g.xmax, xmax)) ++nGapsVis;

    // ========= LEGENDA (canto superior direito, moldura cinza clara) =========
    const int nEntradas = graficos.size() + nGapsVis + (kBandNaLegenda ? 1 : 0);
    const double legH = legDy * nEntradas;
    auto* legenda = new TLegend(legRight - legW, legTop - legH, legRight, legTop);
    legenda->SetBorderSize(1);
    legenda->SetLineColor(cLegBox);
    legenda->SetFillColorAlpha(kWhite, 0.85);
    legenda->SetFillStyle(1001);
    legenda->SetTextFont(42);
    legenda->SetTextSize(legTextSize);
    legenda->SetMargin(0.18);

    for (size_t i = 0; i < graficos.size(); ++i)
        legenda->AddEntry(graficos[i], legGraficos[i].c_str(), "l");

    for (const auto& g : gaps) {
        double lo = std::max(g.xmin, xmin), hi = std::min(g.xmax, xmax);
        if (lo >= hi) continue;  // fora do range visivel

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

    // ========= BANDA ENTRE AS DUAS PRIMEIRAS CURVAS (cinza-azulado) =========
    if (graficos.size() >= 2) {
        TGraph* gA = graficos[0];
        TGraph* gB = graficos[1];

        double x0 = std::max({gA->GetX()[0], gB->GetX()[0], xmin});
        double x1 = std::min({gA->GetX()[gA->GetN()-1],
                              gB->GetX()[gB->GetN()-1], xmax});

        auto clampY = [&](double v) { return std::min(std::max(v, ymin), ymax); };

        std::vector<double> bx, by;
        for (int i = 0; i < gA->GetN(); ++i) {          // ida: ao longo de A
            double xi = gA->GetX()[i];
            if (xi < x0 || xi > x1) continue;
            bx.push_back(xi);
            by.push_back(clampY(gA->GetY()[i]));
        }
        for (int i = gA->GetN() - 1; i >= 0; --i) {     // volta: B interpolada nos x de A
            double xi = gA->GetX()[i];
            if (xi < x0 || xi > x1) continue;
            bx.push_back(xi);
            by.push_back(clampY(gB->Eval(xi)));
        }

        auto* banda = new TGraph(bx.size(), bx.data(), by.data());
        banda->SetFillColorAlpha(cBand, 0.65);
        banda->SetFillStyle(1001);
        banda->SetLineWidth(0);
        banda->Draw("F");

        if (kBandNaLegenda) {
            auto* legBanda = new TBox();
            legBanda->SetFillColorAlpha(cBand, 0.65);
            legBanda->SetFillStyle(1001);
            legBanda->SetLineColor(cBand);
            legenda->AddEntry(legBanda, "Band", "f");
        }
    }

    // ========= CURVAS =========
    for (auto* gr : graficos) gr->Draw("L");
    legenda->Draw();

    // ========= TEXTOS: bloco alinhado a esquerda, canto inferior esquerdo =========
    TLatex tx;
    tx.SetNDC();
    tx.SetTextFont(42);
    tx.SetTextAlign(12);          // esquerda, centrado na vertical
    tx.SetTextSize(txSize);
    tx.SetTextColor(kBlack);

    double yAtual = txY;
    if (!auxTexto.empty()) {      // 1a linha: reacao (como "ep -> e + J/psi X")
        tx.DrawLatex(txX, yAtual, auxTexto.c_str());
        yAtual -= txDy;
    }
    if (!tituloInterno.empty()) { // 2a linha: energia (como "sqrt(s_ep)=320GeV")
        tx.DrawLatex(txX, yAtual, tituloInterno.c_str());
    }

    // ========= SAIDA =========
    c1->RedrawAxis();
    c1->Modified();
    c1->Update();

    gSystem->mkdir(gSystem->DirName(outFile), kTRUE);
    c1->Print(outFile);
}
